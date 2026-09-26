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

#include <dglib/DgIcosaSliceDice.h>
#include <dglib/DgProjIVEA.h>

// the IVEA radial vertex: the icosahedron vertex of each (V, M, C) triangle
static const DgIcosaSliceDice::RadialVertex ivRadial =
                                       DgIcosaSliceDice::RadialVertex::Vertex;

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
   const DgIcosaSliceDice& sd = projTriRF().sphIcosa().sliceDice(ivRadial);

   GeoCoord ll;
   Vec2D xy;
   xy.x = addIn.coord().x();
   xy.y = addIn.coord().y();
   if (sd.inverse(addIn.triNum(), xy, ll) == DgIcosaSliceDice::Status::Fail)
   {
      report("DgProjIVEAInv::convertTypedAddress(): unable to invert point " +
             std::string(addIn), DgBase::Fatal);
   }

   DgGeoCoord geoPt(ll.lon, ll.lat);
   geoPt.normalize();

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
   const DgIcosaSliceDice& sd = projTriRF().sphIcosa().sliceDice(ivRadial);

   GeoCoord ll;
   ll.lon = addIn.lon();
   ll.lat = addIn.lat();

   int face;
   Vec2D xy;
   if (sd.forward(ll, face, xy) == DgIcosaSliceDice::Status::Fail)
   {
      report("DgProjIVEAFwd::convertTypedAddress(): unable to project point " +
             std::string(addIn), DgBase::Fatal);
   }

   return DgProjTriCoord(face, DgDVec2D(xy.x, xy.y));

} // DgProjTriCoord DgProjIVEAFwd::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
