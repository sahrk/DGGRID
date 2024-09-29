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
DgHierNdxIDGGBase::DgHierNdxIDGGBase (const DgHierNdxIDGGSBase& hierNdxDggsIn, int resIn,
               const string& nameIn)
   : DgHierNdxSystemRF (hierNdxDggsIn, resIn, nameIn),
     hierNdxDggs_ (hierNdxDggsIn), dggs_ (hierNdxDggsIn.dggs()),
     aperture_ (hierNdxDggsIn.dggs().aperture()), curResDgg_ (nullptr),
     pResDgg_ (nullptr), chResDgg_ (nullptr)
{
   // RFS has to call initialize to set up the systems
}

DgHierNdxSystemRFBase::DgHierNdxSystemRFBase (
         const DgHierNdxSystemRFSBase& hierNdxRFSIn, int resIn,
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
DgHierNdxSystemRFBase::initialize (void)
{
   setSystemSet(pRes_, res_ - 1);
   setSystemSet(chRes_, res_ + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxSystemRFBase::setSystemSet (DgSystemSet& set, int res)
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
void
DgHierNdxIDGGBase::initialize (void)
{
   DgHierNdxSystemRF::initialize();
   setDgg(&pResDgg_, res() - 1);
   setDgg(&curResDgg_, res());
   setDgg(&chResDgg_, res() + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxIDGGBase::setDgg (const DgIDGG** dgg, int res)
{
   *dgg = nullptr;
   if (res < 0 || res >= rfs().nRes())
      return 1;

   // if we're here res is valid
   *dgg = dggs()[res];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
