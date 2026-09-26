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
// DgProjIVEA.cpp: DgProjIVEA class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>

#include <dglib/DgProjISEA.h> // SKELETON: for snyderFwd/snyderInv
#include <dglib/DgProjIVEA.h>

////////////////////////////////////////////////////////////////////////////////
DgProjIVEAInv::DgProjIVEAInv (const DgRF<DgProjTriCoord, long double>& from,
                       const DgRF<DgGeoCoord, long double>& to)
         : DgConverter<DgProjTriCoord, long double, DgGeoCoord, long double>(from, to),
           pProjTriRF_ (0)
{
   pProjTriRF_ = dynamic_cast<const DgProjTriRF*>(&fromFrame());

   if (!pProjTriRF_)
   {
      report("DgProjIVEAInv::DgProjIVEAInv(): "
        " fromFrame not of type DgProjTriRF", DgBase::Fatal);
   }

} // DgProjIVEAInv::DgProjIVEAInv

////////////////////////////////////////////////////////////////////////////////
DgGeoCoord
DgProjIVEAInv::convertTypedAddress (const DgProjTriCoord& addIn) const
{
//cout << "***DgProjIVEAInv: DgProjTriCoord: " << addIn << std::endl;
   IcosaGridPt gridpt;
   gridpt.pt.x = addIn.coord().x();
   gridpt.pt.y = addIn.coord().y();
   gridpt.triangle = addIn.triNum();

//cout << "    gridpt.triangle .x .y: " << gridpt.triangle << ", " <<
//      gridpt.pt.x << ", " << gridpt.pt.y << std::endl;

   // SKELETON: delegates to ISEA; replaced in Phase 6
   GeoCoord ll = snyderInv(gridpt, projTriRF().sphIcosa().sphIcosa());

//cout << " ll.lon, ll.lat: " << ll.lon << ", " <<
//ll.lat << std::endl;
   DgGeoCoord geoPt(ll.lon, ll.lat);
   geoPt.normalize();

//cout << "    geoPt: " << geoPt << std::endl;
   return geoPt;

} // DgGeoCoord DgProjIVEAInv::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgProjIVEAFwd::DgProjIVEAFwd (const DgRF<DgGeoCoord, long double>& from,
                    const DgRF<DgProjTriCoord, long double>& to)
         : DgConverter<DgGeoCoord, long double, DgProjTriCoord, long double>(from, to)
{
   pProjTriRF_= dynamic_cast<const DgProjTriRF*>(&toFrame());

   if (!pProjTriRF_)
   {
      report("DgProjIVEAFwd::DgProjIVEAFwd(): "
        " toFrame not of type DgProjTriRF", DgBase::Fatal);
   }

} // DgProjIVEAFwd::DgProjIVEAFwd

////////////////////////////////////////////////////////////////////////////////
DgProjTriCoord
DgProjIVEAFwd::convertTypedAddress (const DgGeoCoord& addIn) const
{

//cout << "***DgProjIVEAFwd: geoPt: " << addIn << std::endl;
   GeoCoord ll;

   ll.lon = addIn.lon();
   ll.lat = addIn.lat();

//cout << "   ll.lon, ll.lat: " << ll.lon << ", " << ll.lat << std::endl;

   // SKELETON: delegates to ISEA; replaced in Phase 6
   IcosaGridPt gridpt = snyderFwd(ll, projTriRF().sphIcosa());
//cout << "    gridpt.triangle .x .y: " << gridpt.triangle << ", " <<
//gridpt.pt.x << ", " << gridpt.pt.y << std::endl;

//cout << "DgProjTriCoord: " << DgProjTriCoord(gridpt.triangle,
//                               DgDVec2D(gridpt.pt.x, gridpt.pt.y)) << std::endl;

   return DgProjTriCoord(gridpt.triangle, DgDVec2D(gridpt.pt.x, gridpt.pt.y));

} // DgProjTriCoord DgProjIVEAFwd::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
