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
// DgNdxHierCPI.cpp: DgNdxHierCPI class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include <dglib/DgNdxHierCPI.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// parent is always center cell at next coarser resolution
void
DgNdxHierCPI::setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const
{
   const DgHexIDGG& dgg = hexIdgg()(add.res());
   const DgHexIDGG& dggp = hexIdgg()(add.res() - 1);
    
   DgLocation* centerLoc = dgg.makeLocation(add.address());
   parent = *centerLoc;
   delete centerLoc;
   centerLoc = null_ptr;
   dggp.convert(parent);
}
    
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// default definition returns all 7 CPI children
void
DgNdxHierCPI::setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                        DgLocVector& children) const
{
   const DgHexIDGG& dgg = hexIdgg()(add.res());
   const DgHexIDGG& dggch = hexIdgg()(add.res() + 1);
    
   // the neighbors of the center hex at next res
   DgLocation* centerLoc = dgg.makeLocation(add.address());
   dggch.convert(centerLoc);
   children.push_back(*centerLoc);

   const DgQ2DICoord& centerAdd = *dggch.getAddress(*centerLoc);
   delete centerLoc;
   centerLoc = null_ptr;

   DgLocVector neighbors;
   dggch.setAddNeighbors(centerAdd, neighbors);
   for (int i = 0; i < neighbors.size(); i++)
      children.push_back(neighbors[i], true);

} // void DgNdxHierCPI::setAddInteriorChildren
