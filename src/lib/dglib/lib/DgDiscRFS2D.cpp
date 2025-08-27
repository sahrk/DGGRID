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
// DgDiscRFS2D.cpp: DgDiscRFS2D class implementation
//
// Version 7.0 - Kevin Sahr, 12/14/14
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgDiscRFS2D.h>
#include <dglib/DgDmdD4Grid2DS.h>
#include <dglib/DgDmdD8Grid2DS.h>
#include <dglib/DgHexGrid2DS.h>
#include <dglib/DgSeriesConverter.h>
#include <dglib/DgSqrD4Grid2DS.h>
#include <dglib/DgSqrD8Grid2DS.h>
#include <dglib/DgTriGrid2DS.h>

#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const DgDiscRFS2D*
DgDiscRFS2D::makeRF (DgRFNetwork& net, const DgRF<DgDVec2D, long double>& cc0,
   int nRes, unsigned int aperture, dgg::topo::DgGridTopology gridTopo,
   dgg::topo::DgGridMetric gridMetric, bool isCongruent, bool isAligned,
   const std::string& /* name */, bool isMixed43, int numAp4, bool isSuperfund,
   bool isApSeq, const DgApSeq& apSeq)
{
   const DgDiscRFS2D* dg0 = 0;

   using namespace dgg::topo;

   if (gridTopo == Square && gridMetric == D8) {
      dg0 = DgSqrD8Grid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "SqrD82DS");
   } else if (gridTopo == Square && gridMetric == D4) {
      dg0 = DgSqrD4Grid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "SqrD42DS");
   } else if (gridTopo == Diamond && gridMetric == D8) {
      dg0 = DgDmdD8Grid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "DmdD82DS");
   } else if (gridTopo == Diamond && gridMetric == D4) {
      dg0 = DgDmdD4Grid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "DmdD42DS");
   } else if (gridTopo == Hexagon && gridMetric == D6) {
      dg0 = DgHexGrid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                   isAligned, "HexC12DS", isMixed43, numAp4, isSuperfund,
                   isApSeq, apSeq);
   } else if (gridTopo == Triangle && gridMetric == D3) {
      dg0 = DgTriGrid2DS::makeRF(net, cc0, nRes, aperture, isCongruent,
                             isAligned, "Tri2DS");
   } else {
      report("DgDiscRFS2D::makeRF() invalid or unimplemented grid topology/metric: "
        + to_string(gridTopo) + "/" + to_string(gridMetric), DgBase::Fatal);
   }

   dg0->createSubConverters();

   return dg0;

} // const DgDiscRFS2D* DgDiscRFS2D::makeRF

////////////////////////////////////////////////////////////////////////////////
void
DgDiscRFS2D::createSubConverters (void) const {

   std::vector<const DgConverterBase*> sc;
   for (int i = 0; i < nRes(); i++)
   {
      // create converter grids[i] -> backFrame
      sc.push_back(network().getConverter(*grids()[i], grids()[i]->backFrame()));
      sc.push_back(network().getConverter(grids()[i]->backFrame(), backFrame()));
      new DgSeriesConverter(sc, true);
      sc.resize(0);

      // create converter backFrame -> grids[i]
      sc.push_back(network().getConverter(backFrame(), grids()[i]->backFrame()));
      sc.push_back(network().getConverter(grids()[i]->backFrame(), *grids()[i]));
      new DgSeriesConverter(sc, true);
      sc.resize(0);
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
