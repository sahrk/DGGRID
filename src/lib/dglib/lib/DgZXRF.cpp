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
// DgZXRF.cpp: DgZXRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

//#define __STDC_FORMAT_MACROS
//#include <inttypes.h>
//#include <cmath>
//#include <climits>
//#include <cfloat>
//#include <string.h>

#include <dglib/DgZXRF.h>
//#include <dglib/DgZXSystem.h>

////////////////////////////////////////////////////////////////////////////////
DgZXRF::DgZXRF (const DgHierNdxSystemRFBase& sysIn, int resIn, const std::string& nameIn)
    : DgHierNdxIntRF(sysIn, resIn, nameIn) 
{ 
}

////////////////////////////////////////////////////////////////////////////////
const char*
DgZXRF::str2add (DgHierNdxIntCoord* add, const char* str, char delimiter) const
{
   return str;

} // const char* DgZXRF::str2add

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord 
DgZXRF::quantify (const DgQ2DICoord& point) const 
{
   DgHierNdxIntCoord c;
   return c;
}

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord 
DgZXRF::invQuantify (const DgHierNdxIntCoord& add) const
{
   DgQ2DICoord c;
   return c;
}

////////////////////////////////////////////////////////////////////////////////
