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
// DgProjIcosaGnomonic.h: DgProjIcosaGnomonic class definitions
//
// The icosahedral face-centred gnomonic projection used by H3
// (https://h3geo.org): each point is projected from the centre of the sphere
// onto the plane tangent at the centre of its nearest icosahedron face. It is
// neither equal area nor conformal. This implementation works for any DGGRID
// icosahedron orientation; the face centres and in-face axes come from the
// DGGRID icosahedron rather than from H3's tables, and nothing here depends on
// the H3 library.
//
// Portions of the implementation are copied from H3 v4.5.0 (faceijk.c, vec3d.h,
// latLng.c, vec2d.c and constants.h), Copyright 2016-2023, 2026 Uber
// Technologies, Inc., licensed under the Apache License 2.0 (see LICENSE.H3).
// Modified by DGGRID contributors; see the "DGGRID modification" comments in
// DgProjIcosaGnomonic.cpp.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGPROJ_ICOSA_GNOMONIC_H
#define DGPROJ_ICOSA_GNOMONIC_H

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
#include <dglib/DgDVec2D.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgIcosaProj.h>
#include <dglib/DgProjTriRF.h>
#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
/// per-face unit vectors of an icosahedron: the face centre c, the in-plane
/// direction u from the centre towards the face's vertex 0 (H3's "i axis"),
/// and w = c x u, 90 degrees counter-clockwise from u seen from outside
struct DgProjIcosaGnomonicFaces {

   DgProjIcosaGnomonicFaces (void) { }
   explicit DgProjIcosaGnomonicFaces (const SphIcosa& si);

   long double cen[20][3];  // c
   long double uax[20][3];  // u
   long double wax[20][3];  // w
};

////////////////////////////////////////////////////////////////////////////////
/// gnomonic forward: geographic (radians, unit sphere) -> ProjTri (face, x, y)
class DgProjIcosaGnomonicFwd : public DgConverter<DgGeoCoord, long double,
                                        DgProjTriCoord, long double> {

   public:

      DgProjIcosaGnomonicFwd (const DgRF<DgGeoCoord, long double>& geoRF,
                       const DgRF<DgProjTriCoord, long double>& projTriRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgProjTriCoord convertTypedAddress (const DgGeoCoord& addIn)
                                                               const;

   private:

      const DgProjTriRF* pProjTriRF_;
      DgProjIcosaGnomonicFaces faces_;
};

////////////////////////////////////////////////////////////////////////////////
/// gnomonic inverse: ProjTri (face, x, y) -> geographic (radians, unit sphere);
/// defined on the whole tangent plane, so points outside the face are fine
class DgProjIcosaGnomonicInv : public DgConverter<DgProjTriCoord, long double,
                                           DgGeoCoord, long double> {

   public:

      DgProjIcosaGnomonicInv (const DgRF<DgProjTriCoord, long double>& projTriRF,
                          const DgRF<DgGeoCoord, long double>& geoRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgGeoCoord convertTypedAddress (const DgProjTriCoord& addIn)
                                                                      const;

   private:

      const DgProjTriRF* pProjTriRF_;
      DgProjIcosaGnomonicFaces faces_;
};

////////////////////////////////////////////////////////////////////////////////
/// The icosahedral gnomonic projection (forward and inverse converters).
class DgProjIcosaGnomonic : public DgIcosaProj {

   public:

      DgProjIcosaGnomonic (const DgRF<DgGeoCoord, long double>& geoRF,
                    const DgRF<DgProjTriCoord, long double>& projTriRF)
         : DgIcosaProj(*(new DgProjIcosaGnomonicFwd(geoRF, projTriRF)),
                       *(new DgProjIcosaGnomonicInv(projTriRF, geoRF))) {}

      // The DGGRID ProjTri face frame (unit edge triangle with V0 at the apex
      // (0.5, sqrt(3)/2), V1 at (0, 0), V2 at (1, 0)) and the res 0 class II
      // Hex2d frame of the same face (face centre at the origin, V0 on the
      // positive x axis at distance 2, V1 and V2 at 120 and 240 degrees) are
      // related by an exact similarity (rotation by -90 degrees and scale
      // 2 sqrt(3), no reflection):
      //    hex = 2 sqrt(3) (y - sqrt(3)/6, 0.5 - x)
      // These are shared by the gnomonic projection and the H3 index code.
      static DgDVec2D projTriToHex2d0 (const DgDVec2D& p);
      static DgDVec2D hex2d0ToProjTri (const DgDVec2D& h);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
