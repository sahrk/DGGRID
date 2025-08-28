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
// DgCell.cpp: DgCell class implementation
//
////////////////////////////////////////////////////////////////////////////////
//
//  The DgCell class defines an object with a point node, a region, and a
//  label.
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgBase.h>
#include <dglib/DgCell.h>
#include <dglib/DgLocation.h>
#include <dglib/DgPolygon.h>

////////////////////////////////////////////////////////////////////////////////
void
DgCell::convertTo (const DgRFBase& rfIn)
{
   if (!rf_ || rf() != rfIn)
   {
      rf_ = &rfIn;
      rfIn.convert(&node_);
      if (hasRegion()) rfIn.convert(*region_);
   }

} // DgCell::convertTo

////////////////////////////////////////////////////////////////////////////////
std::string
DgCell::asString (void) const
{
   std::string tmp = "[" + node().asString();
   if (hasRegion()) tmp += ":" + region().asString();
   tmp += "]";

   return tmp;

} // DgCell::asString

////////////////////////////////////////////////////////////////////////////////
std::string
DgCell::asString (char delimiter) const
{
   std::string tmp = node().asString(delimiter);
   if (hasRegion()) tmp += delimiter + region().asString(delimiter);

   return tmp;

} // DgCell::asString

////////////////////////////////////////////////////////////////////////////////
std::string
DgCell::asAddressString (void) const
{
   std::string tmp = "[" + node().asAddressString();
   if (hasRegion()) tmp += ":" + region().asAddressString();
   tmp += "]";

   return tmp;

} // DgCell::asAddressString

////////////////////////////////////////////////////////////////////////////////
std::string
DgCell::asAddressString (char delimiter) const
{
   std::string tmp = node().asAddressString(delimiter);
   if (hasRegion()) tmp += delimiter + region().asAddressString(delimiter);

   return tmp;

} // DgCell::asAddressString

////////////////////////////////////////////////////////////////////////////////
const char*
DgCell::fromString (const char* str, char delimiter)
{
   clearAddress();

   DgLocation tloc(rf());
   const char* tmp = tloc.fromString(str, delimiter);
   setNode(tloc);

   if (*tmp == delimiter) tmp++;

   DgPolygon* reg = new DgPolygon(rf());
   tmp = reg->fromString(tmp, delimiter);
   setRegion(reg);

   return tmp;

} // DgCell::fromString

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
