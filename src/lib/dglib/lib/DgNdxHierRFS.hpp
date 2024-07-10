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
// DgNdxHierRFS.hpp: DgNdxHierRFS template class definition.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgNdxHierRFS<A, B, DB>::setNdxParent (int res, const DgLocation& loc,
                    DgLocation& parent) const
{
   parent.clearAddress();
   rfs().convert(parent);

   if (res > 0 && res < rfs().nRes()) {
      DgLocation tmpLoc(loc);
      rfs().grids()[res]->convert(&tmpLoc);
      rfs().convert(&tmpLoc);
      setAddParent(*(rfs().getAddress(tmpLoc)), parent);
   }

} // void DgNdxHierRFS::setNdxParent

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgNdxHierRFS<A, B, DB>::setNdxChildren (int res, const DgLocation& loc,
                                   DgLocVector& children) const
{
   children.clearAddress();
   rfs().convert(children);

   if (res >= 0 && res < (rfs().nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      rfs().grids()[res]->convert(&tmpLoc);
      rfs().convert(&tmpLoc);
      setAddNdxChildren(*(rfs().getAddress(tmpLoc)), children);
   }

} // void DgNdxHierRFS::setNdxChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
