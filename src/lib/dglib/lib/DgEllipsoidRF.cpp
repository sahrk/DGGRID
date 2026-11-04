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
// DgEllipsoidRF.cpp: DgEllipsoidRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgPolygon.h>
#include <algorithm>
#include <limits>

const DgGeoCoord DgGeoCoord::undefGeoCoord(LDBL_MAX, LDBL_MAX);
const long double DgGeoCoord::tolerance = 0.0000000005L;

namespace {

struct GeodesicState {
   long double r[3], v[3];
};

// Arc-length form of the geodesic equation on x²/a² + y²/a² + z²/b² = 1.
GeodesicState derivative(const GeodesicState& q, long double a, long double b)
{
   GeodesicState d;
   const long double invA2 = 1.0L / (a * a);
   const long double invB2 = 1.0L / (b * b);
   const long double scale =
      (invA2 * (q.v[0] * q.v[0] + q.v[1] * q.v[1]) +
       invB2 * q.v[2] * q.v[2]) /
      (invA2 * invA2 * (q.r[0] * q.r[0] + q.r[1] * q.r[1]) +
       invB2 * invB2 * q.r[2] * q.r[2]);
   for (int i = 0; i < 3; ++i) {
      d.r[i] = q.v[i];
      d.v[i] = -scale * q.r[i] * (i == 2 ? invB2 : invA2);
   }
   return d;
}

GeodesicState advance(const GeodesicState& q, const GeodesicState& d,
                      long double step)
{
   GeodesicState result;
   for (int i = 0; i < 3; ++i) {
      result.r[i] = q.r[i] + step * d.r[i];
      result.v[i] = q.v[i] + step * d.v[i];
   }
   return result;
}

GeodesicState endpoint(const GeodesicState& start, long double azimuth,
                       long double length, long double a, long double b,
                       const long double east[3], const long double north[3])
{
   GeodesicState q = start;
   for (int i = 0; i < 3; ++i)
      q.v[i] = sinl(azimuth) * east[i] + cosl(azimuth) * north[i];

   const int count = static_cast<int>(ceill(length / 10000.0L)) + 1;
   const long double step = length / count;
   for (int j = 0; j < count; ++j) {
      const GeodesicState k1 = derivative(q, a, b);
      const GeodesicState k2 = derivative(advance(q, k1, step / 2.0L), a, b);
      const GeodesicState k3 = derivative(advance(q, k2, step / 2.0L), a, b);
      const GeodesicState k4 = derivative(advance(q, k3, step), a, b);
      for (int i = 0; i < 3; ++i) {
         q.r[i] += step / 6.0L *
            (k1.r[i] + 2.0L * k2.r[i] + 2.0L * k3.r[i] + k4.r[i]);
         q.v[i] += step / 6.0L *
            (k1.v[i] + 2.0L * k2.v[i] + 2.0L * k3.v[i] + k4.v[i]);
      }
   }
   return q;
}

void pointAndBasis(const DgGeoCoord& p, long double a, long double b,
                   GeodesicState& point, long double east[3],
                   long double north[3])
{
   const long double sl = sinl(p.lat()), cl = cosl(p.lat());
   const long double so = sinl(p.lon()), co = cosl(p.lon());
   const long double e2 = 1.0L - b * b / (a * a);
   const long double N = a / sqrtl(1.0L - e2 * sl * sl);
   point.r[0] = N * cl * co;
   point.r[1] = N * cl * so;
   point.r[2] = N * (1.0L - e2) * sl;
   east[0] = -so; east[1] = co; east[2] = 0.0L;
   north[0] = -sl * co; north[1] = -sl * so; north[2] = cl;
}

void residual(const GeodesicState& start, long double azimuth,
              long double length, long double a, long double b,
              const long double sourceEast[3], const long double sourceNorth[3],
              const GeodesicState& target, const long double targetEast[3],
              const long double targetNorth[3], long double error[3])
{
   const GeodesicState end = endpoint(start, azimuth, length, a, b,
                                      sourceEast, sourceNorth);
   error[0] = error[1] = error[2] = 0.0L;
   for (int i = 0; i < 3; ++i) {
      const long double difference = end.r[i] - target.r[i];
      error[0] += difference * targetEast[i];
      error[1] += difference * targetNorth[i];
      error[2] += difference * difference;
   }
   error[2] = sqrtl(error[2]);
}

// Vincenty's fixed-point iteration may have no fixed point near antipodes.
// Shoot geodesics on the ellipsoid and keep the shortest converged solution.
long double antipodalDistance(const DgGeoCoord& p1, const DgGeoCoord& p2,
                              long double a, long double b)
{
   GeodesicState start = {}, target = {};
   long double sourceEast[3], sourceNorth[3], targetEast[3], targetNorth[3];
   pointAndBasis(p1, a, b, start, sourceEast, sourceNorth);
   pointAndBasis(p2, a, b, target, targetEast, targetNorth);
   const long double dlon = remainderl(p2.lon() - p1.lon(), M_2PI);
   const long double central = acosl(std::max(-1.0L, std::min(1.0L,
      sinl(p1.lat()) * sinl(p2.lat()) +
      cosl(p1.lat()) * cosl(p2.lat()) * cosl(dlon))));
   const long double guessLength = (2.0L * a + b) / 3.0L * central;
   const long double sphericalAzimuth = atan2l(
      cosl(p2.lat()) * sinl(dlon),
      cosl(p1.lat()) * sinl(p2.lat()) -
      sinl(p1.lat()) * cosl(p2.lat()) * cosl(dlon));
   const long double guesses[] = {sphericalAzimuth, 0.0L, M_PI,
                                  M_PI_2, -M_PI_2};
   long double best = std::numeric_limits<long double>::infinity();
   for (long double guess : guesses) {
      long double azimuth = guess, length = guessLength;
      for (int iteration = 0; iteration < 30; ++iteration) {
         long double error[3];
         residual(start, azimuth, length, a, b, sourceEast, sourceNorth,
                  target, targetEast, targetNorth, error);
         const long double norm = error[2];
         if (norm < 0.0001L) {
            best = std::min(best, length);
            break;
         }
         long double angleError[3], lengthError[3];
         residual(start, azimuth + 1.0e-5L, length, a, b,
                  sourceEast, sourceNorth, target, targetEast, targetNorth,
                  angleError);
         residual(start, azimuth, length + 10.0L, a, b,
                  sourceEast, sourceNorth, target, targetEast, targetNorth,
                  lengthError);
         const long double j00 = (angleError[0] - error[0]) / 1.0e-5L;
         const long double j10 = (angleError[1] - error[1]) / 1.0e-5L;
         const long double j01 = (lengthError[0] - error[0]) / 10.0L;
         const long double j11 = (lengthError[1] - error[1]) / 10.0L;
         const long double determinant = j00 * j11 - j01 * j10;
         if (fabsl(determinant) < 1.0e-8L) break;
         const long double da = (-error[0] * j11 + error[1] * j01) /
                                determinant;
         const long double ds = (-j00 * error[1] + j10 * error[0]) /
                                determinant;
         bool improved = false;
         for (long double fraction = 1.0L; fraction >= 1.0L / 128.0L;
              fraction /= 2.0L) {
            const long double candidateLength = length + fraction * ds;
            if (candidateLength <= 0.0L || candidateLength > M_PI * a * 1.01L)
               continue;
            long double candidateError[3];
            residual(start, azimuth + fraction * da, candidateLength, a, b,
                     sourceEast, sourceNorth, target, targetEast, targetNorth,
                     candidateError);
            if (candidateError[2] < norm) {
               azimuth += fraction * da;
               length = candidateLength;
               improved = true;
               break;
            }
         }
         if (!improved) break;
      }
   }
   return best;
}

} // namespace

