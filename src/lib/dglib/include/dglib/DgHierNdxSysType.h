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
// DgHierNdxSysType.h: discrete grid address types enum definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTYPE_H
#define DGHIERNDXSYSTYPE_H

#include <iostream>

namespace dgg { namespace hiersystype {

enum DgHierNdxSysType { ZXSystem, NoHierNdxSysType, InvalidHierNdxSysType };

static const std::string hierNdxSysTypeStrings[] = { "ZX_SYSTEM", "NONE", "INVALID" };

DgHierNdxSysType stringToHierNdxSysType (const std::string& str);
const std::string& to_string (DgHierNdxSysType t);

////////////////////////////////////////////////////////////////////////////////
inline std::ostream&
operator<< (std::ostream& stream, DgHierNdxSysType obj)
{
   return stream << to_string(obj);

} // std::ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

}} // namespace dgg::hiersystype

#endif
