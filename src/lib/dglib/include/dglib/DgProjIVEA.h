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
// DgProjIVEA.h: DgProjIVEA class definitions
//
// IVEA (Icosahedral Vertex-oriented great-circle Equal Area) projection: the
// slice-and-dice construction of van Leeuwen & Strebe (2006,
// doi:10.1559/152304006779500687) applied to the 120 fundamental (V, M, C)
// triangles of the icosahedron, with the icosahedron vertex V as the radial
// vertex. It is equal area, and great circles through the icosahedron
// vertices map to straight lines. The kernel is DgIcosaSliceDice; ISEA
// (DgProjISEA) is the same construction with the face centre radial.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGPROJ_IVEA_H
#define DGPROJ_IVEA_H

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgContCartRF.h>
#include <dglib/DgConverter.h>
#include <dglib/DgDVec2D.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgIcosaProj.h>
#include <dglib/DgIVec2D.h>
#include <dglib/DgLocation.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgProjTriRF.h>
#include <dglib/DgRF.h>
#include <dglib/DgUtil.h>

#include <climits>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// IVEA forward: geographic (radians, unit sphere) -> ProjTri (face, x, y).
class DgProjIVEAFwd : public DgConverter<DgGeoCoord, long double,
                                        DgProjTriCoord, long double> {

   public:

      DgProjIVEAFwd (const DgRF<DgGeoCoord, long double>& geoRF,
                       const DgRF<DgProjTriCoord, long double>& projTriRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgProjTriCoord convertTypedAddress (const DgGeoCoord& addIn)
                                                               const;

   private:

      const DgProjTriRF* pProjTriRF_;

};

////////////////////////////////////////////////////////////////////////////////
/// IVEA inverse: ProjTri (face, x, y) -> geographic (radians, unit sphere).
class DgProjIVEAInv : public DgConverter<DgProjTriCoord, long double,
                                           DgGeoCoord, long double> {

   public:

      DgProjIVEAInv (const DgRF<DgProjTriCoord, long double>& projTriRF,
                          const DgRF<DgGeoCoord, long double>& geoRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgGeoCoord convertTypedAddress (const DgProjTriCoord& addIn)
                                                                      const;

   private:

      const DgProjTriRF* pProjTriRF_;

};

////////////////////////////////////////////////////////////////////////////////
/// The IVEA icosahedral projection (forward and inverse converters).
class DgProjIVEA : public DgIcosaProj {

   public:

      DgProjIVEA (const DgRF<DgGeoCoord, long double>& geoRF,
                    const DgRF<DgProjTriCoord, long double>& projTriRF)
         : DgIcosaProj(*(new DgProjIVEAFwd(geoRF, projTriRF)),
                       *(new DgProjIVEAInv(projTriRF, geoRF))) {}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
