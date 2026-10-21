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
// DgBoundedHexC3C2RF2D.cpp: DgBoundedHexC3C2RF2D class implementation
//
// Version 7.0 - Kevin Sahr, 12/15/14
//
////////////////////////////////////////////////////////////////////////////////

#include <climits>

#include <dglib/DgBoundedHexC3C2RF2D.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned long long int
DgBoundedHexC3C2RF2D::seqNumAddress (const DgIVec2D& add) const
{
   DgIVec2D tVec = add - lowerLeft();
   unsigned long long int sNum = tVec.i() * numI() / 7;

   switch (tVec.i() % 7)
   {
      case 0: sNum += tVec.j() / 7;
              break;

      case 1: sNum += (tVec.j() - 5) / 7;
              break;

      case 2: sNum += (tVec.j() - 3) / 7;
              break;

      case 3: sNum += (tVec.j() - 1) / 7;
              break;

      case 4: sNum += (tVec.j() - 6) / 7;
              break;

      case 5: sNum += (tVec.j() - 4) / 7;
              break;

      case 6: sNum += (tVec.j() - 2) / 7;
              break;

   }

   if (!zeroBased()) sNum++;

   sNum = sNum / 3;

   return sNum;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// The Class II/III cells are every third Class III cell, and seqNumAddress()
// is the Class III sequence number divided by 3. Invert it by finding which of
// the (at most) three Class III cells that map to sNum has the Class II/III
// address pattern.
DgIVec2D
DgBoundedHexC3C2RF2D::addFromSeqNum (unsigned long long int sNum) const
{
   // seqNumAddress() divides the (possibly one-based) Class III sequence
   // number by 3, so try the three Class III sequence numbers that divide
   // down to sNum
   for (unsigned long long int i = 0; i < 3; i++)
   {
      unsigned long long int c3Num = 3 * sNum + i;
      if (!zeroBased() && !c3Num) continue;

      DgIVec2D add = DgBoundedHexC3RF2D::addFromSeqNum(c3Num);
      if (validAddress(add) && seqNumAddress(add) == sNum) return add;
   }

   return invalidAdd();

} // DgIVec2D DgBoundedHexC3C2RF2D::addFromSeqNum

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
