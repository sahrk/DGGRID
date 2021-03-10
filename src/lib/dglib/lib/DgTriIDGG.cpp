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
// DgTriIDGG.cpp: DgTriIDGG class implementation
//
// Kevin Sahr, 8/12/20
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>
#include <cfloat>

#include "DgTriIDGG.h"
#include "DgIDGGS4T.h"
#include "DgTriGrid2DS.h"
#include "DgSeriesConverter.h"
#include "DgRadixString.h"
#include "DgBoundedIDGG.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgTriIDGG::DgTriIDGG (const DgIDGGS4T& dggs, unsigned int aperture,
              int res, const string& name, unsigned int precision)
   : DgIDGGBase (&dggs, dggs.geoRF(), aperture, res, name, precision),
	   scaleFac_ (1.0L)
{ 
   initialize();

} // DgTriIDGG::DgTriIDGG

////////////////////////////////////////////////////////////////////////////////
DgTriIDGG::DgTriIDGG (const DgTriIDGG& rfIn)
   : DgIDGGBase (rfIn.dggs(), rfIn.geoRF(), rfIn.aperture(), 
                 rfIn.res(), rfIn.name(), rfIn.precision()),
	scaleFac_ (rfIn.scaleFac())
{ 
   initialize();

} // DgTriIDGG::DgTriIDGG

////////////////////////////////////////////////////////////////////////////////
DgTriIDGG::~DgTriIDGG (void) { }

////////////////////////////////////////////////////////////////////////////////
const DgIDGGS4T& 
DgTriIDGG::triDggs (void) const 
{ return *(static_cast<const DgIDGGS4T*>(dggs())); }

////////////////////////////////////////////////////////////////////////////////
void
DgTriIDGG::initialize (void)
{
   // verify parameter validity

   string apErrStr = string("DgTriIDGG::initialize(): invalid aperture " + 
                         dgg::util::to_string(aperture()) + 
                         string(" for grid topo ") + gridTopo());

   if (gridTopo() != "TRIANGLE") {
      report("DgTriIDGG::initialize(): invalid grid topo " + gridTopo(), 
             DgBase::Fatal);

      if (aperture() != 4) report(apErrStr, DgBase::Fatal);
   }

   // create some internal data structures
   undefLoc_ = makeLocation(undefAddress());
   sphIcosa_ = new DgSphIcosa(vert0(), azDegs());

   isAligned_ = true;
   isCongruent_ = false;

   // initialize parent values as if this is grid res 0
   long double parentScaleFac = 1.0;
   unsigned long long int parentNCells = 1;

   // get actual parent values if there is a parent grid
   if (res() > 0) {
      const DgTriIDGG& parentIDGG = triDggs().triIdgg(res() - 1);

      parentScaleFac = parentIDGG.scaleFac();
      parentNCells = parentIDGG.gridStats().nCells();
   }

   // set-up local network to scale so that quad (and consequently tri) edge 
   // length is 1.0
   ccFrame_ = new DgContCartRF(locNet_, name() + "CC1");
   grid2DS_ = new DgTriGrid2DS(locNet_, ccFrame(), res() + 1, 4, true, false, name() + string("H2DS"));
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

} // DgTriIDGG::initialize

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
