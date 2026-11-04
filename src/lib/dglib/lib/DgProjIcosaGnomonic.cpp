/*******************************************************************************
    Copyright (C) 2026 DGGRID contributors

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
// DgProjIcosaGnomonic.cpp: DgProjIcosaGnomonic class implementation
//
// Portions Copyright 2016-2023, 2026 Uber Technologies, Inc., licensed under
// the Apache License, Version 2.0 (see LICENSE.H3); modified by DGGRID
// contributors.
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>

#include <dglib/DgProjIcosaGnomonic.h>

namespace {

const long double EPSILON = 0.0000000000000001L;
const long double RES0_U_GNOMONIC = 0.38196601125010500003L;
const long double INV_RES0_U_GNOMONIC = 2.61803398874989588842L;

/**
 * Encodes a coordinate on the sphere as 2D hex coordinates relative to the
 * center of its icosahedral face.
 *
 * p is expected to be on the unit sphere.
 *
 * @param p The coordinates to encode.
 * @param face The icosahedral face containing the coordinates
 *             (DgSphIcosa::whichIcosaTri).
 * @param sqd The squared euclidean distance from p to the center of face.
 * @param v Output: The 2D hex coordinates (resolution 0, class II) of the
 *          point.
 */
// Resolution 0 only (DGGRID's own lattices handle resolution). theta is
// measured counter-clockwise from the face's u axis using the face basis; a
// north/east tangent basis fails when a face centre is at a pole (DGGRID
// allows any icosahedron orientation).
void _vec3ToHex2d(const DgProjIcosaGnomonicFaces& faces, const DgDVec3D& p,
                  int face, long double sqd, DgDVec2D *v) {
    // cos(r) = 1 - 2 * sin^2(r/2) = 1 - 2 * (sqd / 4) = 1 - sqd/2
    // r = 2 asin(sqrt(sqd) / 2) is identical but numerically stable;
    // acos(1 - sqd/2) loses precision near the face centre.
    long double r = 2.0L * asinl(sqrtl(sqd) / 2.0L);

    if (r < EPSILON) {
        *v = DgDVec2D(0.0L, 0.0L);
        return;
    }

    // now have face and r, now find CCW theta from the face's i axis
    const DgDVec3D& c = faces.cen[face];
    const DgDVec3D q = p - c * p.dot(c);
    long double theta = atan2l(q.dot(faces.wax[face]), q.dot(faces.uax[face]));

    // perform gnomonic scaling of r
    r = tanl(r);

    // scale for current resolution length u
    r *= INV_RES0_U_GNOMONIC;

    // we now have (r, theta) in hex2d with theta ccw from x-axes

    // convert to local x,y
    *v = DgDVec2D(r * cosl(theta), r * sinl(theta));
}

/**
 * Determines the 3D coordinates of a point given by 2D hex coordinates on a
 * particular icosahedral face.
 *
 * @param v The 2D hex coordinates (resolution 0, class II).
 * @param face The icosahedral face upon which the 2D hex coordinate system is
 *             centered.
 * @return the 3D coordinates of the point
 */
