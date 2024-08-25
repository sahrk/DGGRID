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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZXRF.h>

////////////////////////////////////////////////////////////////////////////////
//const DgZXCoord DgZXCoord::undefDgZXCoord(0xffffffffffffffff);

////////////////////////////////////////////////////////////////////////////////
const char*
DgZXRF::str2addTyped (DgHierNdxCoord<uint64_t>* add, const char* str, char delimiter) const
{
   return str;

} // const char* DgZXRF::str2addTyped

////////////////////////////////////////////////////////////////////////////////
string
DgZXCoord::valString (void) const
{
   const int maxStrSize = 17; // max 16 digits plus 1 for the null terminator
   char str[maxStrSize];
   snprintf(str, maxStrSize, "%016" PRIx64, value_);

   return string(str);

} // const string& DgZXRF::valString

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