long double
DgEllipsoidRF::dist(const DgGeoCoord& p1, const DgGeoCoord& p2) const
{
   // Vincenty's inverse solution. Axes are meters, coordinates are radians,
   // and the public distance unit for geographic reference frames is km.
   if (!std::isfinite(p1.lat()) || !std::isfinite(p1.lon()) ||
       !std::isfinite(p2.lat()) || !std::isfinite(p2.lon()) ||
       fabsl(p1.lat()) > M_PI_2 + 1.0e-15L ||
       fabsl(p2.lat()) > M_PI_2 + 1.0e-15L ||
       !std::isfinite(a()) || !std::isfinite(b()) ||
       !(a() > 0.0L) || !(b() > 0.0L) || b() > a()) {
      report("DgEllipsoidRF::dist(): invalid ellipsoid or coordinate", DgBase::Fatal);
      return 0.0L;
   }

   const long double deltaLon = remainderl(p2.lon() - p1.lon(), M_2PI);
   if ((p1.lat() == p2.lat() && deltaLon == 0.0L) ||
       (p1.lat() * p2.lat() > 0.0L &&
        fabsl(fabsl(p1.lat()) - M_PI_2) < 1.0e-15L &&
        fabsl(fabsl(p2.lat()) - M_PI_2) < 1.0e-15L))
      return 0.0L;

   const long double flattening = (a() - b()) / a();
   if (flattening == 0.0L) {
      const long double numerator = hypotl(
         cosl(p2.lat()) * sinl(deltaLon),
         cosl(p1.lat()) * sinl(p2.lat()) -
         sinl(p1.lat()) * cosl(p2.lat()) * cosl(deltaLon));
      const long double denominator = sinl(p1.lat()) * sinl(p2.lat()) +
         cosl(p1.lat()) * cosl(p2.lat()) * cosl(deltaLon);
      return a() * atan2l(numerator, denominator) / 1000.0L;
   }

   const long double U1 = atanl((1.0L - flattening) * tanl(p1.lat()));
   const long double U2 = atanl((1.0L - flattening) * tanl(p2.lat()));
   const long double sinU1 = sinl(U1), cosU1 = cosl(U1);
   const long double sinU2 = sinl(U2), cosU2 = cosl(U2);
   long double lambda = deltaLon;
   long double sinSigma = 0.0L, cosSigma = 0.0L, sigma = 0.0L;
   long double sinAlpha = 0.0L, cosSqAlpha = 0.0L, cos2SigmaM = 0.0L;
   bool converged = false;

   for (int i = 0; i < 200; ++i) {
      const long double sinLambda = sinl(lambda), cosLambda = cosl(lambda);
      sinSigma = hypotl(cosU2 * sinLambda,
         cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
      if (sinSigma == 0.0L) return 0.0L;
      cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
      sigma = atan2l(sinSigma, cosSigma);
      sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
      cosSqAlpha = 1.0L - sinAlpha * sinAlpha;
      cos2SigmaM = cosSqAlpha > 0.0L
         ? cosSigma - 2.0L * sinU1 * sinU2 / cosSqAlpha : 0.0L;
      const long double C = flattening / 16.0L * cosSqAlpha *
         (4.0L + flattening * (4.0L - 3.0L * cosSqAlpha));
      const long double next = deltaLon + (1.0L - C) * flattening *
         sinAlpha * (sigma + C * sinSigma *
         (cos2SigmaM + C * cosSigma *
         (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM)));
      if (fabsl(next - lambda) < 1.0e-15L) {
         converged = true;
         break;
      }
      lambda = next;
   }

   if (!converged) {
      const long double fallback = antipodalDistance(p1, p2, a(), b());
      if (std::isfinite(fallback)) return fallback / 1000.0L;
      report("DgEllipsoidRF::dist(): inverse geodesic did not converge",
             DgBase::Fatal);
      return 0.0L;
   }

   const long double uSq = cosSqAlpha * (a() * a() - b() * b()) /
                            (b() * b());
   const long double A = 1.0L + uSq / 16384.0L *
      (4096.0L + uSq * (-768.0L + uSq * (320.0L - 175.0L * uSq)));
   const long double B = uSq / 1024.0L *
      (256.0L + uSq * (-128.0L + uSq * (74.0L - 47.0L * uSq)));
   const long double deltaSigma = B * sinSigma *
      (cos2SigmaM + B / 4.0L *
      (cosSigma * (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM) -
       B / 6.0L * cos2SigmaM * (-3.0L + 4.0L * sinSigma * sinSigma) *
       (-3.0L + 4.0L * cos2SigmaM * cos2SigmaM)));
   return b() * A * (sigma - deltaSigma) / 1000.0L;
}

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::gcDist (const DgGeoCoord& g1, const DgGeoCoord& g2,
                    bool rads)
/*
   return great circle distance in radians between g1 and g2.

   CRC Math, 1991, pp. 129-130.

   Kevin Sahr, 3/10/99
*/
{
   // use spherical triangle with g1 at A, g2 at B, and north pole at C

   long double bigC = fabsl(g2.lon() - g1.lon());

   if (bigC > M_PI) // assume we want the complement
   {
      // note that in this case they can't both be negative

      long double lon1 = g1.lon();
      if (lon1 < 0.0L) lon1 += 2.0L * M_PI;
      long double lon2 = g2.lon();
      if (lon2 < 0.0L) lon2 += 2.0L * M_PI;

      bigC = fabsl(lon2 - lon1);
   }

   long double b = M_PI_2 - g1.lat();
   long double a = M_PI_2 - g2.lat();

   // use law of cosines to find c

   long double cosc = cosl(a) * cosl(b) + sinl(a) * sinl(b) * cosl(bigC);
   if (cosc > 1.0L) cosc = 1.0L;
   if (cosc < -1.0L) cosc = -1.0L;

   long double retVal = acosl(cosc);

   if (!rads) retVal *= M_180_PI;

   return retVal;

} // long double DgGeoCoord::gcDist

////////////////////////////////////////////////////////////////////////////////
DgGeoCoord
DgGeoCoord::gcIntersect (const DgGeoCoord& g11, const DgGeoCoord& g12,
          const DgGeoCoord& g21, const DgGeoCoord& /* g22 */)
/*
   Return point of intersection of the two great circle arc segments g11-g12
   and g21-g22.

   Works by calling Lian Song's routine GCintersect.
*/
{
   GeoCoord sv11, sv12, sv21;
   sv11.lon = g11.lon(); sv11.lat = g11.lat();
   sv12.lon = g12.lon(); sv12.lat = g12.lat();
   sv21.lon = g21.lon(); sv21.lat = g21.lat();
   //sv22.lon = g22.lon(); sv22.lat = g22.lat();

   GeoCoord ans = GCintersect(sv11, sv12, sv21, sv11, 1);

   return DgGeoCoord(ans.lon, ans.lat);

} // DgGeoCoord DgGeoCoord::gcIntersect

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::geoTriArea (const DgGeoCoord& g1, const DgGeoCoord& g2,
                        const DgGeoCoord& g3)
/*
   return area in radians.

   CRC Math, 1991, pp. 129-130.

   Kevin Sahr, 3/10/99
*/
{
   // determine the edges

   long double a = DgGeoCoord::gcDist(g2, g3);
   long double b = DgGeoCoord::gcDist(g1, g3);
   long double c = DgGeoCoord::gcDist(g1, g2);

   // determine the angles using half-angle formulas

   long double s = (a + b + c) / 2.0L;

   long double sinsa = sinl(s - a);
   long double sinsb = sinl(s - b);
   long double sinsc = sinl(s - c);

   long double k = sqrtl(sinsa * sinsb * sinsc / sinl(s));

   long double bigA = 2.0L * atanl(k / sinsa);
   long double bigB = 2.0L * atanl(k / sinsb);
   long double bigC = 2.0L * atanl(k / sinsc);

   long double E = bigA + bigB + bigC - M_PI;

/*
   std::cout << "geoTriArea: " << g1 << " " << g2 << " " << g3 << std::endl
        << " a: " << a
        << " b: " << b
        << " c: " << c << std::endl
        << " A: " << bigA
        << " B: " << bigB
        << " C: " << bigC << std::endl
        << " E: " << E << " "
        << E / (4.0L * M_PI) << std::endl;
*/

   return E;

} // long double DgGeoCoord::geoTriArea

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::geoPolyArea (const DgPolygon& poly, const DgGeoCoord center)
//
// returns area of spherical polygon in radians;
//
// assumes DgAddressBase for poly is DgGeoCoord
//
// Assumes poly is a "reasonably" convex polygon (i.e. a cell bouncary) and
// that center is an interior point.
//
{
   long double totArea = 0.0L;

   const DgGeoSphRF* geoRF = dynamic_cast<const DgGeoSphRF*>(&poly.rf());
   if (geoRF == 0)
      report("DgGeoCoord::geoPolyArea() non-geo polygon", DgBase::Fatal);

   // do each sub-triangle
   for (int i = 0; i < poly.size(); i++)
   {
      const DgGeoCoord& v1 = *geoRF->getAddress(poly[i]);
      const DgGeoCoord& v2 = *geoRF->getAddress(poly[(i + 1) % poly.size()]);

      totArea += DgGeoCoord::geoTriArea(center, v1, v2);
   }

   return totArea;

} // long double DgGeoCoord::geoPolyArea

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char*
DgEllipsoidRF::str2add (DgGeoCoord* add, const char* str, char delimiter) const
{
   if (!add) add = new DgGeoCoord();

   DgDVec2D vec;
   const char* tmp = vec.fromString(str, delimiter);

   *add = DgGeoCoord(vec.x(), vec.y(), false);

   return tmp;

} // const char* DgEllipsoidRF::str2add

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*
   sphTrig.C: vector and spherical trig utilities

   Lian Song and Kevin Sahr, July 24, 1994.

   Kevin Sahr, merged & optimized, July 20, 2001
*/
/******************************************************************************/

#include <dglib/DgBase.h>

#include <cmath>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************/
void sphTriInit (SphTri* tri)
/*
   Initialize tri with UNDEFVAL values.
*/
{
   int i;

   for (i = 0; i < 3; i++)
   {
      tri->verts[i].lon  = tri->verts[i].lat  = UNDEFVAL;
      tri->edges[i] = tri->angles[i] = UNDEFVAL;
   }
   tri->code = UNDEFINT;
   tri->area = tri->perimeter = UNDEFVAL;
   tri->compactness = UNDEFVAL;

} /* void sphTriInit */

/******************************************************************************/
void planeTriInit (PlaneTri* tri)
/*
   Initialize tri with UNDEFVAL values.
*/
{
   int i;

   for (i = 0; i < 3; i++)
      tri->points[i].x  = tri->points[i].y = UNDEFVAL;
   tri->code = tri->direction =  UNDEFINT;
   tri->cenpoint.x = tri->cenpoint.y = UNDEFVAL;

} /* void planeTriInit */

/******************************************************************************/
void printInt (long long int val)
/*
   Print val or 'UNDEFVAL' to stdout as appropriate.
*/
{
   if (val == UNDEFVAL)
      dgcout << "UNDEFVAL";
   else
      dgcout << val;

} /* void printInt */

/******************************************************************************/
void printVal (long double val)
/*
   Print val or 'UNDEFVAL' to stdout as appropriate.
*/
{
   if (val == UNDEFVAL)
      dgcout << "UNDEFVAL";
   else
      dgcout << val;

} /* void printVal */

/******************************************************************************/
void printGeoCoord (const GeoCoord& p)
/*
   Print p to stdout.
*/
{
   dgcout << "(" << p.lon * M_180_PI << ", " << p.lat * M_180_PI << ")";

} /* void printGeoCoord */

/******************************************************************************/
void printVec2D (const Vec2D& p)
/*
   Print p to stdout.
*/
{
   dgcout << "(" << p.x << ", " << p.y << ")";

} /* void printVec2D */

/******************************************************************************/
void printSphTri (const SphTri& tri)
/*
   Print tri to stdout.
*/
{
   int i;

   dgcout << "{\n  code: ";
   printInt(tri.code);
   dgcout << "\n  vertices: ";
   for (i = 0; i < 3; i++)
   {
      dgcout << " ";
      printGeoCoord(tri.verts[i]);
   }
   dgcout << "\n";

   dgcout << "  A: "; printVal(tri.edges[0]);
   dgcout << "  B: "; printVal(tri.edges[1]);
   dgcout << "  C: "; printVal(tri.edges[2]);
   dgcout << "\n";

   dgcout << "  a: "; printVal(tri.angles[0]* M_180_PI);
   dgcout << "  b: "; printVal(tri.angles[1]* M_180_PI);
   dgcout << "  c: "; printVal(tri.angles[2]* M_180_PI);
   dgcout << "\n";

   dgcout << "  area: "; printVal(tri.area);
   dgcout << "  perimeter: "; printVal(tri.perimeter);
   dgcout << "  compactness: "; printVal(tri.compactness);
   dgcout << "\n";

   dgcout << "}\n";

} /* void printSphTri */

/******************************************************************************/
void printPlaneTri (const PlaneTri& tri)
/*
   Print tri to stdout.
*/
{
   int i;

   dgcout << "{\n  code: "; printInt(tri.code);
   dgcout << "\n  vertices: ";
   for (i = 0; i < 3; i++)
   {
      dgcout << " ";
      printVec2D(tri.points[i]);
   }
   dgcout << "\n";
   printVec2D(tri.cenpoint);
   dgcout << "\n";

   dgcout << "}\n";

} /* void PlaneTri */

/******************************************************************************/
long double maxval(long double val1, long double val2)
/*
  return the maxmum of two variables
*/
 {
  long double maxx;
  if (val1>val2) maxx=val1;
  else maxx=val2;
  return maxx;
 } /* long double maxval */

/******************************************************************************/
long double minval(long double val1, long double val2)
/*
  return the minmum of two variables
*/
 {
  long double minn;
  if (val1<val2) minn=val1;
  else minn=val2;
  return minn;
 } /* long double minval */

/******************************************************************************/
GeoCoord sphTricenpoint(GeoCoord sp[3])
/*
   Calculate and return the center point of a sphere triangle
   Input and output unit is degree.
*/
 {
  DgDVec3D cp = DgDVec3D::fromGeo(sp[0]) + DgDVec3D::fromGeo(sp[1]) +
                DgDVec3D::fromGeo(sp[2]);
  cp /= 3.0L;
  return cp.normalized().toGeo();
 } /* GeoCoord sphTricenpoint */

/******************************************************************************/
long double chorddist(const GeoCoord& ll1, const GeoCoord& ll2)
/*
   Calculate the chord distance between two points on the sphere
   Input unit is degree. Output unit regard the earth's radius as one unit.
*/
 {
  return DgDVec3D::fromGeo(ll1).distance(DgDVec3D::fromGeo(ll2));

 } /* long double chorddist */

/******************************************************************************/
long double spheredist(const GeoCoord& ll1, const GeoCoord& ll2)
/*
   Calculate the sphere distance between two points on the sphere
   Input and Output unit is in radius.
*/
 {
  long double sd;
  sd=cosl(M_PI_2-ll1.lat)*cosl(M_PI_2-ll2.lat)+
     sinl(M_PI_2-ll1.lat)*sinl(M_PI_2-ll2.lat)*cosl(ll1.lon-ll2.lon);
  if (sd>1) sd=1;
  if (sd<-1) sd=-1;
  return acosl(sd);
 }/* long double spheredist */

/******************************************************************************/
GeoCoord GCmidpoint(const GeoCoord& pp1, const GeoCoord& pp2)
{
  DgDVec3D mpt = DgDVec3D::fromGeo(pp1) + DgDVec3D::fromGeo(pp2);
  mpt /= 2.0L;
  return mpt.normalized().toGeo();
}

/******************************************************************************/
long double Azimuth(const GeoCoord& pt1, const GeoCoord& pt2)
/*
  return the azimuth of pt2 relatived to pt1
  output azimuth unit is radius
*/
 {
  long double az;

  az=atan2l(cosl(pt2.lat)*sinl(pt2.lon-pt1.lon), cosl(pt1.lat)*sinl(pt2.lat)-
     sinl(pt1.lat)*cosl(pt2.lat)*cosl(pt2.lon-pt1.lon));
  return az;
 } /* long double Azimuth */

/******************************************************************************/
GeoCoord GCintersect(const GeoCoord& sv11, const GeoCoord& sv12,
                     const GeoCoord& sv21, const GeoCoord& sv22, int sign)
/*
  return the intersect point of two great circle
  sign=1: two great circle segment with ends of sv11 and sv12,
          sv21 and sv22 respectively
  sign=0: two whole great circle with one pass sv11 and sv12, one pass
          sv21 and sv22, return the intersect point on North Hemisphere
*/
 {
  GeoCoord pt;
  long double a,b,maxlon,minlon;

/* calculate the intersect point of two great circle */
  const DgDVec3D nn1 = DgDVec3D::fromGeo(sv11).cross(DgDVec3D::fromGeo(sv12));
  const DgDVec3D nn2 = DgDVec3D::fromGeo(sv21).cross(DgDVec3D::fromGeo(sv22));
  long double ppx = 0.0L, ppy = 0.0L, ppz = 0.0L;
  if ((nn2.z()*nn1.y()-nn1.z()*nn2.y())!= 0.0L) {
    b=(nn1.x()*nn2.y()-nn2.x()*nn1.y())/(nn2.z()*nn1.y()-nn1.z()*nn2.y());
    a=(nn2.x()*nn1.z()-nn1.x()*nn2.z())/(nn1.y()*nn2.z()-nn2.y()*nn1.z());
    ppx=1/sqrtl(a*a+b*b+1);
    ppy=a*ppx;
    ppz=b*ppx;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) &&
         ((nn1.x()*nn2.y()-nn2.x()*nn1.y())==0.0L) && ((nn1.x()*nn2.z()-nn2.x()*nn1.z())==0.0L)) {
    report("Error in GCintersect: the two great circle planes are parallel.\n",
           DgBase::Fatal);
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn1.z()!=0.0L)) {
    ppx=0.0L;
    ppy=1.0L/sqrtl(1+nn1.y()*nn1.y()/nn1.z()/nn1.z());
    ppz=-nn1.y()/nn1.z()*ppy;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn2.z()!=0.0L)) {
    ppx=0.0L;
    ppy=1.0L/sqrtl(1.0L+nn2.y()*nn2.y()/nn2.z()/nn2.z());
    ppz=-nn2.y()/nn2.z()*ppy;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn1.y()!=0.0L)) {
    ppx=0.0L;
    ppz=1/sqrtl(1+nn1.z()*nn1.z()/nn1.y()/nn1.y());
    ppy=-nn1.z()/nn1.y()*ppz;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn2.y()!=0.0L)) {
    ppx=0.0L;
    ppz=1.0L/sqrtl(1.0L+nn2.z()*nn2.z()/nn2.y()/nn2.y());
    ppy=-nn2.z()/nn2.y()*ppz;
   }
  DgDVec3D pp(ppx, ppy, ppz);

  if (sign==0) {
    if (pp.z()<0.0L) pp *= -1.0L;
    pt=pp.toGeo();
    return pt;
  } else {
   /* judge if the point is on the two great circle segment */

  pt=pp.toGeo();
  maxlon=maxval(sv11.lon,sv12.lon);
  minlon=minval(sv11.lon,sv12.lon);
  if ((pt.lon<=maxlon) && (pt.lon>=minlon))
   return pt;
  else {
    pt=(pp * -1.0L).toGeo();
    if ((pt.lon<=maxlon) && (pt.lat>=minlon))
     return pt;
    else {
      dgcerr << "Error of GCintersect: the point is not on great circle segment.\n";
      pt.lat=UNDEFVAL; pt.lon=UNDEFVAL;
      return pt;
    }
  }
  }
} /* GeoCoord GCintersect */

