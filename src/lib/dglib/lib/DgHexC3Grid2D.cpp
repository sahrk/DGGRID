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
// DgHexC3Grid2D.cpp: DgHexC3Grid2D class implementation
//
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "DgContCartRF.h"
#include "DgHexC3Grid2D.h"
#include "DgHexC1Grid2D.h"
#include "DgHexC2Grid2D.h"
#include "DgPolygon.h"

////////////////////////////////////////////////////////////////////////////////
DgHexC3Grid2D::DgHexC3Grid2D (DgRFNetwork& networkIn, 
             const DgRF<DgDVec2D, long double>& ccFrameIn, bool isClassI,
             const string& nameIn)
         : DgDiscRF2D (networkIn, ccFrameIn, nameIn, M_1_SQRT7, M_1_SQRT7, 
                       M_SQRT7_2, 1.0L),
           isClassI_ (isClassI)
{ 
   area_ = c(); 

   // create the surrogate hex grid: a class I/II hex grid rotated ap7 degrees

   DgContCartRF* surCCRF = new DgContCartRF(network(), nameIn + string("SurBF"));

   long double rotDegs = -M_AP7_ROT_DEGS;
/*
   if (!isClassI)
      rotDegs *= -30.0L;

   new Dg2WayContAffineConverter(backFrame(), *surCCRF, 1.0L, rotDegs);
   surrogate_ = new DgHexC1Grid2D(network(), *surCCRF, nameIn + string("Sur"));
*/
   new Dg2WayContAffineConverter(backFrame(), *surCCRF, 1.0L, rotDegs);
   if (isClassI)
      surrogate_ = new DgHexC1Grid2D(network(), *surCCRF, nameIn + string("Sur"));
   else
      surrogate_ = new DgHexC2Grid2D(network(), *surCCRF, nameIn + string("Sur"));

   // create the substrate hex grid: a class I hex grid one aperture 7 resolution
   // finer (or an aperture 3 + 7 if Class II)

   DgContCartRF* subCCRF = new DgContCartRF(network(), nameIn + string("SubBF"));

   long double scaleFac = M_SQRT7;
   if (!isClassI)
      scaleFac *= M_SQRT3;

   new Dg2WayContAffineConverter(backFrame(), *subCCRF, scaleFac);
   substrate_ = new DgHexC1Grid2D(network(), *subCCRF, nameIn + string("Sub"));

} // DgHexC3Grid2D::DgHexC3Grid2D

////////////////////////////////////////////////////////////////////////////////
long long int
DgHexC3Grid2D::dist (const DgIVec2D& add1, const DgIVec2D& add2) const
{
   DgLocation* loc1 = substrate().makeLocation(add1);
   DgLocation* loc2 = substrate().makeLocation(add2);

   surrogate().convert(loc1);
   surrogate().convert(loc2);

   long long int d = surrogate().dist(*(surrogate().getAddress(*loc1)),
                            *(surrogate().getAddress(*loc2)));

   delete loc1;
   delete loc2;

   return d;

} // int DgHexC3Grid2D::dist

////////////////////////////////////////////////////////////////////////////////
void
DgHexC3Grid2D::setAddVertices (const DgIVec2D& add, DgPolygon& vec) const
{
   DgLocation* tmpLoc = substrate().makeLocation(add);
   surrogate().setVertices(*tmpLoc, vec);

   backFrame().convert(vec);

   delete tmpLoc;

} // void DgHexC3Grid2D::setAddVertices

////////////////////////////////////////////////////////////////////////////////
void
DgHexC3Grid2D::setAddNeighbors (const DgIVec2D& add, DgLocVector& vec) const
{
   DgLocation* tmpLoc = substrate().makeLocation(add);

   DgLocVector tmpVec;
   surrogate().setNeighbors(*tmpLoc, tmpVec);
   substrate().convert(tmpVec);

   delete tmpLoc;

   vector<DgAddressBase*>& v = vec.addressVec();
   for (long long int i = 0; i < tmpVec.size(); i++)
   {
      v.push_back(new DgAddress<DgIVec2D>(
                     *(substrate().getAddress(tmpVec[i]))));
   }

} // void DgHexC3Grid2D::setAddNeighbors

////////////////////////////////////////////////////////////////////////////////
void
DgHexC3Grid2D::setAddNeighborsBdry2 (const DgIVec2D& add, DgLocVector& vec) const
{
   DgLocation* tmpLoc = substrate().makeLocation(add);

   DgLocVector tmpVec;
   surrogate().setNeighborsBdry2(*tmpLoc, tmpVec);
   substrate().convert(tmpVec);

   delete tmpLoc;

   vector<DgAddressBase*>& v = vec.addressVec();
   for (long long int i = 0; i < tmpVec.size(); i++)
   {
      v.push_back(new DgAddress<DgIVec2D>(
                     *(substrate().getAddress(tmpVec[i]))));
   }

} // void DgHexC3Grid2D::setAddNeighborsBdry2

////////////////////////////////////////////////////////////////////////////////
DgIVec2D 
DgHexC3Grid2D::quantify (const DgDVec2D& point) const
{
   DgLocation* tmpLoc = backFrame().makeLocation(point);

   surrogate().convert(tmpLoc);  // to quantify

   substrate().convert(tmpLoc);  // to set "correct" address

   DgIVec2D add(*(substrate().getAddress(*tmpLoc)));

   delete tmpLoc;

   return add;

} // DgIVec2D DgHexC3Grid2D::quantify

////////////////////////////////////////////////////////////////////////////////
DgDVec2D 
DgHexC3Grid2D::invQuantify (const DgIVec2D& add) const
{
   DgLocation* tmpLoc = substrate().makeLocation(add);

   backFrame().convert(tmpLoc);

   DgDVec2D point(*(backFrame().getAddress(*tmpLoc)));

   delete tmpLoc;

   return point;

} // DgDVec2D DgHexC3Grid2D::invQuantify

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
