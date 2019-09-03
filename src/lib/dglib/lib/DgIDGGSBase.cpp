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
// DgIDGGBase.cpp: DgIDGGBase class implementation
//
// Version 7.0 - Kevin Sahr, 11/15/14
//
////////////////////////////////////////////////////////////////////////////////

#include "DgIDGGSBase.h"
#include "DgIDGGS.h"
#include "DgHexIDGGS.h"
#include "DgApSeq.h"

////////////////////////////////////////////////////////////////////////////////
const DgIDGGSBase*
DgIDGGSBase::makeRF (DgRFNetwork& network, const DgGeoSphRF& backFrame,
          const DgGeoCoord& vert0, long double azDegs, unsigned int apertureIn,
          int nResIn, const string& gridTopo, const string& name,
          const string& projTypeIn, bool isMixed43In, int numAp4In,
          bool isSuperfundIn, bool isApSeqIn, const DgApSeq& apSeqIn)
{
   const DgIDGGSBase* dg0 = NULL;

   if (gridTopo == "HEXAGON")
   {
      DgApSeq apSeq = apSeqIn;

      if (!isApSeqIn) // need to build an apSeq
      {
         int r;

         if (isMixed43In)
         {
            for (r = 0; r < numAp4In; r++)
               apSeq.addAperture(DgAperture(4));

            for (; r < nResIn - 1; r++)
               apSeq.addAperture(DgAperture(3));
         }
         else
            for (r = 0; r < nResIn - 1; r++)
               apSeq.addAperture(DgAperture((apertureIn == 3) ? 3 : 4));
      }
         
      //cout << "\nNEW HEX GRIDS: nResIn: " << nResIn << " apSeq: " << apSeq << endl;
      dg0 = new DgHexIDGGS(network, backFrame, vert0, azDegs, apertureIn, nResIn,
              gridTopo, name, projTypeIn, apSeq); 

      //cout << "\nIN MAKERF\n";
      //cout << *dg0 << endl;
   }
   else
   {
      dg0 = DgIDGGS::makeRF(network, backFrame, vert0, azDegs, 
            apertureIn, nResIn, gridTopo, name, projTypeIn, isMixed43In, 
            numAp4In, isSuperfundIn);
   }

   return dg0;

} // DgIDGGSBase::makeRF

////////////////////////////////////////////////////////////////////////////////
