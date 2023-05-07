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
// dggrid.cpp: DGGRID main program
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include <dglib/DgConstants.h>
#include "dggrid.h"
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgGeoProjConverter.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void orientGrid (MainParam& dp, DgGridPList& plist)
//
// Set the orientation parameters if not specified
//
{
   if (dp.placeRandom) // randomize grid orientation
   {
      dp.vert0 = dp.orientRand->nextGeo();

      dp.azimuthDegs = dp.orientRand->randInRange(0.0, 360.0);

      // set the paramlist to match so we can print it back out

      plist.setParam("dggs_orient_specify_type", "SPECIFIED");
      plist.setParam("dggs_num_placements", dgg::util::to_string(1));
      plist.setParam("dggs_vert0_lon", dgg::util::to_string(dp.vert0.lonDegs()));
      plist.setParam("dggs_vert0_lat", dgg::util::to_string(dp.vert0.latDegs()));
      plist.setParam("dggs_vert0_azimuth", dgg::util::to_string(dp.azimuthDegs));

      dgcout << "Grid " << dp.curGrid <<
           " #####################################################" << endl;
      dgcout << "grid #" << dp.curGrid << " orientation randomized to: " << endl;
      dgcout << plist << endl;
   }
   else if (dp.orientCenter && dp.curGrid == 1)
   {
      DgRFNetwork netc;
      const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(netc, "GS0", dp.earthRadius));

      long double lonc = 0.0, latc = 0.0;
      getParamValue(plist, "region_center_lon", lonc, false);
      getParamValue(plist, "region_center_lat", latc, false);

      const DgProjGnomonicRF& gnomc = *(DgProjGnomonicRF::makeRF(netc, "cgnom", DgGeoCoord(lonc, latc, false)));
      Dg2WayGeoProjConverter(geoRF, gnomc);

      DgLocation* gloc = gnomc.makeLocation(DgDVec2D(-7289214.618283,
                                                      7289214.618283));
      geoRF.convert(gloc);

      DgGeoCoord p0 = *geoRF.getAddress(*gloc);
      delete gloc;

      gloc = gnomc.makeLocation(DgDVec2D(2784232.232959, 2784232.232959));
      geoRF.convert(gloc);
      DgGeoCoord p1 = *geoRF.getAddress(*gloc);
      delete gloc;

      dp.vert0 = p0;
      dp.azimuthDegs = DgGeoSphRF::azimuth(p0, p1, false);
   }

} // void orientGrid

////////////////////////////////////////////////////////////////////////////////
void pause (const string& where)
{
   dgcout << "*** execution paused: " << where << endl;
   dgcout << "*** press ENTER to continue: ";
   scanf("%*c");
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   //////// setup /////////////////

   //// parse the command line parameters ////

   // could have up to 2 flags and a metafile
   if (argc < 2 || argc > 4) {
      DgBase::testArgEqual(argc, argv, 1, string("metaFileName"));
   }

   // if we're here we have a reasonable # of cla's
   bool flagErr = false;
   bool hFlag = false;
   bool vFlag = false;
   bool hasMetaFile = false;
   string metaFileName;
   for (int i = 1; i < argc; i++) {
      // check if it's a flag
      if (*argv[i] == '-') {
         int numFlags = strlen(argv[i]) - 1;
         if (numFlags == 0 || numFlags > 2) {
            flagErr = true;
         } else { // 1 or 2 flags

            if (argv[i][1] == 'h')
               hFlag = true;
            else if (argv[i][1] == 'v')
               vFlag = true;
            else {
               flagErr = true;
               break;
            }

            if (!flagErr && numFlags > 1) {
               if (argv[i][2] == 'h')
                  hFlag = true;
               else if (argv[i][2] == 'v')
                  vFlag = true;
               else {
                  flagErr = true;
               }
            }
         }
      } else if (hasMetaFile) { // metafileName already encountered
         report(string("invalid command line argument ") + string(argv[i]), DgBase::Fatal);
      } else { // must be the metafileName
         hasMetaFile = true;
         metaFileName = string(argv[i]);
      }

      if (flagErr) {
         report(string("invalid command line flag ") + string(argv[i]), DgBase::Fatal);
      }
   }

   if (hFlag || (hFlag && vFlag)) { // output is currently redundant
      dgcout << "DGGRID version " << DGGRID_VERSION << " released " << DGGRID_RELEASE_DATE << endl;
      dgcout << "How to use:" << endl;
      dgcout << endl;
      dgcout << "dggrid metafileName" << endl;
      dgcout << endl;
      dgcout << "dggrid -v" << endl;
      dgcout << "dggrid -h" << endl;
      dgcout << endl;
      dgcout << "Credits:" << endl;
      dgcout << "Southern Terra Cognita Laboratory" << endl;
      dgcout << "website: http://www.discreteglobalgrids.org/" << endl;
      dgcout << "Kevin Sahr, Director" << endl;
      dgcout << "http://www.linkedin.com/in/Kevin-Sahr" << endl;
      dgcout << endl;
      dgcout << "License: GNU Affero General Public License v3.0" << endl;
      dgcout << endl;
      dgcout << "Github development:" << endl;
      dgcout << "https://github.com/sahrk/DGGRID" << endl;
   } else if (vFlag) {
      dgcout << "DGGRID version " << DGGRID_VERSION << " released " << DGGRID_RELEASE_DATE << endl;
   }

   if (!hasMetaFile)
      exit(0);


   //// build and load the parameter list ////

   dgcout << "** executing DGGRID version " << DGGRID_VERSION << " **\n";
   dgcout << "type sizes: big int: " << sizeof(long long int) * 8 << " bits / ";
   dgcout << "big double: " << sizeof(long double) * 8 << " bits\n";

   dgcout << "\n** loading meta file " << metaFileName << "..." << endl;

   // first parse the meta file
   DgGridPList plist; // builds the parameter list
   plist.loadParams(metaFileName);

   // now build our "global parameters" structure
   string tmp;
   getParamValue(plist, "dggrid_operation", tmp, false);
   MainParam* pdp = 0;
   if (tmp == "GENERATE_GRID")
      pdp = new GridGenParam(plist);
   else if (tmp == "OUTPUT_STATS")
      pdp = new MainParam(plist);
   else if (tmp == "BIN_POINT_VALS")
      pdp = new BinValsParam(plist);
   else if (tmp == "BIN_POINT_PRESENCE")
      pdp = new BinPresenceParam(plist);
   else if (tmp == "TRANSFORM_POINTS")
      pdp = new TransformParam(plist);

   // echo the parameter list
   dgcout << "* using parameter values:\n";
   dgcout << plist << endl;

   if (pdp->pauseOnStart)
      pause("parameters loaded");

   // execute the operation
   if (tmp == "GENERATE_GRID")
      doGridGen(static_cast<GridGenParam&>(*pdp), plist);
   else if (tmp == "OUTPUT_STATS")
      doTable(*pdp, plist);
   else if (tmp == "BIN_POINT_VALS")
      doBinVals(static_cast<BinValsParam&>(*pdp), plist);
   else if (tmp == "BIN_POINT_PRESENCE")
      doBinPresence(static_cast<BinPresenceParam&>(*pdp), plist);
   else if (tmp == "TRANSFORM_POINTS")
      doTransforms(static_cast<TransformParam&>(*pdp), plist);

   bool pauseBeforeExit = pdp->pauseBeforeExit;

   delete pdp;

   if (pauseBeforeExit)
      pause("before exit");

   return 0;

} // main

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
