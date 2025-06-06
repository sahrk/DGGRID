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
// DgAddressType.h: discrete grid address types enum definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGADDRESSTYPE_H
#define DGADDRESSTYPE_H

#include <iostream>

namespace dgg { namespace addtype {

enum DgAddressType { Geo, Plane, ProjTri, Q2DD, Q2DI, SeqNum, Vertex2DD,
    HierNdx,
    // remaining types deprecated
    ZOrder, ZOrderString, Z3, Z3String, Z7, Z7String,
    InvalidAddressType
};

static const std::string addTypeStrings[] = { "GEO", "PLANE", "PROJTRI", "Q2DD",
    "Q2DI", "SEQNUM", "VERTEX2DD", "HIERNDX",
    // remaining types deprecated
    "ZORDER", "ZORDER_STRING", "Z3", "Z3_STRING", "Z7", "Z7_STRING",
    "INVALID"
};

enum class DgHierNdxSysType { ZOrder, Z3, Z7, InvalidHierNdxSysType };

static const std::string hierNdxSysTypeStrings[] = {
    "ZORDER", "Z3", "Z7", "INVALID"
};

enum DgHierNdxFormType { Int64, DigitString, InvalidHierNdxFormType };

static const std::string hierNdxFormTypeStrings[] = {
    "INT64", "DIGIT_STRING", "INVALID"
};

/*
 static const std::string addTypeStrings[] = { "GEO", "PLANE", "PROJTRI", "Q2DD",
     "Q2DI", "SEQNUM", "VERTEX2DD", "ZORDER", "ZORDER_STRING", "Z3", "Z3_STRING",
     "Z7", "Z7_STRING", "INVALID" };
 */

DgAddressType stringToAddressType (const std::string& str);
const std::string& to_string (DgAddressType t);

DgHierNdxSysType stringToHierNdxSysType (const std::string& str);
const std::string& to_string (DgHierNdxSysType t);

DgHierNdxFormType stringToHierNdxFormType (const std::string& str);
const std::string& to_string (DgHierNdxFormType  t);

////////////////////////////////////////////////////////////////////////////////
inline std::ostream&
operator<< (std::ostream& stream, DgAddressType obj)
{
   return stream << to_string(obj);

} // std::ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
inline std::ostream&
operator<< (std::ostream& stream, DgHierNdxSysType obj)
{
   return stream << to_string(obj);

} // std::ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
inline std::ostream&
operator<< (std::ostream& stream, DgHierNdxFormType obj)
{
   return stream << to_string(obj);

} // std::ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

}} // namespace dgg::addtype

#endif
