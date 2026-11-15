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

#include <cmath>

#include <dglib/DgAuthalicConverter.h>
#include <dglib/DgDVec3D.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgWGS84RF.h>

namespace {

// Signed solid angle of the unit-sphere triangle abc, stable for small
// triangles. Positive when a, b, c are counterclockwise seen from outside.
long double sphericalExcess (const DgDVec3D& a, const DgDVec3D& b,
                             const DgDVec3D& c)
{
   const long double triple = a.dot(b.cross(c));
   const long double denom = 1.0L + a.dot(b) + b.dot(c) + c.dot(a);
   return 2.0L * atan2l(triple, denom);
}

void reducedLatitude (long double lat, long double f,
                      long double& sinU, long double& cosU)
{
   const long double sinLat = sinl(lat);
   const long double cosLat = cosl(lat);
   const long double norm = hypotl((1.0L - f) * sinLat, cosLat);
   sinU = (1.0L - f) * sinLat / norm;
   cosU = cosLat / norm;
}

// Vincenty inverse on WGS 84. A pole is given the other point's longitude,
// so the meridian from that pole is well defined. startLon is that longitude.
struct InverseGeodesic {
   bool ok;
   long double distanceM;
   long double azimuth;
   long double startLon;
};

InverseGeodesic vincentyInverse (DgGeoCoord from, DgGeoCoord to)
{
   const long double a = DgWGS84RF::semiMajorAxisMeters();
   const long double f = 1.0L / DgWGS84RF::inverseFlattening();
   const long double b = a * (1.0L - f);
   InverseGeodesic result{false, 0.0L, 0.0L, from.lon()};
   if (fabsl(fabsl(from.lat()) - M_PI_2) < 1.0e-14L) from.setLon(to.lon());
   if (fabsl(fabsl(to.lat()) - M_PI_2) < 1.0e-14L) to.setLon(from.lon());
   result.startLon = from.lon();

   const long double deltaLon = remainderl(to.lon() - from.lon(), M_2PI);
   long double sinU1, cosU1, sinU2, cosU2;
   reducedLatitude(from.lat(), f, sinU1, cosU1);
   reducedLatitude(to.lat(), f, sinU2, cosU2);
   long double lambda = deltaLon;
   for (int i = 0; i < 200; ++i) {
      const long double sinLambda = sinl(lambda);
      const long double cosLambda = cosl(lambda);
      const long double sinSigma = hypotl(cosU2 * sinLambda,
         cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
      if (sinSigma == 0.0L) {
         result.ok = true;
         return result;
      }
      const long double cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
      const long double sigma = atan2l(sinSigma, cosSigma);
      const long double sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
      const long double cos2Alpha = 1.0L - sinAlpha * sinAlpha;
      const long double cos2SigmaM = cos2Alpha > 0.0L
         ? cosSigma - 2.0L * sinU1 * sinU2 / cos2Alpha : 0.0L;
      const long double C = f / 16.0L * cos2Alpha *
         (4.0L + f * (4.0L - 3.0L * cos2Alpha));
      const long double next = deltaLon + (1.0L - C) * f * sinAlpha *
         (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma *
          (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM)));
      if (fabsl(next - lambda) < 1.0e-15L) {
         const long double u2 = cos2Alpha * (a * a - b * b) / (b * b);
         const long double A = 1.0L + u2 / 16384.0L *
            (4096.0L + u2 * (-768.0L + u2 * (320.0L - 175.0L * u2)));
         const long double B = u2 / 1024.0L *
            (256.0L + u2 * (-128.0L + u2 * (74.0L - 47.0L * u2)));
         const long double deltaSigma = B * sinSigma * (cos2SigmaM + B / 4.0L *
            (cosSigma * (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM) -
             B / 6.0L * cos2SigmaM * (-3.0L + 4.0L * sinSigma * sinSigma) *
             (-3.0L + 4.0L * cos2SigmaM * cos2SigmaM)));
         result.ok = true;
         result.distanceM = b * A * (sigma - deltaSigma);
         result.azimuth = atan2l(cosU2 * sinLambda,
            cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
         return result;
      }
      lambda = next;
   }
   return result;
}

// Point reached by travelling distanceM along azimuth from start.
DgGeoCoord vincentyDirect (const DgGeoCoord& start, long double azimuth,
                           long double distanceM)
{
   const long double a = DgWGS84RF::semiMajorAxisMeters();
   const long double f = 1.0L / DgWGS84RF::inverseFlattening();
   const long double b = a * (1.0L - f);
   long double sinU1, cosU1;
   reducedLatitude(start.lat(), f, sinU1, cosU1);
   const long double sigma1 = atan2l(sinU1, cosU1 * cosl(azimuth));
   const long double sinAlpha = cosU1 * sinl(azimuth);
   const long double cos2Alpha = 1.0L - sinAlpha * sinAlpha;
   const long double u2 = cos2Alpha * (a * a - b * b) / (b * b);
   const long double A = 1.0L + u2 / 16384.0L *
      (4096.0L + u2 * (-768.0L + u2 * (320.0L - 175.0L * u2)));
   const long double Bcoef = u2 / 1024.0L *
      (256.0L + u2 * (-128.0L + u2 * (74.0L - 47.0L * u2)));
   long double sigma = distanceM / (b * A);
   for (int i = 0; i < 30; ++i) {
      const long double twoSigmaM = 2.0L * sigma1 + sigma;
      const long double cos2SigmaM = cosl(twoSigmaM);
      const long double sinSigma = sinl(sigma);
      const long double cosSigma = cosl(sigma);
      const long double deltaSigma = Bcoef * sinSigma * (cos2SigmaM +
         Bcoef / 4.0L * (cosSigma * (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM) -
          Bcoef / 6.0L * cos2SigmaM * (-3.0L + 4.0L * sinSigma * sinSigma) *
          (-3.0L + 4.0L * cos2SigmaM * cos2SigmaM)));
      const long double next = distanceM / (b * A) + deltaSigma;
      if (fabsl(next - sigma) < 1.0e-15L) {
         sigma = next;
         break;
      }
      sigma = next;
   }
   const long double sinSigma = sinl(sigma);
   const long double cosSigma = cosl(sigma);
   const long double cos2SigmaM = cosl(2.0L * sigma1 + sigma);
   const long double lat2 = atan2l(
      sinU1 * cosSigma + cosU1 * sinSigma * cosl(azimuth),
      (1.0L - f) * hypotl(sinAlpha,
         sinU1 * sinSigma - cosU1 * cosSigma * cosl(azimuth)));
   const long double lambda = atan2l(sinSigma * sinl(azimuth),
      cosU1 * cosSigma - sinU1 * sinSigma * cosl(azimuth));
   const long double C = f / 16.0L * cos2Alpha *
      (4.0L + f * (4.0L - 3.0L * cos2Alpha));
   const long double L = lambda - (1.0L - C) * f * sinAlpha *
      (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma *
       (-1.0L + 2.0L * cos2SigmaM * cos2SigmaM)));
   return DgGeoCoord(remainderl(start.lon() + L, M_2PI), lat2);
}

// Area in m² of the geodesic triangle on WGS 84. Edges longer than 50 km are
// split; on a shorter edge the excess of the authalic images, times the
// square of the authalic radius, matches the geodesic area to about 1e-8.
long double ellipsoidalTriangleM2 (const DgGeoCoord& p, const DgGeoCoord& q,
                                   const DgGeoCoord& r, int depth)
{
   const DgGeoCoord verts[3] = {p, q, r};
   int longest = 0;
   long double longestM = -1.0L;
   InverseGeodesic longestInv{false, 0.0L, 0.0L, 0.0L};
   for (int i = 0; i < 3; ++i) {
      const InverseGeodesic inv = vincentyInverse(verts[i], verts[(i + 1) % 3]);
      if (inv.ok && inv.distanceM > longestM) {
         longest = i;
         longestM = inv.distanceM;
         longestInv = inv;
      }
   }

   if (depth < 24 && longestM > 50000.0L) {
      const DgGeoCoord start(longestInv.startLon, verts[longest].lat());
      const DgGeoCoord mid = vincentyDirect(start, longestInv.azimuth,
                                            longestM / 2.0L);
      const DgGeoCoord& far = verts[(longest + 2) % 3];
      const DgGeoCoord& next = verts[(longest + 1) % 3];
      return ellipsoidalTriangleM2(verts[longest], mid, far, depth + 1) +
             ellipsoidalTriangleM2(mid, next, far, depth + 1);
   }

   const long double radiusM = DgAuthalic::authalicRadiusKM() * 1000.0L;
   auto auxiliary = [](const DgGeoCoord& g) {
      return DgDVec3D::fromLatLon(
         DgAuthalic::geodeticToAuthalicLatitude(g.lat()), g.lon());
   };
   return radiusM * radiusM *
      sphericalExcess(auxiliary(p), auxiliary(q), auxiliary(r));
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
long double
DgWGS84RF::cellAreaKM (const DgLocation& center,
                        const DgPolygon& boundary) const
{
   DgLocation cLoc(center);
   convert(&cLoc);
   DgPolygon ring(boundary);
   convert(ring);
   if (ring.size() < 3)
      report("DgWGS84RF::cellAreaKM(): boundary has fewer than 3 vertices",
             DgBase::Fatal);

   const DgGeoCoord c = *getAddress(cLoc);
   long double areaM2 = 0.0L;
   for (int i = 0; i < ring.size(); ++i) {
      const DgGeoCoord v1 = *getAddress(ring[i]);
      const DgGeoCoord v2 = *getAddress(ring[(i + 1) % ring.size()]);
      areaM2 += ellipsoidalTriangleM2(c, v1, v2, 0);
   }

   return fabsl(areaM2) / 1.0e6L;
}
