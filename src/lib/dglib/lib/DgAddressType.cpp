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
// DgAddressType.cpp: DgAddressType enum function implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgAddressType.h>

#include <cmath>

using namespace std;

namespace dgg { namespace addtype {

/*
static const string addTypeStrings[] = { "GEO", "PLANE", "PROJTRI", "Q2DD",
    "Q2DI", "SEQNUM", "VERTEX2DD", "ZORDER", "ZORDER_STRING", "INVALID" };
*/

DgAddressType stringToAddressType (const string& str) {
   for (int i = 0; i < InvalidAddressType; i++)
      if (str == addTypeStrings[i]) return static_cast<DgAddressType>(i);

   return InvalidAddressType;
}

const string& to_string (DgAddressType t) {
   if (t <= InvalidAddressType) return addTypeStrings[t];
   return addTypeStrings[InvalidAddressType];
}

}} // namespace dgg::addtype
