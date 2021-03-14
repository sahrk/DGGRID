/*******************************************************************************
    Copyright (C) 2018 Kevin Sahr

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
// DgDmdIDGG.cpp: DgDmdIDGG class implementation
//
// Kevin Sahr, 8/12/20
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>
#include <cfloat>

#include "DgDmdIDGG.h"
#include "DgIDGGS4D.h"
#include "DgDmdD4Grid2D.h"
#include "DgDmdD8Grid2D.h"
#include "DgDmdD4Grid2DS.h"
#include "DgDmdD8Grid2DS.h"
#include "DgSeriesConverter.h"
#include "DgRadixString.h"
#include "DgBoundedIDGG.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgDmdIDGG::DgDmdIDGG (const DgIDGGS4D& dggs, unsigned int aperture,
              int res, const string& name, DgGridMetric gridMetric, 
              unsigned int precision)
   : DgIDGGBase (&dggs, dggs.geoRF(), aperture, res, name, Diamond, gridMetric, 
                 precision),
	   scaleFac_ (1.0L)
{ 
   initialize();

} // DgDmdIDGG::DgDmdIDGG

////////////////////////////////////////////////////////////////////////////////
DgDmdIDGG::DgDmdIDGG (const DgDmdIDGG& rfIn)
   : DgIDGGBase (rfIn.dggs(), rfIn.geoRF(), rfIn.aperture(), rfIn.res(), 
                 rfIn.name(), Diamond, rfIn.gridMetric(), rfIn.precision()),
	scaleFac_ (rfIn.scaleFac())
{ 
   initialize();

} // DgDmdIDGG::DgDmdIDGG

////////////////////////////////////////////////////////////////////////////////
DgDmdIDGG::~DgDmdIDGG (void) { }

////////////////////////////////////////////////////////////////////////////////
const DgIDGGS4D& 
DgDmdIDGG::dmdDggs (void) const 
{ return *(static_cast<const DgIDGGS4D*>(dggs())); }

////////////////////////////////////////////////////////////////////////////////
void
DgDmdIDGG::initialize (void)
{
   // verify parameter validity

   string apErrStr = string("DgDmdIDGG::initialize(): invalid aperture " + 
             dgg::util::to_string(aperture()) + string(" for grid topo ") + 
             to_string(gridTopo()));

   if (gridTopo() != Diamond)
      report("DgDmdIDGG::initialize(): invalid grid topo " + 
             to_string(gridTopo()), DgBase::Fatal);

   if (aperture() != 4) report(apErrStr, DgBase::Fatal);
 
   // create some internal data structures
   undefLoc_ = makeLocation(undefAddress());
   sphIcosa_ = new DgSphIcosa(vert0(), azDegs());

   isAligned_ = false;
   isCongruent_ = false;

   // initialize parent values as if this is grid res 0
   long double parentScaleFac = 1.0;
   unsigned long long int parentNCells = 1;

   // get actual parent values if there is a parent grid
   if (res() > 0) {
      const DgDmdIDGG& parentIDGG = dmdDggs().dmdIdgg(res() - 1);

      parentScaleFac = parentIDGG.scaleFac();
      parentNCells = parentIDGG.gridStats().nCells();
   }

   // set-up local network to scale so that quad (and consequently dmd) edge 
   // length is 1.0
   ccFrame_ = new DgContCartRF(locNet_, name() + "CC1");
   if (gridMetric() == D4)
      grid2DS_ = new DgDmdD4Grid2DS(locNet_, ccFrame(), res() + 1, 4, true, false, name() + string("D4H2DS"));
   else // must be D8
      grid2DS_ = new DgDmdD8Grid2DS(locNet_, ccFrame(), res() + 1, 4, true, false, name() + string("D8H2DS"));
   //cout << "== NEW GRID2DS:" << endl;
   //cout << *grid2DS_;

   if (res() == 0)
      maxD_ = 0;
   else {
      double factor = parentScaleFac * 2.0L; // aperture 4

      scaleFac_ = factor;
      maxD_ = factor - 1.0L;

      //cout << res() << " " << aperture();
      //cout << " f: " << factor << " maxD: " << maxD_ << endl;
   }

   maxI_ = maxD();
   maxJ_ = maxD();
   mag_ = maxD() + 1;
   firstAdd_ = DgQ2DICoord(0, DgIVec2D(0, 0));
   lastAdd_ = DgQ2DICoord(11, DgIVec2D(0, 0));

   if (res() == 0)
      gridStats_.setNCells(12);
   else
      gridStats_.setNCells((parentNCells - 2) * aperture() + 2);

   createConverters();

   ///// calculate the statistics /////

   gridStats_.setPrecision(precision());
   //gridStats_.setNCells(bndRF().size());

   long double tmpLen = DgGeoSphRF::icosaEdgeKM();
////// NEEDS UPDATING
   gridStats_.setCellDistKM(tmpLen / pow(sqrt((long double) aperture()), res()));
/*
   else // mixed43
   {
      if (res() < numAp4())
      {
         tmpLen = tmpLen / pow((long double) 2.0, (long double) res());
      }
      else
      {
         tmpLen = tmpLen / pow((long double) 2.0, (long double) numAp4());
         if (res() > numAp4())
            tmpLen = tmpLen / pow(M_SQRT3, (long double) (res() - numAp4()));
      }
      gridStats_.setCellDistKM(tmpLen);
   }
*/

      // a = globeArea / ((#cells - 12) + (12 * 5/6))
      //   = globeArea / (#cells - 2);
      gridStats_.setCellAreaKM(DgGeoSphRF::totalAreaKM() / 
                       (gridStats_.nCells() - 2));

   gridStats_.setCLS(2.0L * 2.0L * DgGeoSphRF::earthRadiusKM() * 
                     asinl(sqrt(gridStats_.cellAreaKM() / M_PI) / 
                     (2.0L * DgGeoSphRF::earthRadiusKM())));

} // DgDmdIDGG::initialize

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
