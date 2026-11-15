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

DgDVec3D auxiliarySphere (const DgGeoCoord& g)
{
   return DgDVec3D::fromLatLon(
      DgAuthalic::geodeticToAuthalicLatitude(g.lat()), g.lon());
}

} // namespace

// Area in m² of the geodesic triangle on WGS 84. Edges longer than 50 km are
// split along the geodesic; on a shorter edge the excess of the authalic
// images, times the square of the authalic radius, matches the geodesic
// area to about 1e-8 relative.
long double
DgWGS84RF::triangleAreaM2 (const DgGeoCoord& p, const DgGeoCoord& q,
                           const DgGeoCoord& r, int depth) const
{
   const DgGeoCoord verts[3] = {p, q, r};
   int longest = 0;
   long double longestM = -1.0L;
   long double longestAz = 0.0L;
   for (int i = 0; i < 3; ++i) {
      long double distanceM = 0.0L, azimuth = 0.0L;
      if (geodesicInverse(verts[i], verts[(i + 1) % 3], distanceM, azimuth) &&
          distanceM > longestM) {
         longest = i;
         longestM = distanceM;
         longestAz = azimuth;
      }
   }

   if (depth < 24 && longestM > 50000.0L) {
      const DgGeoCoord mid = geodesicDirect(verts[longest], longestAz,
                                            longestM / 2.0L);
      const DgGeoCoord& far = verts[(longest + 2) % 3];
      const DgGeoCoord& next = verts[(longest + 1) % 3];
      return triangleAreaM2(verts[longest], mid, far, depth + 1) +
             triangleAreaM2(mid, next, far, depth + 1);
   }

   const long double radiusM = DgAuthalic::authalicRadiusKM() * 1000.0L;
   return radiusM * radiusM *
      sphericalExcess(auxiliarySphere(p), auxiliarySphere(q),
                      auxiliarySphere(r));
}

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
      areaM2 += triangleAreaM2(c, v1, v2, 0);
   }

   return fabsl(areaM2) / 1.0e6L;
}
