/*******************************************************************************
    Copyright (C) 2026 Kevin Sahr

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

#include <dglib/DgQuadtreeRF.h>
#include <dglib/DgQuadtreeSystem.h>

DgHierNdxIntCoord
DgQuadtreeRF::quantify (const DgQ2DICoord& point) const
{
   return system().toIntCoord(system().strRF()->quantify(point));
}

DgQ2DICoord
DgQuadtreeRF::invQuantify (const DgHierNdxIntCoord& add) const
{
   return system().strRF()->invQuantify(system().toStringCoord(add));
}

const char*
DgQuadtreeRF::str2add (DgHierNdxIntCoord* add, const char* str, char delimiter) const
{
   DgQuadtreeSystem::validateResolution(res());
   if (!str || !*str)
      report("QUADTREE requires a hexadecimal index", DgBase::Fatal);
   const char* end = str;
   while (*end && *end != delimiter) ++end;
   const std::size_t length = end - str;
   if (length == 0 || length > 16)
      report("QUADTREE INT64 requires 1 through 16 hexadecimal characters", DgBase::Fatal);
   uint64_t value = 0;
   for (const char* p = str; p != end; ++p) {
      int digit = -1;
      if (*p >= '0' && *p <= '9') digit = *p - '0';
      else if (*p >= 'a' && *p <= 'f') digit = *p - 'a' + 10;
      else if (*p >= 'A' && *p <= 'F') digit = *p - 'A' + 10;
      if (digit < 0)
         report("QUADTREE INT64 contains an invalid hexadecimal character", DgBase::Fatal);
      value = (value << 4) | static_cast<uint64_t>(digit);
   }
   DgHierNdxIntCoord coord(value);
   system().toStringCoord(coord); // validate root and resolution-dependent padding
   if (add) *add = coord;
   return (*end && *(end + 1)) ? end + 1 : nullptr;
}
