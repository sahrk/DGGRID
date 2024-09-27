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
// DgHierNdxIDGG.hpp: DgHierNdxIDGG template method definitions
//
////////////////////////////////////////////////////////////////////////////////

//#include <dglib/DgHierNdxIDGG.h>
//#include <dglib/DgHierNdxIDGGSBase.h>

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIDGG::DgHierNdxIDGG (const DgHierNdxIDGGSBase& hierNdxDggsIn, int resIn,
               const string& nameIn)
   : DgHierNdxSystemRF (hierNdxDggsIn, resIn, nameIn),
     hierNdxDggs_ (hierNdxDggsIn), dggs_ (hierNdxDggsIn.dggs()),
     aperture_ (hierNdxDggsIn.dggs().aperture()), curResDgg_ (nullptr),
     pResDgg_ (nullptr), chResDgg_ (nullptr)
{
  curRes_.intRF_ = hierNdxDggs_[res_].intRF();
  curRes_.strRF_ = hierNdxDggs_[res_].strRF();

   // RFS has to call initialize to set up the systems
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
