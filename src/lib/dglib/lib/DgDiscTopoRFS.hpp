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
// DgDiscTopoRFS.hpp: DgDiscTopoRFS template class definition.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setAddNeighbors (const DgResAdd<A>& add,
                                            DgLocVector& vec) const
{
   grids()[add.res()]->convert(vec);
   topoRF(add.res())->setAddNeighbors(add.address(), vec);
   this->convert(vec);

} // void DgDiscTopoRFS<A, B, DB>::setAddNeighbors

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setParents (int res, const DgLocation& loc,
                                                DgLocVector& vec) const
{
   vec.clearAddress();
   this->convert(vec);

   if (res > 0 && res < nRes())
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      this->convert(&tmpLoc);
      setAddParents(*(this->getAddress(tmpLoc)), vec);
   }

} // void DgDiscTopoRFS::setParents

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setInteriorChildren (int res, const DgLocation& loc,
                                   DgLocVector& vec) const
{
   vec.clearAddress();
   this->convert(vec);

   if (res >= 0 && res < (nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      this->convert(&tmpLoc);
      setAddInteriorChildren(*(this->getAddress(tmpLoc)), vec);
   }

} // void DgDiscTopoRFS::setInteriorChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setBoundaryChildren (int res, const DgLocation& loc,
                                   DgLocVector& vec) const
{
   vec.clearAddress();
   this->convert(vec);
   if (res >= 0 && res < (nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      this->convert(&tmpLoc);
      setAddBoundaryChildren(*(this->getAddress(tmpLoc)), vec);
   }

} // void DgDiscTopoRFS::setBoundaryChildren

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setBoundary2Children (int res, const DgLocation& loc,
                                   DgLocVector& vec) const
{
   vec.clearAddress();
   this->convert(vec);
   if (res >= 0 && res < (nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      this->convert(&tmpLoc);
      setAddBoundary2Children(*(this->getAddress(tmpLoc)), vec);
   }

} // void DgDiscTopoRFS::setBoundaryChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> void
DgDiscTopoRFS<A, B, DB>::setAllChildren (int res, const DgLocation& loc,
                              DgLocVector& vec) const
{
   vec.clearAddress();
   this->convert(vec);

   if (res >= 0 && res < (nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      grids()[res]->convert(&tmpLoc);
      this->convert(&tmpLoc);
      setAddAllChildren(*(this->getAddress(tmpLoc)), vec);
   }

} // void DgDiscTopoRFS::setAllChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////