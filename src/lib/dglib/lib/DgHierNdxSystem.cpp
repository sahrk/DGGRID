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
// DgHierNdxSystem.hpp: DgHierNdxSystem template class definition.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxSystem.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystem::DgHierNdxSystem (const DgIDGGS& dggsIn, int resIn, 
                     const string& nameIn)
   : dggs_ (dggsIn), res_ (resIn), aperture_ (dggsIn.aperture());
     pRes_ {nullptr, nullptr, nullptr},
     curRes_ {nullptr, nullptr, nullptr},
     chRes_ {nullptr, nullptr, nullptr}
{
   setSystemSet(curRes_, res());
   setSystemSet(pRes_, res() - 1);
   setSystemSet(chRes_, res() + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystem::setSystemSet (DgSystemSet& set, int resIn)
{
   if (resIn >= 0 && resIn < dggs().nRes()) {
      set.dgg_ = dggsIn.idgg(resIn));
      //set.intRF_ =
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystem::setNdxParent (int res, const DgLocation& loc,
                    DgLocation& parent) const
{
   parent.clearAddress();
   dggs().convert(&parent);

   //if (res > 0 && res < dggs().nRes()) {
   if (pDgg() && dgg()) {
      DgLocation tmpLoc(loc);
      dggs().grids()[res]->convert(&tmpLoc);
      dggs().convert(&tmpLoc);
      intRF().setAddNdxParent(*(dggs().getAddress(tmpLoc)), parent);
   }

} // void DgHierNdxSystem::setNdxParent

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystem::setNdxChildren (int res, const DgLocation& loc,
                                   DgLocVector& children) const
{
   children.clearAddress();
   dggs().convert(children);

   if (res >= 0 && res < (dggs().nRes() - 1))
   {
      DgLocation tmpLoc(loc);
      dggs().grids()[res]->convert(&tmpLoc);
      dggs().convert(&tmpLoc);
      intRF().setAddNdxChildren(*(dggs().getAddress(tmpLoc)), children);
   }

} // void DgHierNdxSystem::setNdxChildren

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
