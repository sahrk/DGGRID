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
// DgZXRF.cpp: DgZXRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZXRF.h>

////////////////////////////////////////////////////////////////////////////////
INT_NDX_TYPE 
DgZXRF::quantify (const DgQ2DICoord& point) const
{
   return undefAddress();
}

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord 
DgZXRF::invQuantify (const INT_NDX_TYPE& add) const
{
   DgQ2DICoord c;
   return c;
}

////////////////////////////////////////////////////////////////////////////////
void
DgZXRF::setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const
{
   const DgHexIDGG& dgg = hexDggs().hexIdgg(add.res());
   const DgHexIDGG& dggp = hexDggs().hexIdgg(add.res() - 1);
    
   DgLocation* centerLoc = dgg.makeLocation(add.address());
   parent = *centerLoc;
   delete centerLoc;
   centerLoc = nullptr;
   dggp.convert(&parent);
}
    
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// default definition returns all 7 CPI children
void
DgZXRF::setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                        DgLocVector& children) const
{
   const DgHexIDGG& dgg = hexDggs().hexIdgg(add.res());
   const DgHexIDGG& dggch = hexDggs().hexIdgg(add.res() + 1);
    
   // the neighbors of the center hex at next res
   DgLocation* centerLoc = dgg.makeLocation(add.address());
   dggch.convert(centerLoc);
   children.push_back(*centerLoc);

   const DgQ2DICoord& centerAdd = *dggch.getAddress(*centerLoc);
   delete centerLoc;
   centerLoc = nullptr;

   DgLocVector neighbors;
   dggch.setAddNeighbors(centerAdd, neighbors);
   for (int i = 0; i < neighbors.size(); i++)
      children.push_back(neighbors[i], true);

} // void DgZXRF::setAddInteriorChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