// Resolution 0 only, using the face basis (see _vec3ToHex2d)
DgDVec3D _hex2dToVec3(const DgProjIcosaGnomonicFaces& faces, const DgDVec2D& v,
                      int face) {
    // calculate (r, theta) in hex2d
    long double r = v.magnitude();

    const DgDVec3D& c = faces.cen[face];
    if (r < EPSILON) return c;

    long double theta = atan2l(v.y(), v.x());

    r *= RES0_U_GNOMONIC;

    // perform inverse gnomonic scaling of r
    r = atanl(r);

    // now find the point at (r,theta) from the face center
    const DgDVec3D dir = faces.uax[face] * cosl(theta) +
                         faces.wax[face] * sinl(theta);

    return (c * cosl(r) + dir * sinl(r)).normalized();
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
DgProjIcosaGnomonicFaces::DgProjIcosaGnomonicFaces (const SphIcosa& si)
{
   for (int t = 0; t < 20; t++) {
      cen[t] = DgDVec3D::fromGeo(si.triCen[t].pt);
      const DgDVec3D v0 = DgDVec3D::fromGeo(si.icotri[t][0]);

      // in-plane direction from the face centre to V0
      uax[t] = (v0 - cen[t] * v0.dot(cen[t])).normalized();
      wax[t] = cen[t].cross(uax[t]);
   }
}

////////////////////////////////////////////////////////////////////////////////
DgDVec2D
DgProjIcosaGnomonic::projTriToHex2d0 (const DgDVec2D& p)
{
   const long double s3 = sqrtl(3.0L);

   return DgDVec2D(2.0L * s3 * (p.y() - s3 / 6.0L), 2.0L * s3 * (0.5L - p.x()));
}

////////////////////////////////////////////////////////////////////////////////
DgDVec2D
DgProjIcosaGnomonic::hex2d0ToProjTri (const DgDVec2D& h)
{
   const long double s3 = sqrtl(3.0L);

   return DgDVec2D(0.5L - h.y() / (2.0L * s3), s3 / 6.0L + h.x() / (2.0L * s3));
}

////////////////////////////////////////////////////////////////////////////////
DgProjIcosaGnomonicInv::DgProjIcosaGnomonicInv (
                       const DgRF<DgProjTriCoord, long double>& from,
                       const DgRF<DgGeoCoord, long double>& to)
         : DgConverter<DgProjTriCoord, long double, DgGeoCoord, long double>(from, to),
           pProjTriRF_ (0)
{
   pProjTriRF_ = dynamic_cast<const DgProjTriRF*>(&fromFrame());

   if (!pProjTriRF_)
   {
      report("DgProjIcosaGnomonicInv::DgProjIcosaGnomonicInv(): "
        " fromFrame not of type DgProjTriRF", DgBase::Fatal);
   }

   faces_ = DgProjIcosaGnomonicFaces(projTriRF().sphIcosa().sphIcosa());

} // DgProjIcosaGnomonicInv::DgProjIcosaGnomonicInv

////////////////////////////////////////////////////////////////////////////////
DgGeoCoord
DgProjIcosaGnomonicInv::convertTypedAddress (const DgProjTriCoord& addIn) const
{
   if (addIn.triNum() < 0 || addIn.triNum() > 19)
   {
      report("DgProjIcosaGnomonicInv::convertTypedAddress(): invalid triangle " +
             std::string(addIn), DgBase::Fatal);
   }

   // the gnomonic inverse is defined on the whole plane, so a point outside
   // its triangle (such as the vertex of a cell that crosses a face edge) is
   // fine
   const DgDVec3D p = _hex2dToVec3(faces_,
         DgProjIcosaGnomonic::projTriToHex2d0(addIn.coord()), addIn.triNum());

   DgGeoCoord geoPt(p.lon(), p.lat());
   geoPt.normalize();

   return geoPt;

} // DgGeoCoord DgProjIcosaGnomonicInv::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgProjIcosaGnomonicFwd::DgProjIcosaGnomonicFwd (
                    const DgRF<DgGeoCoord, long double>& from,
                    const DgRF<DgProjTriCoord, long double>& to)
         : DgConverter<DgGeoCoord, long double, DgProjTriCoord, long double>(from, to)
{
   pProjTriRF_= dynamic_cast<const DgProjTriRF*>(&toFrame());

   if (!pProjTriRF_)
   {
      report("DgProjIcosaGnomonicFwd::DgProjIcosaGnomonicFwd(): "
        " toFrame not of type DgProjTriRF", DgBase::Fatal);
   }

   faces_ = DgProjIcosaGnomonicFaces(projTriRF().sphIcosa().sphIcosa());

} // DgProjIcosaGnomonicFwd::DgProjIcosaGnomonicFwd

////////////////////////////////////////////////////////////////////////////////
DgProjTriCoord
DgProjIcosaGnomonicFwd::convertTypedAddress (const DgGeoCoord& addIn) const
{
   const DgDVec3D p = DgDVec3D::fromLatLon(addIn.lat(), addIn.lon());

   long double sqd;
   const int face = projTriRF().sphIcosa().whichIcosaTri(p, &sqd);

   DgDVec2D hex;
   _vec3ToHex2d(faces_, p, face, sqd, &hex);

   return DgProjTriCoord(face, DgProjIcosaGnomonic::hex2d0ToProjTri(hex));

} // DgProjTriCoord DgProjIcosaGnomonicFwd::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
