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
// DgHierNdx.cpp: DgHierNdx class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdx.h>
#include <dglib/DgHierNdxIntRF.h>
#include <dglib/DgHierNdxStringRF.h>
#include <dglib/DgHierNdxSystemRFSBase.h>
#include <dglib/DgDiscRFSGrids.h>

////////////////////////////////////////////////////////////////////////////////
// Define static undef coords in dependency order within this translation unit
// to avoid static initialization order fiasco across object files.
const DgHierNdxIntCoord DgHierNdxIntRF::undefCoord(UINT64_MAX);

const DgHierNdxStringCoord DgHierNdxStringRF::undefCoord("99");

const DgHierNdx
DgHierNdx::undefCoord(DgHierNdxIntRF::undefCoord, DgHierNdxStringRF::undefCoord, true);

const DgResAdd<DgHierNdx>
DgHierNdxSystemRFSBase::undefCoord(DgHierNdx::undefCoord, -1);

////////////////////////////////////////////////////////////////////////////////
DgHierNdx::DgHierNdx (bool extModeIntIn)
   : intNdx_ (undefCoord.intNdx()), strNdx_(undefCoord.strNdx()),
                   extModeInt_ (extModeIntIn)
{
}

////////////////////////////////////////////////////////////////////////////////
std::string
DgHierNdx::valString (void) const
{
  if (extModeInt())
     return intNdx_.valString();
  else
     return strNdx_.valString();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
