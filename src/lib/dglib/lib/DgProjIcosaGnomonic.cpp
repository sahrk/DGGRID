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

/** @brief 3D floating point vector */
struct Vec3d {
    long double x;  /// towards 0deg lat, 0deg lon
    long double y;  /// towards 0deg lat, 90deg lon
    long double z;  /// towards north pole
};

const long double EPSILON = 0.0000000000000001L;
const long double RES0_U_GNOMONIC = 0.38196601125010500003L;
const long double INV_RES0_U_GNOMONIC = 2.61803398874989588842L;

/** Convert latitude and longitude to a unit Vec3d on the sphere. */
inline Vec3d latLngToVec3(long double lat, long double lng) {
    long double r = cosl(lat);
    Vec3d out = {
        cosl(lng) * r,
        sinl(lng) * r,
        sinl(lat),
    };
    return out;
}

inline Vec3d vec3LinComb(long double a, Vec3d v1, long double b, Vec3d v2) {
    Vec3d out = {
        a * v1.x + b * v2.x,
        a * v1.y + b * v2.y,
        a * v1.z + b * v2.z,
    };
    return out;
}

inline Vec3d vec3Cross(Vec3d v1, Vec3d v2) {
    Vec3d out = {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
    return out;
}

inline long double vec3Dot(Vec3d v1, Vec3d v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline long double vec3NormSq(Vec3d v) { return vec3Dot(v, v); }

inline long double vec3Norm(Vec3d v) { return sqrtl(vec3NormSq(v)); }

inline void vec3Normalize(Vec3d *v) {
    long double norm = vec3Norm(*v);

    // Norm can be zero either from true zero vector, or from squaring
    // underflowing to zero.
    // If the norm is nonzero, we normalize v using it.
    // If the norm is zero, we set the vector to be exactly zero.
    long double s = 0.0L;
    if (norm > 0.0L) {
        s = 1.0L / norm;
    }

    v->x *= s;
    v->y *= s;
    v->z *= s;
}

inline long double vec3DistSq(Vec3d v1, Vec3d v2) {
    Vec3d d = vec3LinComb(1.0L, v1, -1.0L, v2);
    return vec3NormSq(d);
}

inline long double _v2dMag(const DgDVec2D& v) {
    return sqrtl(v.x() * v.x() + v.y() * v.y());
}

inline Vec3d toVec3 (const long double a[3]) {
    Vec3d v = { a[0], a[1], a[2] };
    return v;
}

/**
 * Encodes a coordinate on the sphere to the corresponding icosahedral face and
 * containing the squared euclidean distance to that face center.
 *
 * Vec3d v is expected to be on the unit sphere.
 *
 * @param v The Vec3d coordinates to encode.
 * @param face Output: The icosahedral face containing the coordinates.
 * @param sqd Output: The squared euclidean distance to its face center.
 */
void _vec3ToClosestFace(const DgProjIcosaGnomonicFaces& faces, const Vec3d *v,
                        int *face, long double *sqd) {
    *face = 0;
    // The distance between two farthest points is 2.0, therefore the square of
    // the distance between two points should always be less or equal than 4.0 .
    *sqd = 5.0L;
    for (int f = 0; f < 20; ++f) {
        long double sqdT = vec3DistSq(toVec3(faces.cen[f]), *v);
        if (sqdT < *sqd) {
            *face = f;
            *sqd = sqdT;
        }
    }
}

/**
 * Encodes a coordinate on the sphere to the corresponding icosahedral face and
 * containing 2D hex coordinates relative to that face center.
 *
 * Vec3d p is expected to be on the unit sphere.
 *
 * @param p The Vec3d coordinates to encode.
 * @param face Output: The icosahedral face containing the coordinates.
 * @param v Output: The 2D hex coordinates (resolution 0, class II) of the
 *          point.
 */
// Resolution 0 only (DGGRID's own lattices handle resolution). theta is
// measured counter-clockwise from the face's u axis using the face basis; a
// north/east tangent basis fails when a face centre is at a pole (DGGRID
// allows any icosahedron orientation).
void _vec3ToHex2d(const DgProjIcosaGnomonicFaces& faces, const Vec3d *p,
                  int *face, DgDVec2D *v) {
    // determine the icosahedron face
    long double sqd;
    _vec3ToClosestFace(faces, p, face, &sqd);

    // cos(r) = 1 - 2 * sin^2(r/2) = 1 - 2 * (sqd / 4) = 1 - sqd/2
    // r = 2 asin(sqrt(sqd) / 2) is identical but numerically stable;
    // acos(1 - sqd/2) loses precision near the face centre.
    long double r = 2.0L * asinl(sqrtl(sqd) / 2.0L);

    if (r < EPSILON) {
        *v = DgDVec2D(0.0L, 0.0L);
        return;
    }

    // now have face and r, now find CCW theta from the face's i axis
    const Vec3d c = toVec3(faces.cen[*face]);
    const Vec3d u = toVec3(faces.uax[*face]);
    const Vec3d w = toVec3(faces.wax[*face]);
    Vec3d q = vec3LinComb(1.0L, *p, -vec3Dot(*p, c), c);
    long double theta = atan2l(vec3Dot(q, w), vec3Dot(q, u));

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
 * @param v3 Output: the 3D coordinates of the point
 */
// Resolution 0 only, using the face basis (see _vec3ToHex2d)
void _hex2dToVec3(const DgProjIcosaGnomonicFaces& faces, const DgDVec2D& v,
                  int face, Vec3d *v3) {
    // calculate (r, theta) in hex2d
    long double r = _v2dMag(v);

    const Vec3d c = toVec3(faces.cen[face]);
    if (r < EPSILON) {
        *v3 = c;
        return;
    }

    long double theta = atan2l(v.y(), v.x());

    r *= RES0_U_GNOMONIC;

    // perform inverse gnomonic scaling of r
    r = atanl(r);

    // now find the point at (r,theta) from the face center
    const Vec3d u = toVec3(faces.uax[face]);
    const Vec3d w = toVec3(faces.wax[face]);
    Vec3d dir = vec3LinComb(cosl(theta), u, sinl(theta), w);

    *v3 = vec3LinComb(cosl(r), c, sinl(r), dir);
    vec3Normalize(v3);
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
DgProjIcosaGnomonicFaces::DgProjIcosaGnomonicFaces (const SphIcosa& si)
{
   for (int t = 0; t < 20; t++) {
      const Vec3d ct = latLngToVec3(si.triCen[t].pt.lat, si.triCen[t].pt.lon);
      const Vec3d v0 = latLngToVec3(si.icotri[t][0].lat, si.icotri[t][0].lon);

      // in-plane direction from the face centre to V0
      Vec3d u = vec3LinComb(1.0L, v0, -vec3Dot(v0, ct), ct);
      vec3Normalize(&u);
      const Vec3d w = vec3Cross(ct, u);

      cen[t][0] = ct.x; cen[t][1] = ct.y; cen[t][2] = ct.z;
      uax[t][0] = u.x; uax[t][1] = u.y; uax[t][2] = u.z;
      wax[t][0] = w.x; wax[t][1] = w.y; wax[t][2] = w.z;
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
   Vec3d p;
   _hex2dToVec3(faces_, DgProjIcosaGnomonic::projTriToHex2d0(addIn.coord()),
                addIn.triNum(), &p);

   // atan2l keeps full precision for latitudes near the poles
   DgGeoCoord geoPt(atan2l(p.y, p.x), atan2l(p.z, hypotl(p.x, p.y)));
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
   const Vec3d p = latLngToVec3(addIn.lat(), addIn.lon());

   int face;
   DgDVec2D hex;
   _vec3ToHex2d(faces_, &p, &face, &hex);

   return DgProjTriCoord(face, DgProjIcosaGnomonic::hex2d0ToProjTri(hex));

} // DgProjTriCoord DgProjIcosaGnomonicFwd::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
