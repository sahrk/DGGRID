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
// SubOpStats.cpp: SubOpStats class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include "OpBasic.h"
#include "SubOpStats.h"

////////////////////////////////////////////////////////////////////////////////
int
SubOpStats::executeOp (void) {

   int numRes = op.dggOp.actualRes + 1;

   dgcout << "Earth Radius: "
        << dgg::util::addCommas(op.dggOp.geoRF().earthRadiusKM(), op.mainOp.precision)
        << "\n" << endl;

   string resS = "Res";
   string nCellsS = "# Cells";
   string areaS = "Area (km^2)";
   string spcS = "Spacing (km)";
   string clsS = "CLS (km)";

   const DgGridStats& gs0 = op.dggOp.dggs().idggBase(0).gridStats();
   const DgGridStats& gsR = op.dggOp.dggs().idggBase(numRes - 1).gridStats();
   int resWidth =  (int) resS.length();
   int nCellsWidth = max((int) dgg::util::addCommas(gsR.nCells()).length(),
                         (int) nCellsS.length()) + 1;
   int areaWidth = max((int) dgg::util::addCommas(gs0.cellAreaKM(),
                         op.mainOp.precision).length(),  (int) areaS.length()) + 1;
//   int spcWidth = max((int) dgg::util::addCommas(gs0.cellDistKM(),
//                         op.mainOp.precision).length(), spcS.length()) + 1;
   int clsWidth = max((int) dgg::util::addCommas(gs0.cls(),
                         op.mainOp.precision).length(), (int) clsS.length()) + 1;

   dgcout << setw(resWidth) << resS
        << setw(nCellsWidth) << nCellsS
        << setw(areaWidth) << areaS
 //       << setw(spcWidth) << spcS
        << setw(clsWidth) << clsS << endl;

   for (int r = 0; r < numRes; r++) {
      if (op.dggOp.dggs().idggBase(r).outputRes() >= 0) { // in case invalid sf res

         const DgGridStats& gs = op.dggOp.dggs().idggBase(r).gridStats();
         dgcout << setw(resWidth)  << op.dggOp.dggs().idggBase(r).outputRes()
           << setw(nCellsWidth) << dgg::util::addCommas(gs.nCells())
           << setw(areaWidth) << dgg::util::addCommas(gs.cellAreaKM(),
                                                op.mainOp.precision)
//           << setw(spcWidth) << dgg::util::addCommas(gs.cellDistKM(),
//                                                op.mainOp.precision)
           << setw(clsWidth) << dgg::util::addCommas(gs.cls(),
                                                op.mainOp.precision) << endl;
      }
   }

   return 0;

} // int SubOpStats::executeOp

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

