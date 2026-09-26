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
// IVEA (Icosahedral Vertex-oriented great circle Equal Area, "slice-and-dice")
// projection. SKELETON: currently delegates to the ISEA (Snyder) functions;
// the IVEA kernel replaces it in Phase 6
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
