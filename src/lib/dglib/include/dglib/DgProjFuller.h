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
// DgProjFuller.h: DgProjFuller class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGPROJ_FULLER_H
#define DGPROJ_FULLER_H

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

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgProjFullerFwd : public DgConverter<DgGeoCoord, long double,
                                        DgProjTriCoord, long double> {

   public:

      DgProjFullerFwd (const DgRF<DgGeoCoord, long double>& geoRF,
                       const DgRF<DgProjTriCoord, long double>& projTriRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgProjTriCoord convertTypedAddress (const DgGeoCoord& addIn)
                                                               const;

   private:

      const DgProjTriRF* pProjTriRF_;

};

////////////////////////////////////////////////////////////////////////////////
class DgProjFullerInv : public DgConverter<DgProjTriCoord, long double,
                                           DgGeoCoord, long double> {

   public:

      DgProjFullerInv (const DgRF<DgProjTriCoord, long double>& projTriRF,
                          const DgRF<DgGeoCoord, long double>& geoRF);

      const DgProjTriRF& projTriRF (void) const { return *pProjTriRF_; }

      virtual DgGeoCoord convertTypedAddress (const DgProjTriCoord& addIn)
                                                                      const;

   private:

      const DgProjTriRF* pProjTriRF_;

};

////////////////////////////////////////////////////////////////////////////////
class DgProjFuller : public DgIcosaProj {

   public:

      DgProjFuller (const DgRF<DgGeoCoord, long double>& geoRF,
                    const DgRF<DgProjTriCoord, long double>& projTriRF)
         : DgIcosaProj(*(new DgProjFullerFwd(geoRF, projTriRF)),
                       *(new DgProjFullerInv(projTriRF, geoRF))) {}
};

IcosaGridPt fullerFwd (const GeoCoord& ll, DgSphIcosa& sphicosa);
GeoCoord fullerInv (const IcosaGridPt& pt, SphIcosa& sphicosa);

////////////////////////////////////////////////////////////////////////////////
/*
   The C++ methods above are wrappers for the C functions below
*/
////////////////////////////////////////////////////////////////////////////////

Vec2D fullerFwdOneTri (const GeoCoord geo, long double R, long double * v1,
                       long double * v2, long double * v3);

GeoCoord fullerInvOneTri (const IcosaGridPt pt, long double R, long double* pAzimuth,
                       long double* pTheta);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
