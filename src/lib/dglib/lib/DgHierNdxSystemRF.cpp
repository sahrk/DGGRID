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
// DgHierNdxSystemRF.cpp: DgHierNdxSystemRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxSystemRF.h>
#include <dglib/DgHierNdxIDGGSBase.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRF<B, DB>::DgHierNdxSystemRF (
         const DgHierNdxRFS<B, DB>& hierNdxRFSIn, int resIn,
         const string& nameIn = "HierNdxSysRF")
   : DgDiscRF<DgHierNdx, B, DB>(hierNdxDggsIn.dggs().network,
              hierNdxDggsIn.dggs()[resIn], nameIn),
     hierNdxDggs_ (hierNdxDggsIn), dggs_ (hierNdxDggsIn.dggs()), res_ (resIn),
     aperture_ (hierNdxDggsIn.dggs().aperture()), pRes_ {nullptr, nullptr},
     curRes_ {nullptr, nullptr}, chRes_ {nullptr, nullptr}
{ 
   // RFS has to call initialize to set up the parent and child systems 
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRF<B, DB>::initialize (void)
{
   setSystemSet(pRes_, res_ - 1);
   setSystemSet(chRes_, res_ + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystemRF<B, DB>::setSystemSet (DgSystemSet& set, int res)
{
   set.intRF_ = nullptr;
   set.strRF_ = nullptr;
   if (res < 0 || res >= rfs().nRes())
      return 1;

   // if we're here res is valid
   set.intRF_ = hierNdxDggs_[res].intRF();
   set.strRF_ = hierNdxDggs_[res].strRF();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
