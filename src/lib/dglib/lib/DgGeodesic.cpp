/*******************************************************************************
    Copyright (C) 2023 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//
// DgGeodesic.cpp: DgGeodesic class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgConstants.h>
#include <dglib/DgGeodesic.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <limits>

namespace {

const long double kEps = LDBL_EPSILON;

// Stands in for a zero cosine or sine so that the limit at a pole or on the
// equator is taken from the right side; its square does not underflow.
// (A function, so that it does not depend on the order of static
// initialization.)
long double tiny (void)
{
   static const long double value = sqrtl(LDBL_MIN);
   return value;
}

// A reduced latitude closer to the equator than this is on the equator.
const long double kLatFloor = LDBL_EPSILON * LDBL_EPSILON;

// Residual of the longitude equation at which the inverse has converged.
const long double kLonTol = 8.0L * LDBL_EPSILON;

const int kMaxInverseIter = 400;

// The boundary of a polygon must stay this far (radians) from one pole.
const long double kMinPoleClearance = 0.05L;

// Half-length limit (radians of arc) of a quadrature panel along an edge.
const long double kMaxAreaPanel = 0.05L;

////////////////////////////////////////////////////////////////////////////////
// Gauss-Legendre nodes and weights on [0, 1].
struct GaussRule {

   enum { N = 12 };

   long double u[N], w[N];

   GaussRule (void)
   {
      for (int i = 0; i < N; ++i) {
         // Newton's method on the Legendre polynomial P_N
         long double x = cosl(M_PI_L * (i + 0.75L) / (N + 0.5L));
         long double dp = 1.0L;
         for (int it = 0; it < 100; ++it) {
            const long double dx = legendre(x, dp) / dp;
            x -= dx;
            if (fabsl(dx) <= 2.0L * LDBL_EPSILON) break;
         }
         legendre(x, dp);
         u[i] = (1.0L + x) / 2.0L;
         w[i] = 1.0L / ((1.0L - x * x) * dp * dp);
      }
   }

   // P_N(x), with its derivative in dp
   static long double legendre (long double x, long double& dp)
   {
      long double p0 = 1.0L, p1 = x;
      for (int k = 2; k <= N; ++k) {
         const long double p2 = ((2 * k - 1) * x * p1 - (k - 1) * p0) / k;
         p0 = p1;
         p1 = p2;
      }
      dp = N * (x * p1 - p0) / (x * x - 1.0L);
      return p1;
   }
};

const GaussRule& gaussRule (void)
{
   static const GaussRule rule;
   return rule;
}

////////////////////////////////////////////////////////////////////////////////
// scale (s, c) to unit length; (0, 0) becomes the zero angle
void normalize (long double& s, long double& c)
{
   const long double r = hypotl(s, c);
   if (r > 0.0L) {
      s /= r;
      c /= r;
   } else {
      s = 0.0L;
      c = 1.0L;
   }
}

long double log1pOverX (long double x)
{
   return x == 0.0L ? 1.0L : log1pl(x) / x;
}

long double atanhOverX (long double x)
{
   return x == 0.0L ? 1.0L : atanhl(x) / x;
}

long double sq (long double x) { return x * x; }

} // namespace

////////////////////////////////////////////////////////////////////////////////
DgGeodesic::DgGeodesic (long double a, long double b)
   : a_ (a), b_ (b), f_ (0.0L), f1_ (1.0L), e2_ (0.0L), e_ (0.0L),
     ep2_ (0.0L), valid_ (false)
{
   if (!std::isfinite(a) || !std::isfinite(b) || !(b > 0.0L) || !(a >= b))
      return;

   f_ = (a - b) / a;
   f1_ = b / a;
   e2_ = f_ * (2.0L - f_);
   e_ = sqrtl(e2_);
   ep2_ = e2_ / (f1_ * f1_);
   valid_ = std::isfinite(ep2_);
}

////////////////////////////////////////////////////////////////////////////////
// Sine and cosine of the reduced latitude: tan(beta) = (1 - f) tan(lat).
// The cosine is kept positive so that a pole is the limit of nearby points.
//
// Returns the length of ((1 - f) sin(lat), cos(lat)), which gives
// sin(beta2 - beta1) = (1 - f) sin(lat2 - lat1) / (norm1 norm2) without the
// cancellation of sin(beta2) cos(beta1) - cos(beta2) sin(beta1).
long double
DgGeodesic::reduced (long double lat, long double& sinBeta,
                     long double& cosBeta) const
{
   sinBeta = f1_ * sinl(lat);
   cosBeta = cosl(lat);
   const long double norm = hypotl(sinBeta, cosBeta);
   normalize(sinBeta, cosBeta);
   if (!(cosBeta > tiny())) cosBeta = tiny();
   if (fabsl(sinBeta) < kLatFloor) sinBeta = 0.0L;
   return norm;
}

////////////////////////////////////////////////////////////////////////////////
// Snyder's q: the area of the zone from the equator to the latitude, as a
// fraction of a^2 / 2 per radian of longitude. q(1) * a^2 / 2 is the square
// of the authalic radius.
long double
DgGeodesic::q (long double sinLat) const
{
   return (1.0L - e2_) * sinLat *
          (1.0L / (1.0L - e2_ * sinLat * sinLat) + atanhOverX(e_ * sinLat));
}

long double
DgGeodesic::totalArea (void) const
{
   return M_2PI * a_ * a_ * q(1.0L);
}

////////////////////////////////////////////////////////////////////////////////
// Add to E, D3 and J the integrals over [sigma, sigma + dSigma] of
//
//    E:  w - 1                       (distance:  s / b = sigma + E)
//    D3: 1 - (2 - f) / (1 + (1 - f) w)
//                                    (longitude: lon = omega
//                                                - f sin(alpha0) (sigma - D3))
//    J:  k^2 sin^2 / w               (reduced length)
//
// with w = sqrt(1 + k^2 sin^2). The integrands are analytic within
// asinh(1 / k) of the real axis, so the panels are kept shorter than that.
void
DgGeodesic::panelSums (long double k2, long double sigma, long double dSigma,
                       long double& E, long double& D3, long double& J) const
{
   const GaussRule& rule = gaussRule();
   const long double maxPanel =
      std::min(M_PI_2_L, asinhl(1.0L / sqrtl(k2)) / 2.0L);
   const int panels =
      std::max(1, static_cast<int>(ceill(fabsl(dSigma) / maxPanel)));
   const long double h = dSigma / panels;

   long double sumE = 0.0L, sumD = 0.0L, sumJ = 0.0L;
   for (int p = 0; p < panels; ++p) {
      for (int i = 0; i < GaussRule::N; ++i) {
         const long double sn = sinl(sigma + (p + rule.u[i]) * h);
         const long double x = k2 * sn * sn;
         const long double w = sqrtl(1.0L + x);
         const long double delta = x / (1.0L + w);
         sumE += rule.w[i] * delta;
         sumD += rule.w[i] * f1_ * delta / (2.0L - f_ + f1_ * delta);
         sumJ += rule.w[i] * x / w;
      }
   }

   E += h * sumE;
   D3 += h * sumD;
   J += h * sumJ;
}

void
DgGeodesic::integrals (long double k2, long double sigma, long double dSigma,
                       long double* E, long double* D3, long double* J) const
{
   long double e = 0.0L, d = 0.0L, j = 0.0L;
   if (k2 > 0.0L && dSigma != 0.0L) {
      // the integrands have period pi: whole periods cost one evaluation
      const long double periods = truncl(dSigma / M_PI_L);
      if (fabsl(periods) >= 2.0L) {
         panelSums(k2, 0.0L, M_PI_L, e, d, j);
         e *= periods;
         d *= periods;
         j *= periods;
         dSigma -= periods * M_PI_L;
      }
      panelSums(k2, sigma, dSigma, e, d, j);
   }

   if (E) *E = e;
   if (D3) *D3 = d;
   if (J) *J = j;
}

////////////////////////////////////////////////////////////////////////////////
DgGeodesic::Line
DgGeodesic::line (long double lat1, long double sinAzimuth1,
                  long double cosAzimuth1) const
{
   long double sbet1, cbet1;
   reduced(lat1, sbet1, cbet1);

   Line l;
   l.sinAlpha0 = sinAzimuth1 * cbet1;
   l.cosAlpha0 = hypotl(cosAzimuth1, sinAzimuth1 * sbet1);
   l.sinSigma1 = sbet1;
   l.cosSigma1 = cosAzimuth1 * cbet1;
   normalize(l.sinSigma1, l.cosSigma1);
   l.sigma1 = atan2l(l.sinSigma1, l.cosSigma1);
   l.k2 = ep2_ * l.cosAlpha0 * l.cosAlpha0;
   return l;
}

////////////////////////////////////////////////////////////////////////////////
// The longitude difference lam12 between point 1 and the point where the
// geodesic leaving point 1 at azimuth alpha1 first crosses the parallel of
// point 2 while heading north, and its derivative with respect to alpha1.
// Requires sinBeta1 < 0 and |sinBeta2| <= -sinBeta1.
void
DgGeodesic::lambda12 (long double sbet1, long double cbet1,
                      long double sbet2, long double cbet2,
                      long double salp1, long double calp1, Lambda& o) const
{
   o.sinAlpha1 = salp1;
   o.cosAlpha1 = calp1;
   o.sinAlpha0 = salp1 * cbet1;
   o.cosAlpha0 = hypotl(calp1, salp1 * sbet1);

   long double ssig1 = sbet1, csig1 = calp1 * cbet1;
   long double somg1 = o.sinAlpha0 * sbet1, comg1 = csig1;
   normalize(ssig1, csig1);
   normalize(somg1, comg1);

   // Clairaut: sin(alpha2) cos(beta2) = sin(alpha1) cos(beta1). The
   // difference of the squared cosines is taken in its better-conditioned
   // form.
   o.sinAlpha2 = o.sinAlpha0 / cbet2;
   if (cbet2 != cbet1 || fabsl(sbet2) != -sbet1) {
      const long double diff = cbet1 < -sbet1 ?
         (cbet2 - cbet1) * (cbet1 + cbet2) :
         (sbet1 - sbet2) * (sbet1 + sbet2);
      o.cosAlpha2 =
         sqrtl(std::max(0.0L, sq(calp1 * cbet1) + diff)) / cbet2;
   } else {
      o.cosAlpha2 = fabsl(calp1);
   }

   long double ssig2 = sbet2, csig2 = o.cosAlpha2 * cbet2;
   long double somg2 = o.sinAlpha0 * sbet2, comg2 = csig2;
   normalize(ssig2, csig2);
   normalize(somg2, comg2);

   o.sigma12 = atan2l(std::max(0.0L, csig1 * ssig2 - ssig1 * csig2),
                      csig1 * csig2 + ssig1 * ssig2);
   const long double omg12 =
      atan2l(std::max(0.0L, comg1 * somg2 - somg1 * comg2),
             comg1 * comg2 + somg1 * somg2);

   o.k2 = ep2_ * o.cosAlpha0 * o.cosAlpha0;
   o.sinSigma1 = ssig1;
   o.cosSigma1 = csig1;
   o.sigma1 = atan2l(ssig1, csig1);

   long double D3 = 0.0L, J = 0.0L;
   integrals(o.k2, o.sigma1, o.sigma12, nullptr, &D3, &J);
   o.lam12 = omg12 - f_ * o.sinAlpha0 * (o.sigma12 - D3);

   // d(lam12)/d(alpha1) = m12 / (a cos(alpha2) cos(beta2)), m12 being the
   // reduced length of the arc
   if (o.cosAlpha2 > 0.0L) {
      const long double w1 = sqrtl(1.0L + o.k2 * ssig1 * ssig1);
      const long double w2 = sqrtl(1.0L + o.k2 * ssig2 * ssig2);
      const long double m12b =
         w2 * csig1 * ssig2 - w1 * ssig1 * csig2 - csig1 * csig2 * J;
      o.dlam12 = f1_ * m12b / (o.cosAlpha2 * cbet2);
   } else {
      o.dlam12 = 0.0L;
   }
}

////////////////////////////////////////////////////////////////////////////////
bool
DgGeodesic::inverse (long double lat1, long double lon1,
                     long double lat2, long double lon2, Arc& arc) const
{
   if (!valid_ || !std::isfinite(lat1) || !std::isfinite(lon1) ||
       !std::isfinite(lat2) || !std::isfinite(lon2))
      return false;

   // Arrange the points so that 0 <= lam12 <= pi, beta1 <= 0 and
   // |beta2| <= |beta1|; the signs record how to undo this.
   long double lam12 = remainderl(lon2 - lon1, M_2PI);
   const int lonSign = lam12 < 0.0L ? -1 : 1;
   lam12 = fabsl(lam12);

   long double sbet1, cbet1, sbet2, cbet2;
   const long double norm1 = reduced(lat1, sbet1, cbet1);
   const long double norm2 = reduced(lat2, sbet2, cbet2);
   // which point is farther from the equator: toward the poles the sines
   // round to 1, so the cosines decide there
   const bool nearEquator = fabsl(sbet1) < 0.7L || fabsl(sbet2) < 0.7L;
   const int swapSign =
      (nearEquator ? fabsl(sbet1) < fabsl(sbet2) : cbet1 > cbet2) ? -1 : 1;
   if (swapSign < 0) {
      std::swap(sbet1, sbet2);
      std::swap(cbet1, cbet2);
   }
   const int latSign = sbet1 > 0.0L ? -1 : 1;
   sbet1 *= latSign;
   sbet2 *= latSign;

   // sin(beta2 - beta1) in this arrangement, from the latitude difference
   const long double sbet12 =
      swapSign * latSign * f1_ * sinl(lat2 - lat1) / (norm1 * norm2);

   long double salp1 = 0.0L, calp1 = 1.0L, salp2 = 0.0L, calp2 = 1.0L;
   long double sigma12 = 0.0L, s12 = 0.0L;
   int iterations = 0;

   const bool pole1 = cbet1 <= tiny();
   if (pole1 || lam12 == 0.0L || lam12 >= M_PI_L) {

      // Along a meridian. On an oblate ellipsoid this is the shortest path
      // between opposite meridians, passing over the nearer pole, which is
      // the south pole in this arrangement. From a pole the azimuth is the
      // longitude difference.
      if (pole1) {
         salp1 = sinl(lam12);
         calp1 = cosl(lam12);
      } else {
         salp1 = 0.0L;
         calp1 = lam12 == 0.0L ? 1.0L : -1.0L;
      }
      long double ssig1 = sbet1, csig1 = calp1 * cbet1;
      long double ssig2 = sbet2, csig2 = cbet2;
      normalize(ssig1, csig1);
      normalize(ssig2, csig2);
      // on the same meridian the arc is beta2 - beta1
      const long double ssig12 = (!pole1 && lam12 == 0.0L) ?
         sbet12 : csig1 * ssig2 - ssig1 * csig2;
      sigma12 = atan2l(std::max(0.0L, ssig12),
                       csig1 * csig2 + ssig1 * ssig2);
      long double E = 0.0L;
      integrals(ep2_, atan2l(ssig1, csig1), sigma12, &E, nullptr, nullptr);
      s12 = b_ * (sigma12 + E);

   } else if (sbet1 == 0.0L && lam12 <= f1_ * M_PI_L) {

      // Along the equator, which is the shortest path up to a longitude
      // difference of (1 - f) pi.
      salp1 = salp2 = 1.0L;
      calp1 = calp2 = 0.0L;
      sigma12 = lam12 / f1_;
      s12 = a_ * lam12;

   } else {

      if (sbet1 == 0.0L) sbet1 = -tiny();

      // Starting azimuth from the sphere that fits the ellipsoid at the
      // mean latitude: radius b dnm, longitudes scaled by (1 - f) dnm.
      const long double sbm = sbet1 + sbet2, cbm = cbet1 + cbet2;
      const long double dnm =
         sqrtl(1.0L + ep2_ * sbm * sbm / (sbm * sbm + cbm * cbm));
      const long double omg12 = std::min(M_PI_L, lam12 / (f1_ * dnm));
      const long double somg12 = sinl(omg12), comg12 = cosl(omg12);
      const long double sbet12a = sbet2 * cbet1 + cbet2 * sbet1;

      // sin(alpha1) sin(sigma12) and cos(alpha1) sin(sigma12) on that
      // sphere; the form of the cosine depends on which of 1 -/+ cos(omega12)
      // is free of cancellation
      long double sa = cbet2 * somg12;
      long double ca = comg12 >= 0.0L ?
         sbet12 + cbet2 * sbet1 * somg12 * somg12 / (1.0L + comg12) :
         sbet12a - cbet2 * sbet1 * somg12 * somg12 / (1.0L - comg12);
      const long double ssig12 = hypotl(sa, ca);
      const long double csig12 = sbet1 * sbet2 + cbet1 * cbet2 * comg12;
      const long double sigGuess = atan2l(ssig12, csig12);
      normalize(sa, ca);

      // That sphere is exact to a relative error of about f sigma^2.
      const long double shortArc =
         0.1L * sqrtl(kEps / std::max(f_, 1.0e-3L));

      if (ssig12 > 0.0L && sa > 0.0L && sigGuess < shortArc) {

         salp1 = sa;
         calp1 = ca;
         salp2 = cbet1 * somg12;
         calp2 = comg12 >= 0.0L ?
            sbet12 - cbet1 * sbet2 * somg12 * somg12 / (1.0L + comg12) :
            -sbet12a + cbet1 * sbet2 * somg12 * somg12 / (1.0L - comg12);
         normalize(salp2, calp2);
         sigma12 = sigGuess;
         s12 = b_ * dnm * sigma12;

      } else {

         if (!(sa > 0.0L)) {
            sa = 1.0L;
            ca = 0.0L;
         }

         // lam12(alpha1) is 0 at alpha1 = 0 and pi at alpha1 = pi, and
         // equals the target at exactly one azimuth between, below which it
         // is smaller and above which it is larger (Karney 2013). lo and hi
         // bracket that root as unit vectors (sin, cos), which keep full
         // precision where an angle would not.
         long double loS = 0.0L, loC = 1.0L, hiS = 0.0L, hiC = -1.0L;
         Lambda best = Lambda();
         long double bestAbs = std::numeric_limits<long double>::infinity();
         long double prevAbs = bestAbs;
         bool newton = false;

         for (int it = 0; it < kMaxInverseIter; ++it) {

            ++iterations;
            Lambda cur;
            lambda12(sbet1, cbet1, sbet2, cbet2, sa, ca, cur);
            const long double r = cur.lam12 - lam12;
            const long double absR = fabsl(r);
            if (it == 0 || absR < bestAbs) {
               best = cur;
               bestAbs = absR;
            }
            if (absR <= kLonTol) break;
            // at the rounding level and no longer improving
            if (absR <= 64.0L * kLonTol && absR >= prevAbs) break;

            if (r < 0.0L) {
               loS = sa;
               loC = ca;
            } else {
               hiS = sa;
               hiC = ca;
            }

            // Newton step, kept inside the bracket; bisect otherwise, or
            // when the last Newton step gained little
            const bool slow = newton && absR > 0.75L * prevAbs;
            prevAbs = absR;
            newton = false;
            if (!slow && cur.dlam12 > 0.0L) {
               const long double t = -r / cur.dlam12;
               if (fabsl(t) < M_PI_L) {
                  const long double st = sinl(t), ct = cosl(t);
                  long double ns = sa * ct + ca * st;
                  long double nc = ca * ct - sa * st;
                  normalize(ns, nc);
                  if (ns * loC - nc * loS > 0.0L &&
                      hiS * nc - hiC * ns > 0.0L) {
                     if (ns == sa && nc == ca) break;
                     sa = ns;
                     ca = nc;
                     newton = true;
                  }
               }
            }

            if (!newton) {
               long double ms = loS + hiS, mc = loC + hiC;
               if (hypotl(ms, mc) > 0.0L) {
                  normalize(ms, mc);
               } else {
                  ms = 1.0L;
                  mc = 0.0L;
               }
               if ((ms == loS && mc == loC) || (ms == hiS && mc == hiC) ||
                   (ms == sa && mc == ca))
                  break;
               sa = ms;
               ca = mc;
            }
         }

         salp1 = best.sinAlpha1;
         calp1 = best.cosAlpha1;
         salp2 = best.sinAlpha2;
         calp2 = best.cosAlpha2;
         normalize(salp2, calp2);
         sigma12 = best.sigma12;
         long double E = 0.0L;
         integrals(best.k2, best.sigma1, sigma12, &E, nullptr, nullptr);
         s12 = b_ * (sigma12 + E);
      }
   }

   // Undo the arrangement of the points. Swapping them while keeping lam12
   // positive mirrored east and west as well as reversing the direction of
   // travel, so a swap changes the sign of the cosines only.
   if (swapSign < 0) {
      std::swap(salp1, salp2);
      std::swap(calp1, calp2);
   }
   salp1 *= lonSign;
   calp1 *= swapSign * latSign;
   salp2 *= lonSign;
   calp2 *= swapSign * latSign;

   arc.line = line(lat1, salp1, calp1);
   arc.sigma12 = sigma12;
   arc.s12 = s12;
   arc.iterations = iterations;
   arc.sinAzimuth1 = salp1;
   arc.cosAzimuth1 = calp1;
   arc.sinAzimuth2 = salp2;
   arc.cosAzimuth2 = calp2;

   return std::isfinite(s12);
}

////////////////////////////////////////////////////////////////////////////////
void
DgGeodesic::direct (long double lat1, long double lon1, long double azimuth1,
                    long double s12, long double& lat2, long double& lon2,
                    long double& azimuth2) const
{
   const Line l = line(lat1, sinl(azimuth1), cosl(azimuth1));

   // solve sigma12 + E(sigma12) = s12 / b by Newton's method; the slope is
   // within e'^2 / 2 of 1
   const long double tau = s12 / b_;
   long double sigma12 = tau / (1.0L + l.k2 / 4.0L);
   for (int it = 0; it < 50; ++it) {
      long double E = 0.0L;
      integrals(l.k2, l.sigma1, sigma12, &E, nullptr, nullptr);
      const long double sn = sinl(l.sigma1 + sigma12);
      const long double step =
         ((sigma12 - tau) + E) / sqrtl(1.0L + l.k2 * sn * sn);
      sigma12 -= step;
      if (!(fabsl(step) > 2.0L * kEps * fabsl(sigma12))) break;
   }

   const long double s = sinl(sigma12), c = cosl(sigma12);
   const long double ssig2 = l.sinSigma1 * c + l.cosSigma1 * s;
   const long double csig2 = l.cosSigma1 * c - l.sinSigma1 * s;

   const long double sbet2 = l.cosAlpha0 * ssig2;
   const long double cbet2 = hypotl(l.sinAlpha0, l.cosAlpha0 * csig2);
   lat2 = atan2l(sbet2, f1_ * cbet2);

   // omega2 - omega1 with omega = atan2(sin(alpha0) sin(sigma), cos(sigma))
   const long double omg12 =
      atan2l(l.sinAlpha0 * s,
             l.cosSigma1 * csig2 +
             l.sinAlpha0 * l.sinAlpha0 * l.sinSigma1 * ssig2);
   long double D3 = 0.0L;
   integrals(l.k2, l.sigma1, sigma12, nullptr, &D3, nullptr);
   lon2 = remainderl(lon1 + omg12 - f_ * l.sinAlpha0 * (sigma12 - D3),
                     M_2PI);

   azimuth2 = atan2l(l.sinAlpha0, l.cosAlpha0 * csig2);
}

////////////////////////////////////////////////////////////////////////////////
bool
DgGeodesic::polygonArea (const std::vector<long double>& lat,
                         const std::vector<long double>& lon,
                         long double centerLat, long double centerLon,
                         long double& areaM2) const
{
   areaM2 = 0.0L;
   const size_t n = lat.size();
   if (!valid_ || n < 3 || lon.size() != n || !std::isfinite(centerLat) ||
       !std::isfinite(centerLon))
      return false;

   // the edges, the distance of the boundary from each pole (as arc on the
   // auxiliary sphere) and the reach of the ring from the center
   std::vector<Arc> arcs(n);
   long double clearN = M_PI_L, clearS = M_PI_L, reach = 0.0L;
   long double sbetC, cbetC;
   reduced(centerLat, sbetC, cbetC);
   for (size_t i = 0; i < n; ++i) {
      const size_t j = (i + 1) % n;
      Arc& arc = arcs[i];
      if (!inverse(lat[i], lon[i], lat[j], lon[j], arc)) return false;

      long double sbet, cbet;
      reduced(lat[i], sbet, cbet);
      clearN = std::min(clearN, atan2l(cbet, sbet));
      clearS = std::min(clearS, atan2l(cbet, -sbet));

      // a geodesic is nearest a pole at sigma = pi/2 (north) and at
      // sigma = -pi/2 or 3 pi/2 (south), at arc asin|sin(alpha0)| from it
      const Line& l = arc.line;
      const long double end = l.sigma1 + arc.sigma12;
      const long double nearest =
         atan2l(fabsl(l.sinAlpha0), l.cosAlpha0);
      if (l.sigma1 < M_PI_2_L && end > M_PI_2_L)
         clearN = std::min(clearN, nearest);
      if ((l.sigma1 < -M_PI_2_L && end > -M_PI_2_L) ||
          end > 3.0L * M_PI_2_L)
         clearS = std::min(clearS, nearest);

      // Central angle from the center, as the atan2 of its sine and cosine
      // written with sin(dBeta) and sin^2(dLon / 2). That stays accurate
      // beside a pole, where 1 - cos(dBeta) would round to nothing and make
      // a ring around the pole look far from it.
      const long double dLon = lon[i] - centerLon;
      const long double hav2 = 2.0L * sq(sinl(dLon / 2.0L));
      const long double sinDBeta = sbet * cbetC - cbet * sbetC;
      const long double cosDBeta = cbet * cbetC + sbet * sbetC;
      reach = std::max(reach, atan2l(
         hypotl(cbet * sinl(dLon), sinDBeta + sbetC * cbet * hav2),
         cosDBeta - cbetC * cbet * hav2));
   }

   // Area = (a^2 / 2) * integral around the ring of (q0 - q(lat)) dlon.
   //
   // Away from the poles q0 is q at the center: the integrand is then of
   // the size of the polygon and the constant integrates to nothing because
   // the ring does not go around a pole. Near a pole, or around it, q0 is
   // q at that pole: the integrand vanishes there as fast as dlon grows, so
   // the integral needs no winding count and stays regular for an edge
   // through the pole. Either way the other pole(s) of the integrand must be
   // far from the ring compared with a quadrature panel.
   enum Form { Mid, North, South };
   Form form;
   long double clearance;
   if (std::min(clearN, clearS) >= 4.0L * reach) {
      form = Mid;
      clearance = std::min(clearN, clearS);
   } else if (clearN <= clearS) {
      form = North;
      clearance = clearS;
   } else {
      form = South;
      clearance = clearN;
   }
   // a polar form is only used when the ring is near that pole, so the
   // other pole is then far away unless the ring spans a hemisphere
   if (form != Mid && clearance < kMinPoleClearance) return false;
   const long double maxPanel =
      2.0L * std::min(kMaxAreaPanel, clearance / 4.0L);

   // the center, for the Mid form
   long double sphiC = sinl(centerLat), cphiC = cosl(centerLat);
   normalize(sphiC, cphiC);
   const long double dC = cphiC * cphiC / (1.0L + fabsl(sphiC));

   const GaussRule& rule = gaussRule();
   const long double poleSign = form == South ? -1.0L : 1.0L;
   long double total = 0.0L;

   for (size_t i = 0; i < n; ++i) {
      const Arc& arc = arcs[i];
      const Line& l = arc.line;
      // a meridian contributes nothing: dlon is zero along it
      if (!(arc.sigma12 > 0.0L) || l.sinAlpha0 == 0.0L) continue;

      const int panels =
         std::max(1, static_cast<int>(ceill(arc.sigma12 / maxPanel)));
      const long double h = arc.sigma12 / panels;
      long double sum = 0.0L;

      for (int p = 0; p < panels; ++p) {
         for (int g = 0; g < GaussRule::N; ++g) {
            const long double tau = (p + rule.u[g]) * h;
            const long double st = sinl(tau), ct = cosl(tau);
            const long double ssig = l.sinSigma1 * ct + l.cosSigma1 * st;
            const long double csig = l.cosSigma1 * ct - l.sinSigma1 * st;

            const long double sbet = l.cosAlpha0 * ssig;
            const long double cbet = hypotl(l.sinAlpha0, l.cosAlpha0 * csig);
            const long double w = sqrtl(1.0L + l.k2 * ssig * ssig);
            // dlon/dsigma = sin(alpha0) (1 / cos^2(beta) - c3)
            const long double c3 = f_ * (2.0L - f_) / (1.0L + f1_ * w);

            const long double r = hypotl(sbet, f1_ * cbet);
            const long double sphi = sbet / r;
            const long double cphi = f1_ * cbet / r;

            long double value;
            if (form == Mid) {

               // q(center) - q(here), from the difference of the sines.
               // Toward a pole that difference is taken between the
               // distances 1 - |sin| from it, which stay accurate there.
               const long double d = cphi * cphi / (1.0L + fabsl(sphi));
               long double ds;
               if (sphiC > 0.5L && sphi > 0.5L)
                  ds = d - dC;
               else if (sphiC < -0.5L && sphi < -0.5L)
                  ds = dC - d;
               else
                  ds = sphiC - sphi;
               const long double cross = 1.0L - e2_ * sphiC * sphi;
               const long double dq = (1.0L - e2_) * ds *
                  ((1.0L + e2_ * sphiC * sphi) /
                      ((1.0L - e2_ * sphiC * sphiC) *
                       (1.0L - e2_ * sphi * sphi)) +
                   atanhOverX(e_ * ds / cross) / cross);
               value = dq * (1.0L / (cbet * cbet) - c3);

            } else {

               // q(pole) - q(here) = g * d, where d = 1 - sin(lat) is
               // measured from the chosen pole and g is regular. d over
               // cos^2(beta) is regular there too.
               const long double s = poleSign * sphi;
               long double d, dOverCbet2;
               if (s > 0.0L) {
                  d = cphi * cphi / (1.0L + s);
                  dOverCbet2 = sq(f1_ / r) / (1.0L + s);
               } else {
                  d = 1.0L - s;
                  dOverCbet2 = d / (cbet * cbet);
               }
               const long double g1 =
                  (1.0L + e2_ * s) / (1.0L - e2_ * s * s) +
                  (1.0L - e2_) / 2.0L *
                  (log1pOverX(e_ * d / (1.0L + e_ * s)) / (1.0L + e_ * s) +
                   log1pOverX(e_ * d / (1.0L - e_)) / (1.0L - e_));
               value = poleSign * g1 * (dOverCbet2 - d * c3);
            }

            sum += rule.w[g] * value;
         }
      }

      total += l.sinAlpha0 * h * sum;
   }

   areaM2 = fabsl(total) * a_ * a_ / 2.0L;
   return std::isfinite(areaM2);
}

////////////////////////////////////////////////////////////////////////////////
