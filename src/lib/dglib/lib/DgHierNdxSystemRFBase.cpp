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
// DgHierNdxSystemRFBase.cpp: DgHierNdxSystemRFBase class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxSystemRFBase.h>
#include <dglib/DgHierNdxSystemRFSBase.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRFBase::DgHierNdxSystemRFBase (
         const DgHierNdxSystemRFSBase& hierNdxRFSIn, int resIn,
         const string& nameIn = "HierNdxSysRF")
   : DgDiscRF<DgHierNdx, DgQ2DICoord, long long int>(hierNdxRFSIn.dggs().network,
              hierNdxRFSIn.dggs()[resIn], nameIn),
     hierNdxRFS_ (hierNdxRFSIn), dggs_ (hierNdxRFSIn.dggs()), res_ (resIn),
     aperture_ (hierNdxRFSIn.dggs().aperture()), pRes_ {nullptr, nullptr, nullptr},
     curRes_ {nullptr, nullptr, nullptr}, chRes_ {nullptr, nullptr, nullptr}
{ 
   // RFS has to call initialize to set up the parent and child systems 
   // after the grids_ are all initialized
}

////////////////////////////////////////////////////////////////////////////////
void
DgHierNdxSystemRFBase::initialize (void)
{
   // current res set in the constructor
   //setSystemSet(curRes_, res_);

   setSystemSet(pRes_, res_ - 1);
   setSystemSet(chRes_, res_ + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystemRFBase::setSystemSet (DgSystemSet& set, int res)
{
   set.dgg_ = nullptr;
   set.intRF_ = nullptr;
   set.strRF_ = nullptr;
   if (res < 0 || res >= rfs().nRes())
      return 1;

   // if we're here res is valid
   set.dgg_ = hierNdxRFS_[res].dgg();
   set.intRF_ = hierNdxRFS_[res].intRF();
   set.strRF_ = hierNdxRFS_[res].strRF();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
