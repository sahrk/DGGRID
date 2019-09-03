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
// DgIDGGS.cpp: DgIDGGS class implementation
//
// Version 7.0 - Kevin Sahr, 11/16/14
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include "DgIDGGS.h"
#include "DgIDGGS3H.h"
#include "DgIDGGS43H.h"
#include "DgIDGGS4H.h"
#include "DgIDGGS4D.h"
#include "DgIDGGS4T.h"
#include "DgSuperfund.h"

////////////////////////////////////////////////////////////////////////////////
const DgIDGGS*
DgIDGGS::makeRF (DgRFNetwork& network, const DgGeoSphRF& backFrame,
          const DgGeoCoord& vert0, long double azDegs, unsigned int apertureIn, 
          int nRes, const string& gridTopo, const string& name,
          const string& projTypeIn, bool isMixed43In, int numAp4In, 
          bool isSuperfundIn, bool isApSeqIn, const DgApSeq& apSeqIn)
{
   if (isApSeqIn)
      report("DgIDGGS::makeRF(): isApSeq must be false", DgBase::Fatal);

   DgIDGGS* dg0 = 0;

   string apErrStr = string("DgIDGGS::makeRF(): invalid aperture " +
                         dgg::util::to_string(apertureIn) +
                         string(" for grid topo ") + gridTopo);

   string theName = name;
   bool defaultName = (theName == string("IDGGS"));

   if (gridTopo == "HEXAGON")
   {
      if (!isMixed43In)
      {
         if (apertureIn == 4)
         {
            if (defaultName) theName = projTypeIn + string("4H");
            dg0 = new DgIDGGS4H(network, backFrame, vert0, azDegs, nRes, 
                          theName, projTypeIn);
         }
         else if (apertureIn == 3)
         {
            if (defaultName) theName = projTypeIn + string("3H");
            dg0 = new DgIDGGS3H(network, backFrame, vert0, azDegs, nRes, 
                          theName, projTypeIn);
         }
         else
            report(apErrStr, DgBase::Fatal);
      }
      else
      {
         if (defaultName) theName = projTypeIn + string("43H");
         dg0 = new DgIDGGS43H(network, backFrame, vert0, azDegs, nRes, theName,
                        projTypeIn, numAp4In, isSuperfundIn);
      }
   }
   else if (gridTopo == "DIAMOND")
   {
      if (apertureIn == 4)
      {
         if (defaultName) theName = projTypeIn + string("4D");
         dg0 = new DgIDGGS4D(network, backFrame, vert0, azDegs, nRes, 
                       theName, projTypeIn);
      }
      else
            report(apErrStr, DgBase::Fatal);
   }
   else if (gridTopo == "TRIANGLE")
   {
      if (apertureIn == 4)
      {
         if (defaultName) theName = projTypeIn + string("4T");
         dg0 = new DgIDGGS4T(network, backFrame, vert0, azDegs, nRes, 
                       theName, projTypeIn);
      }
      else
            report(apErrStr, DgBase::Fatal);
   }
   else
   {
      report("DgIDGGS::makeRF() invalid or unimplemented grid topology: " +
         gridTopo, DgBase::Fatal);
   }

   return dg0;

} // const DgIDGG& makeRF

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgIDGGS::DgIDGGS (DgRFNetwork& network, const DgGeoSphRF& backFrame,
                  const DgGeoCoord& vert0, long double azDegs, 
                  unsigned int aperture, int nRes, const string& gridTopo, 
                  const string& name, const string& projType, bool isMixed43, 
                  int numAp4, bool isSuperfund, bool isApSeq,
                  const DgApSeq& apSeq)
        : DgIDGGSBase (network, backFrame, vert0, azDegs, nRes, aperture, name, 
                       gridTopo, projType, !(isMixed43 || isApSeq)),
          numAp4_ (numAp4), isSuperfund_ (isSuperfund), isApSeq_ (isApSeq),
          apSeq_ (apSeq)
{
   
   undefLoc_ = makeLocation(undefAddress());

   // create the DGGs

   for (int i = 0; i < nRes; i++)
   {
      if (!isSuperfund)
         (*grids_)[i] = new DgIDGG(this, backFrame, vert0, azDegs, aperture, i,
            "DDG", gridTopo, projType, isMixed43, numAp4, isSuperfund);
      else
         (*grids_)[i] = new DgIDGG(this, backFrame, vert0, azDegs, aperture, i,
            "DDG", gridTopo, projType, isMixed43, numAp4, isSuperfund,
            actualRes2sfRes(i));

      new Dg2WayResAddConverter<DgQ2DICoord, DgGeoCoord, long double> 
                                                   (*this, *(grids()[i]), i);
   }

   isAligned_ = idgg(0).isAligned();
   isCongruent_ = idgg(0).isCongruent();

} // DgIDGGS::DgIDGGS

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
