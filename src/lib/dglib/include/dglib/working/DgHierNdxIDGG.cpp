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
#include <dglib/DgNdxHierIDGGSBase.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxSystemRF::DgHierNdxSystemRF (const DgNdxHierIDGGSBase& ndxHierDggsIn, int resIn,
               bool outModeIntIn, const string& nameIn)
   : ndxHierDggs_ (ndxHierDggsIn), dggs_ (ndxHierDggsIn.dggs()), res_ (resIn), 
     aperture_ (ndxHierDggsIn.dggs().aperture()), pRes_ {nullptr, nullptr}, 
     curRes_ {nullptr, nullptr}, chRes_ {nullptr, nullptr}, outModeIntIn_ (outModeIntIn)
{
   setSystemSet(pRes_, res_ - 1);
   setSystemSet(curRes_, res_);
   setSystemSet(chRes_, res_ + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystemRF::setSystemSet (DgSystemSet& set, int res)
{
   set.dgg_ = nullptr;
   set.intRF_ = nullptr;
   set.strRF_ = nullptr;
   if (res < 0 || res >= rfs().nRes())
      return 1;

   // res must be valid
   set.dgg_ = dggs_[res];
   set.intRF_ = ndxHierDggs_[i].intRF();
   set.strRF_ = ndxHierDggs_[i].strRF();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
