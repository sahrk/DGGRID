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
// DgHierNdxSysType.cpp: DgHierNdxSysType enum function implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgHierNdxSysType.h>

#include <cmath>

using namespace std;

namespace dgg { namespace hiersystype {

DgHierNdxSysType stringToHierNdxSysType (const string& str) {
   for (int i = 0; i < InvalidHierNdxSysType; i++)
      if (str == hierNdxSysTypeStrings[i]) return static_cast<DgHierNdxSysType>(i);

   return InvalidHierNdxSysType;
}

const string& to_string (DgHierNdxSysType t) {
   if (t <= InvalidHierNdxSysType) return hierNdxSysTypeStrings[t];
   return hierNdxSysTypeStrings[InvalidHierNdxSysType];
}

}} // namespace dgg::hiersystype
