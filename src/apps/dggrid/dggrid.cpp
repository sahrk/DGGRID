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
// dggrid.cpp: DGGRID main program
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <dglib/DgConstants.h>
#include <dglib/DgBase.h>

#include "OpBasic.h"

////////////////////////////////////////////////////////////////////////////////
void pause (const std::string& where)
{
   dgcout << "*** execution paused: " << where << std::endl;
   dgcout << "*** press ENTER to continue: ";
   scanf("%*c");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   //////// setup /////////////////

   //// parse the command line parameters ////

   // could have up to 2 flags and a metafile
   if (argc < 2 || argc > 4) {
      DgBase::testArgEqual(argc, argv, 1, std::string("metaFileName"));
   }

   // if we're here we have a reasonable # of cla's
   bool flagErr = false;
   bool hFlag = false;
   bool vFlag = false;
   bool hasMetaFile = false;
   std::string metaFileName;
   for (int i = 1; i < argc; i++) {
      // check if it's a flag
      if (*argv[i] == '-') {
         int numFlags = (int) strlen(argv[i]) - 1;
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
         report(std::string("invalid command line argument ") + std::string(argv[i]), DgBase::Fatal);
      } else { // must be the metafileName
         hasMetaFile = true;
         metaFileName = std::string(argv[i]);
      }

      if (flagErr) {
         report(std::string("invalid command line flag ") + std::string(argv[i]), DgBase::Fatal);
      }
   }

   if (hFlag || (hFlag && vFlag)) { // output is currently redundant
      dgcout << "DGGRID version " << DGGRID_VERSION << " released " << DGGRID_RELEASE_DATE << std::endl;
#ifdef USE_GDAL
      dgcout << "built with GDAL version " << std::string(GDALVersionInfo("VERSION_NUM")) << std::endl;
#else
      dgcout << "built without GDAL" << std::endl;;
#endif
      dgcout << "\nTo use:" << std::endl;
      dgcout << std::endl;
      dgcout << "dggrid metafileName" << std::endl;
      dgcout << std::endl;
      dgcout << "dggrid -v" << std::endl;
      dgcout << "dggrid -h" << std::endl;
      dgcout << std::endl;
      dgcout << "From:" << std::endl;
      dgcout << "Southern Terra Cognita Laboratory" << std::endl;
      dgcout << "website: http://www.discreteglobalgrids.org/" << std::endl;
      dgcout << "Kevin Sahr, Director" << std::endl;
      dgcout << "http://www.linkedin.com/in/Kevin-Sahr" << std::endl;
      dgcout << std::endl;
      dgcout << "License: GNU Affero General Public License v3.0" << std::endl;
      dgcout << std::endl;
      dgcout << "Github development:" << std::endl;
      dgcout << "https://github.com/sahrk/DGGRID" << std::endl;
   } else if (vFlag) {
      dgcout << "DGGRID version " << DGGRID_VERSION << " released " << DGGRID_RELEASE_DATE << std::endl;
#ifdef USE_GDAL
      dgcout << "built with GDAL version " << std::string(GDALVersionInfo("VERSION_NUM")) << std::endl;
#else
      dgcout << "built without GDAL" << std::endl;;
#endif
   }

   if (!hasMetaFile)
      exit(0);

   //// build and load the parameter list ////

   dgcout << "** executing DGGRID version " << DGGRID_VERSION;
#ifdef USE_GDAL
   dgcout << " with GDAL version " << std::string(GDALVersionInfo("VERSION_NUM"));
#else
   dgcout << " without GDAL";
#endif
   dgcout << " **\n";
   dgcout << "type sizes: big int: " << sizeof(long long int) * 8 << " bits / ";
   dgcout << "big double: " << sizeof(long double) * 8 << " bits\n";
   dgcout << "\n** using meta file " << metaFileName << "..." << std::endl;

   // create the operation object using parameters in the meta file
   OpBasic theOperation(metaFileName);
   theOperation.initialize();

   // echo the parameter list
   dgcout << "* parameter values:\n";
   dgcout << theOperation.pList << std::endl;

   if (theOperation.mainOp.pauseOnStart)
      pause("parameters loaded");

   // do the operation
   theOperation.execute();

   // grab the value before the op is cleaned
   bool pauseBeforeExit = theOperation.mainOp.pauseBeforeExit;

   theOperation.cleanupAll();

   if (pauseBeforeExit)
      pause("before exit");

   return 0;

} // main

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
