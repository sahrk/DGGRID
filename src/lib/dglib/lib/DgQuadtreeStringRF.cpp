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

#include <dglib/DgQuadtreeStringRF.h>
#include <dglib/DgQuadtreeSystem.h>

namespace {
// Child = (2I + di, 2J + dj). Rows are up/even J and down/odd J;
// columns are center, apex, then the other corners counter-clockwise.
const int kChildI[2][4] = {{1, 1, 1, 0}, {0, 0, 0, 1}};
const int kChildJ[2][4] = {{1, 0, 2, 0}, {0, 1, -1, 1}};
}

DgHierNdxStringCoord
DgQuadtreeStringRF::quantify (const DgQ2DICoord& point) const
{
   DgQuadtreeSystem::validateResolution(res());
   long long i = point.coord().i();
   long long j = point.coord().j();
   const long long size = 1LL << res();
   if (point.quadNum() < 1 || point.quadNum() > 10 ||
       i < 0 || i >= size || j < 0 || j >= 2 * size)
      report("QUADTREE Q2DI address is outside triangle grid bounds", DgBase::Fatal);

   std::string path(res(), '0');
   for (int r = res(); r > 0; --r) {
      const long long parentI = i / 2;
      const long long parentSize = 1LL << (r - 1);
      long long parentJ = -1;
      int digit = -1;
      // The +/-1 candidates account for the apex/corner row offsets. Plain
      // integer division of both coordinates is not the triangle parent map.
      for (long long candidate = j / 2 - 1; candidate <= j / 2 + 1; ++candidate) {
         if (candidate < 0 || candidate >= 2 * parentSize) continue;
         const int orientation = static_cast<int>(candidate % 2);
         for (int d = 0; d < 4; ++d)
            if (i == 2 * parentI + kChildI[orientation][d] &&
                j == 2 * candidate + kChildJ[orientation][d]) {
               parentJ = candidate;
               digit = d;
            }
      }
      if (digit < 0)
         report("QUADTREE Q2DI address has no triangle parent", DgBase::Fatal);
      path[r - 1] = static_cast<char>('0' + digit);
      i = parentI;
      j = parentJ;
   }
   const int root = 2 * (point.quadNum() - 1) + static_cast<int>(j);
   return DgHierNdxStringCoord(dgg::util::to_string(root, 2) + path);
}

DgQ2DICoord
DgQuadtreeStringRF::invQuantify (const DgHierNdxStringCoord& add) const
{
   const std::string& value = add.value();
   const int root = DgQuadtreeSystem::validateString(value, res());
   long long i = 0;
   long long j = root % 2;
   for (int r = 1; r <= res(); ++r) {
      const int orientation = static_cast<int>(j % 2);
      const int digit = value[r + 1] - '0';
      i = 2 * i + kChildI[orientation][digit];
      j = 2 * j + kChildJ[orientation][digit];
   }
   return DgQ2DICoord(root / 2 + 1, DgIVec2D(i, j));
}

const char*
DgQuadtreeStringRF::str2add (DgHierNdxStringCoord* add, const char* str,
                             char delimiter) const
{
   if (!str)
      report("QUADTREE requires a digit string", DgBase::Fatal);
   const char* end = str;
   while (*end && *end != delimiter) ++end;
   const std::string value(str, end - str);
   DgQuadtreeSystem::validateString(value, res());
   if (add) add->setValue(value);
   return (*end && *(end + 1)) ? end + 1 : nullptr;
}
