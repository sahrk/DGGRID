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
// DgDiscRFS2D.cpp: DgDiscRFS2D class implementation
//
// Version 7.0 - Kevin Sahr, 12/14/14
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include "DgDiscRFS2D.h"
#include "DgDmdD4Grid2DS.h"
#include "DgDmdD8Grid2DS.h"
#include "DgHexGrid2DS.h"
#include "DgSqrD4Grid2DS.h"
#include "DgSqrD8Grid2DS.h"
#include "DgTriGrid2DS.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgDiscRFS2D* 
DgDiscRFS2D::makeRF (DgRFNetwork& net, const DgRF<DgDVec2D, long double>& cc0,
   int nRes, unsigned int aperture, bool isCongruent, bool isAligned, 
   const string& name, const string geometry, bool isMixed43, int numAp4,
   bool isSuperfund, bool isApSeq, const DgApSeq& apSeq)
{
   DgDiscRFS2D* dg0 = 0;
   if (geometry == "sqr8")
   {
      dg0 = new DgSqrD8Grid2DS(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "SqrD82DS");
   }
   else if (geometry == "sqr4")
   {
      dg0 = new DgSqrD4Grid2DS(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "SqrD42DS");
   }
   else if (geometry == "dmd8")
   {
      dg0 = new DgDmdD8Grid2DS(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "DmdD82DS");
   }
   else if (geometry == "dmd4")
   {
      dg0 = new DgDmdD4Grid2DS(net, cc0, nRes, aperture, isCongruent,
                               isAligned, "DmdD82DS");
   }
   else if (geometry == "hex")
   {
      dg0 = new DgHexGrid2DS(net, cc0, nRes, aperture, isCongruent,
                   isAligned, "HexC12DS", isMixed43, numAp4, isSuperfund,
                   isApSeq, apSeq);
   }
   else if (geometry == "tri")
   {
      dg0 = new DgTriGrid2DS(net, cc0, nRes, aperture, isCongruent,
                             isAligned, "Tri2DS");
   }
   else
   {
     report("DgDiscRFS2D::makeRF() invalid or unimplemented geometry type: " + 
            geometry, DgBase::Fatal);
   }

   return dg0;

} // DgDiscRFS2D* DgDiscRFS2D::makeRF

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