/******************************************************************************/
long double GCptlat(long double lon, const GeoCoord& sv1, const GeoCoord& sv2)
/*
  return latitude of the point on great circle segment with known longtitude
*/
 {
  long double lat,a,b,c;
  const DgDVec3D p1 = DgDVec3D::fromGeo(sv1);
  const DgDVec3D p2 = DgDVec3D::fromGeo(sv2);

  a=(p1.y()*p2.z()-p1.z()*p2.y())*cosl(lon);
  b=(p1.x()*p2.z()-p1.z()*p2.x())*sinl(lon);
  c=(p1.x()*p2.y()-p1.y()*p2.x());
  if (c!=0.0L) lat=atanl((b-a)/c);
  else {
     lat = UNDEFVAL;
     dgcerr << "Error in GCptlat: the two end points are at one longitude.\n";
  }
  return(lat);
 } /* long double GCptlat */

/******************************************************************************/
int ptinsphtri(const GeoCoord& pt, GeoCoord sTri[3])
/*
  decide if a point is in a spherical triangle
  return 1 if it is in and return 0 if not
*/
{
   // pt is on the same side as each vertex of the plane through the other
   // two vertices (and the centre): compare the signs of the triple products
   DgDVec3D ptri[3];
   for (int i = 0; i < 3; i++) ptri[i] = DgDVec3D::fromGeo(sTri[i]);
   const DgDVec3D pp = DgDVec3D::fromGeo(pt);

   for (int k = 0; k < 3; k++)
   {
      const DgDVec3D n = ptri[(k + 1) % 3].cross(ptri[(k + 2) % 3]);
      if (pp.dot(n) * ptri[k].dot(n) < 0.0L) return 0;
   }

   return 1;

}  /* ptinsphtri */

/******************************************************************************/
GeoCoord GCdaz(const GeoCoord& pt, long double distance, long double az)
/*
  compute and return the point whose azimuth (az) and distance
  (distance) relative to a known point (pt) are given.
  all angles are in radians.

  The point is p cos(distance) + (n cos(az) + e sin(az)) sin(distance), with
  p the unit vector of pt and n, e the unit north and east tangents at p.
  These stay defined at a pole (north is then along the meridian pt.lon), so
  paths along a meridian, over a pole or from a pole need no special case.
*/
 {
  const long double sinLat = sinl(pt.lat), cosLat = cosl(pt.lat);
  const long double sinLon = sinl(pt.lon), cosLon = cosl(pt.lon);

  const DgDVec3D p(cosLat * cosLon, cosLat * sinLon, sinLat);
  const DgDVec3D n(-sinLat * cosLon, -sinLat * sinLon, cosLat);
  const DgDVec3D e(-sinLon, cosLon, 0.0L);

  const DgDVec3D q = p * cosl(distance) +
                     (n * cosl(az) + e * sinl(az)) * sinl(distance);

  return q.toGeo();
 } /* GeoCoord GCdaz */

/******************************************************************************/
