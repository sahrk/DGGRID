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
// DgIDGGBase.cpp: DgIDGGBase class implementation
//
// Version 7.0 - Kevin Sahr, 11/15/14
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgIDGGSBase.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgHexIDGGS.h>
#include <dglib/DgApSeq.h>
#include <dglib/DgPolygon.h>

////////////////////////////////////////////////////////////////////////////////
const DgIDGGSBase*
DgIDGGSBase::makeRF (DgRFNetwork& network, const DgGeoSphRF& backFrame,
          const DgGeoCoord& vert0, long double azDegs, unsigned int apertureIn,
          int nResIn, DgGridTopology gridTopo, DgGridMetric gridMetric,
          const std::string& name, const std::string& projTypeIn, bool isApSeqIn, const DgApSeq& apSeqIn,
          bool isMixed43In, int numAp4In, bool isSuperfundIn, DgHierNdxSysType hierNdxSysType)
{
   const DgIDGGSBase* dg0 = NULL;

   if (gridTopo == Hexagon)
      dg0 = DgHexIDGGS::makeRF(network, backFrame, vert0, azDegs, apertureIn, nResIn,
            name, projTypeIn, isApSeqIn, apSeqIn, isMixed43In, numAp4In,
            isSuperfundIn, hierNdxSysType);
   else
      dg0 = DgIDGGS::makeRF(network, backFrame, vert0, azDegs,
            apertureIn, nResIn, gridTopo, gridMetric, name, projTypeIn,
            isApSeqIn, apSeqIn, isMixed43In, numAp4In,
            isSuperfundIn, hierNdxSysType);

   return dg0;

} // DgIDGGSBase::makeRF

////////////////////////////////////////////////////////////////////////////////
long double
DgIDGGSBase::cellAreaKM (const DgResAdd<DgQ2DICoord>& add, int densify,
                          const DgEllipsoidRF& datum) const
{
   const DgIDGGBase& grid = idggBase(add.res());

   // The cell center, by converting the cell itself to the grid sphere.
   // grid.setPoint() cannot be used: it relies on invQuantify(), which
   // DgIDGGBase stubs out because its conversions go through the converter
   // network, so it leaves the point at (0, 0). A fan drawn from there still
   // sums to the right area on a sphere, but on WGS 84 its triangles are
   // thousands of km long, which is slow and loses accuracy.
   DgLocation* cell = grid.makeLocation(add.address());
   grid.geoRF().convert(cell);
   const DgLocation center(*cell);
   delete cell;

   DgPolygon boundary(grid.geoRF());
   grid.setAddVertices(add.address(), boundary, densify, true);

   return datum.cellAreaKM(center, boundary);
}

////////////////////////////////////////////////////////////////////////////////
long double
DgIDGGSBase::equalAreaCellAreaKM (unsigned long long int nCells) const
{
   const long double total = geoRF().totalAreaKM();

   // total = (nCells - 12) * hex + 12 * (5/6) * hex = (nCells - 2) * hex
   if (gridTopo() == Hexagon)
      return total / (nCells - 2);

   return total / nCells;
}

////////////////////////////////////////////////////////////////////////////////
