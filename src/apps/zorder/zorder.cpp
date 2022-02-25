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
// zorder.cpp: 
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include <dglib/DgIDGGS3H.h>
#include <dglib/DgIDGGS4H.h>
#include <dglib/DgIDGGS4T.h>
#include <dglib/DgIDGGS4D.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgZOrderRF.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   ///// create the DGG /////

   // create the reference frame (RF) conversion network
   DgRFNetwork net0;

   // create the geodetic reference frame
   // reference frames must be created dynamically using makeRF
   // they will be deleted by the Network
   const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(net0, "GS0"));

   // create the ISEA3H grid system with resolutions 0-9; requires a
   // fixed icosahedron vertex and edge azimuth
   DgGeoCoord vert0(11.25L, 58.28252559L, false); // args: lon, lat, isRadians
   long double azimuth = 0.0L;

   // all DGGS's must be created using a factory makeRF method
   // the DGGS is memory managed by the DgRFNetwork
   const DgIDGGS3H* idggsPtr = DgIDGGS3H::makeRF(net0, geoRF, vert0, azimuth, 12); 
   const DgIDGGS3H& idggs = *idggsPtr;

   // get the resolution 7 dgg from the dggs
   const DgIDGG& dgg = idggs.idgg(2);
   cout << dgg.gridStats() << endl;

   //////// now use the DGG /////////

   ///// given a point in lon/lat, get the cell it lies within /////

   // first create a DgLocation in geoRF coordinates
   //DgGeoCoord geoAddress(-122.7083, 42.1947, false);
   DgGeoCoord geoAddress(-124.839, 42.9778, false);
   DgLocation* thePt = geoRF.makeLocation(geoAddress);
   cout << "the point " << *thePt << endl;

   // converting the point location to the dgg RF determines which cell it's in
   dgg.convert(thePt);
   cout << "* lies in cell " << *thePt << endl;
/*
geoRF.convert(thePt);
cout << *thePt << endl;
exit(1);
*/

   DgZOrderRF& zRF = *(DgZOrderRF::makeRF(net0, "zRF"));
   new Dg2WayZOrderConverter(dgg, zRF);
   zRF.convert(thePt);
   cout << "* zorder " << *thePt << endl;
   dgg.convert(thePt);
   cout << "* is cell " << *thePt << endl;

   delete thePt;

   cout << "=========" << endl;
   DgLocation* addLoc = new DgLocation(dgg.bndRF().first());
   while (dgg.bndRF().validLocation(*addLoc)) {

      DgQ2DICoord start = *dgg.getAddress(*addLoc);
      cout << (*addLoc) << " -> ";
      zRF.convert(addLoc);
      cout << (*addLoc) << " -> ";
      dgg.convert(addLoc);
      cout << (*addLoc);
      DgQ2DICoord finish = *dgg.getAddress(*addLoc);
      if (start != finish) {
         cout << "ERROR" << endl;
         exit(1);
      }

      cout << endl;

      dgg.bndRF().incrementLocation(*addLoc);
   }

   return 0;

} // main 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
