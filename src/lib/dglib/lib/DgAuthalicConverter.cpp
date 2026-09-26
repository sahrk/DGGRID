/*******************************************************************************
    Copyright (C) 2026 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/

#include <dglib/DgAuthalicConverter.h>

#include <cmath>
#include <limits>

namespace {

const long double halfPi = M_2PI / 4.0L;

long double eccentricitySquared()
{
   const long double f = 1.0L / DgWGS84RF::inverseFlattening();
   return f * (2.0L - f);
}

long double eccentricity()
{
   return sqrtl(eccentricitySquared());
}

// Snyder's meridional-area function q(phi), normalized by a squared.
long double q(long double phi)
{
   const long double es = eccentricitySquared();
   const long double e = eccentricity();
   const long double s = sinl(phi);
   return (1.0L - es) *
          (s / (1.0L - es * s * s) + atanhl(e * s) / e);
}

long double qp()
{
   const long double es = eccentricitySquared();
   const long double e = eccentricity();
   return 1.0L + (1.0L - es) * atanhl(e) / e;
}

long double qDerivative(long double phi)
{
   const long double es = eccentricitySquared();
   const long double s = sinl(phi);
   const long double denom = 1.0L - es * s * s;
   return 2.0L * (1.0L - es) * cosl(phi) / (denom * denom);
}

// q(pi/2) - q(phi) without subtracting two nearly equal numbers. Express
// 1 - sin(phi) as 2 sin^2((pi/2 - phi)/2) to retain near-pole precision.
long double polarAreaDeficit(long double colatitude)
{
   const long double es = eccentricitySquared();
   const long double e = eccentricity();
   const long double h = sinl(colatitude / 2.0L);
   const long double d = 2.0L * h * h;
   const long double s = 1.0L - d;
   const long double rational = d * (1.0L + es * s) /
                                ((1.0L - es) * (1.0L - es * s * s));
   const long double atanhDifference =
      (log1pl(e * d / (1.0L + e * s)) +
       log1pl(e * d / (1.0L - e))) / 2.0L;
   return (1.0L - es) * (rational + atanhDifference / e);
}

long double polarAreaDerivative(long double colatitude)
{
   const long double es = eccentricitySquared();
   const long double s = cosl(colatitude);
   const long double denom = 1.0L - es * s * s;
   return 2.0L * (1.0L - es) * sinl(colatitude) /
          (denom * denom);
}

void checkLatitude(long double latitude, const char* method)
{
   if (!std::isfinite(latitude) || fabsl(latitude) > halfPi)
      report(std::string(method) + ": latitude must be finite and within "
             "[-pi/2, pi/2] radians", DgBase::Fatal);
}

void checkLongitude(long double longitude, const char* method)
{
   if (!std::isfinite(longitude))
      report(std::string(method) + ": longitude must be finite", DgBase::Fatal);
}

} // namespace

long double DgAuthalic::geodeticToAuthalicLatitude(long double phi)
{
   checkLatitude(phi, "DgAuthalic::geodeticToAuthalicLatitude");
   if (phi == 0.0L || fabsl(phi) == halfPi) return phi;

   const long double absPhi = fabsl(phi);
   long double beta;
   if (absPhi < halfPi / 2.0L) {
      beta = asinl(q(absPhi) / qp());
   } else {
      const long double deficit = polarAreaDeficit(halfPi - absPhi) / qp();
      beta = halfPi - 2.0L * asinl(sqrtl(deficit / 2.0L));
   }
   return copysignl(beta, phi);
}

long double DgAuthalic::authalicToGeodeticLatitude(long double beta)
{
   checkLatitude(beta, "DgAuthalic::authalicToGeodeticLatitude");
   if (beta == 0.0L || fabsl(beta) == halfPi) return beta;

   const long double absBeta = fabsl(beta);
   // Solve q(phi) near the equator and its complement near the poles.
   // Subtracting the resulting colatitude from pi/2 would round small
   // equatorial latitudes to zero, even when beta itself is representable.
   const bool usePolarComplement = absBeta > halfPi / 2.0L;
   const long double h = sinl((halfPi - absBeta) / 2.0L);
   const long double target = usePolarComplement
      ? qp() * 2.0L * h * h : qp() * sinl(absBeta);
   long double low = 0.0L;
   long double high = halfPi;
   long double t = usePolarComplement ? halfPi - absBeta : absBeta;
   const long double tolerance =
      8.0L * std::numeric_limits<long double>::epsilon();
   bool converged = false;

   // Both q(phi) and the polar area deficit are monotone. Keep Newton's
   // method bracketed so the solve remains reliable over the whole domain.
   for (int iteration = 0; iteration < 64; ++iteration) {
      const long double residual = (usePolarComplement
         ? polarAreaDeficit(t) : q(t)) - target;
      if (fabsl(residual) <= tolerance * target) {
         converged = true;
         break;
      }

      const long double derivative = usePolarComplement
         ? polarAreaDerivative(t) : qDerivative(t);
      if (!std::isfinite(derivative) || derivative <= 0.0L) break;
      const long double correction = residual / derivative;
      // The residual can stop shrinking once q rounds to the nearest
      // representable value. In that case the inferred angular correction
      // gives a meaningful stopping rule in the variable being solved.
      if (std::isfinite(correction) &&
          fabsl(correction) <= tolerance * t) {
         converged = true;
         break;
      }

      if (residual < 0.0L) low = t;
      else high = t;

      long double next = t - correction;
      if (!std::isfinite(next) || next <= low || next >= high)
         next = (low + high) / 2.0L;
      if (next == t) break;
      t = next;
   }

   if (!converged) {
      report("DgAuthalic::authalicToGeodeticLatitude: inverse did not "
             "converge", DgBase::Fatal);
      return std::numeric_limits<long double>::quiet_NaN();
   }

   return copysignl(usePolarComplement ? halfPi - t : t, beta);
}

long double DgAuthalic::authalicRadiusKM()
{
   return DgWGS84RF::semiMajorAxisMeters() * sqrtl(qp() / 2.0L) / 1000.0L;
}

DgGeoCoord DgGeodeticToAuthalicConverter::convertTypedAddress(
   const DgGeoCoord& address) const
{
   checkLongitude(address.lon(), "DgGeodeticToAuthalicConverter");
   return DgGeoCoord(address.lon(),
                     DgAuthalic::geodeticToAuthalicLatitude(address.lat()));
}

DgGeoCoord DgAuthalicToGeodeticConverter::convertTypedAddress(
   const DgGeoCoord& address) const
{
   checkLongitude(address.lon(), "DgAuthalicToGeodeticConverter");
   return DgGeoCoord(address.lon(),
                     DgAuthalic::authalicToGeodeticLatitude(address.lat()));
}

Dg2WayAuthalicConverter::Dg2WayAuthalicConverter(
   const DgWGS84RF& ellipsoid, const DgGeoSphRF& sphere)
   : Dg2WayConverter(*(new DgGeodeticToAuthalicConverter(ellipsoid, sphere)),
                     *(new DgAuthalicToGeodeticConverter(sphere, ellipsoid)))
{
   if (fabsl(sphere.a() / 1000.0L - DgAuthalic::authalicRadiusKM()) >
       1.0e-9L)
      report("Dg2WayAuthalicConverter: sphere must have the WGS 84 "
             "authalic radius", DgBase::Fatal);
}
