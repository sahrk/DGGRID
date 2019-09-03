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
// appex.cpp: simple application that demonstrates using the dglib library to
//            manipulate DGG cells.
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include "DgIDGGS4H.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   ///// create the DGG /////

   // create the reference frame (RF) conversion network
   DgRFNetwork net0;

   // create the geodetic reference frame
   DgGeoSphRF geoRF(net0, "GS0");

   // create the ISEA4H grid system with resolutions 0-9; requires a
   // fixed icosahedron vertex and edge azimuth
   DgGeoCoord vert0(11.25L, 58.28252559L, false); // args: lon, lat, isRadians
   long double azimuth = 0.0L;
   const DgIDGGS4H idggs(net0, geoRF, vert0, azimuth, 10); // last argument is
                                                      // number of resolutions

   // get the resolution 7 dgg from the dggs
   const DgIDGG& dgg = idggs.idgg(7);

   //////// now use the DGG /////////

   ///// given a point in lon/lat, get the cell it lies within /////

   // first create a DgLocation in geoRF coordinates
   DgGeoCoord geoAddress(-122.7083, 42.1947, false);
   DgLocation* thePt = geoRF.makeLocation(geoAddress);
   cout << "the point " << *thePt << endl;

   // converting the point location to the dgg RF determines which cell it's in
   dgg.convert(thePt);
   cout << "* lies in cell " << *thePt << endl;

   // we can get the cell's vertices, which are defined in geoRF
   DgPolygon verts;
   int ptsPerEdgeDensify = 3;
   dgg.setVertices(*thePt, verts, ptsPerEdgeDensify);
   cout << "* with cell boundary:\n" << verts << endl;

   // we can get the cell's center point by converting the cell back to geoRF
   geoRF.convert(thePt);
   cout << "* and cell center point:" << *thePt << endl;

   // we can extract the coordinates into primitive data types
   const DgGeoCoord& centCoord = *geoRF.getAddress(*thePt);
   double latRads = centCoord.lat();
   double lonRads = centCoord.lon();
   cout << "* center point lon,lat in radians: " 
        << lonRads << ", " << latRads << endl;

   const DgGeoCoord& firstVert = *geoRF.getAddress(verts[0]);
   double latDegs = firstVert.latDegs();
   double lonDegs = firstVert.lonDegs();
   cout << "* first boundary vertex lon,lat in degrees: " 
        << lonDegs << ", " << latDegs << endl;

   return 0;

} // main 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
