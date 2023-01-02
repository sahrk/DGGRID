/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

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
// table.cpp: creates tables of grid statistics
//
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include <sstream>
#include "dggrid.h"
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgString.h>

////////////////////////////////////////////////////////////////////////////////
void doTable (MainParam& dp, DgGridPList& plist)
{
   orientGrid(dp, plist);

   DgRFNetwork net0;
   const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(net0, dp.datum, dp.earthRadius));
   const DgIDGGSBase *idggs = DgIDGGSBase::makeRF(net0, geoRF, dp.vert0,
          dp.azimuthDegs, dp.aperture, dp.actualRes+1, dp.gridTopo,
          dp.gridMetric, "IDGGS", dp.projType, dp.isMixed43, dp.numAp4,
          dp.isSuperfund, dp.isApSeq, dp.apSeq);

   int numRes = dp.actualRes + 1;

   dgcout << "Earth Radius: "
        << dgg::util::addCommas(geoRF.earthRadiusKM(), dp.precision)
        << "\n" << endl;

   string resS = "Res";
   string nCellsS = "# Cells";
   string areaS = "Area (km^2)";
   string spcS = "Spacing (km)";
   string clsS = "CLS (km)";

   const DgGridStats& gs0 = idggs->idggBase(0).gridStats();
   const DgGridStats& gsR = idggs->idggBase(numRes - 1).gridStats();
   int resWidth =  (int) resS.length();
   int nCellsWidth = max((int) dgg::util::addCommas(gsR.nCells()).length(),
                         (int) nCellsS.length()) + 1;
   int areaWidth = max((int) dgg::util::addCommas(gs0.cellAreaKM(),
                         dp.precision).length(),  (int) areaS.length()) + 1;
//   int spcWidth = max((int) dgg::util::addCommas(gs0.cellDistKM(),
//                         dp.precision).length(), spcS.length()) + 1;
   int clsWidth = max((int) dgg::util::addCommas(gs0.cls(),
                         dp.precision).length(), (int) clsS.length()) + 1;

   dgcout << setw(resWidth) << resS
        << setw(nCellsWidth) << nCellsS
        << setw(areaWidth) << areaS
 //       << setw(spcWidth) << spcS
        << setw(clsWidth) << clsS << endl;

   for (int r = 0; r < numRes; r++)
   {
      if (idggs->idggBase(r).outputRes() >= 0) // in case invalid sf res
      {
         const DgGridStats& gs = idggs->idggBase(r).gridStats();
         dgcout << setw(resWidth)  << idggs->idggBase(r).outputRes()
           << setw(nCellsWidth) << dgg::util::addCommas(gs.nCells())
           << setw(areaWidth) << dgg::util::addCommas(gs.cellAreaKM(),
                                                dp.precision)
//           << setw(spcWidth) << dgg::util::addCommas(gs.cellDistKM(),
//                                                dp.precision)
           << setw(clsWidth) << dgg::util::addCommas(gs.cls(),
                                                dp.precision) << endl;
      }
   }
}

