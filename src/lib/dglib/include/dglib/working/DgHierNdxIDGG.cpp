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
// DgHierNdxIDGG.cpp: DgHierNdxIDGG class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxIDGG.h>
#include <dglib/DgNdxHierIDGGSBase.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIDGG::DgHierNdxIDGG (const DgNdxHierIDGGSBase& ndxHierDggsIn, int resIn,
               bool outModeIntIn, const string& nameIn)
   : DgHierNdxSystemRF (ndxHierDggsIn, resIn, outModeIntIn, nameIn),
     ndxHierDggs_ (ndxHierDggsIn), dggs_ (ndxHierDggsIn.dggs()), 
     aperture_ (ndxHierDggsIn.dggs().aperture()), curResDgg_ (nullptr),
     pResDgg_ (nullptr), chResDgg_ (nullptr)
{
   setDgg(&pResDgg_, res() - 1);
   setDgg(&curResDgg_, res());
   setDgg(&chResDgg_, res() + 1);
}

////////////////////////////////////////////////////////////////////////////////
int 
DgHierNdxIDGG::setDgg (const DgIDGG** dgg, int res)
{
   *dgg = nullptr;
   if (res < 0 || res >= rfs().nRes())
      return 1;

   // if we're here res is valid
   *dgg = dggs()[res];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
