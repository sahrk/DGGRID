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
// transform.cpp: perform transformations of addresses
//
// Created by Kevin Sahr, October 1, 2001
//
//    version 3.1b: November 11, 2001
//    version 4.3b: June 21, 2003
//    version 6.2b: October 25, 2014
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace std;

#include "dggrid.h"
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgCell.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgInputStream.h>
#include <dglib/DgOutAIGenFile.h>
#include <dglib/DgOutputStream.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
TransformParam::TransformParam (DgParamList& plist)
      : MainParam(plist), nDensify (1)
{
      using namespace dgg;

      /////// fill state variables from the parameter list //////////

      // input file name

      getParamValue(plist, "input_file_name", inFileName, false);

      // output file name

      getParamValue(plist, "output_file_name", outFileNameBase, false);

      getParamValue(plist, "densification", nDensify, false);

      if (inAddType == dgg::addtype::Geo && outAddType == dgg::addtype::Geo) {
         ::report("TransformParam::TransformParam() At least one of the " +
                  string("address types must be non-GEO"), DgBase::Fatal);
      }

} // TransformParam::TransformParam

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void TransformParam::dump (void)
{
   MainParam::dump();

   dgcout << "BEGIN TRANSFORM PARAMETER DUMP" << endl;

   dgcout << " outFileNameBase: " << outFileNameBase << endl;
   dgcout << " outFileName: " << outFileName << endl;
   dgcout << " outAddType: " << outAddType << endl;
   dgcout << " outputDelimiter: " << outputDelimiter << endl;
   dgcout << " nDensify: " << nDensify << endl;

   dgcout << " inFileName: " << inFileName << endl;
   dgcout << " inAddType: " << inAddType << endl;
   dgcout << " inSeqNum: " << inSeqNum << endl;

  dgcout << "END TRANSFORM PARAMETER DUMP" << endl;

} // void TransformParam::dump

////////////////////////////////////////////////////////////////////////////////
void doTransform (TransformParam& dp)
{
   ////// create the reference frames ////////

   DgRFNetwork net0;
   const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(net0, dp.datum, dp.earthRadius));
   const DgIDGGSBase *idggs = DgIDGGSBase::makeRF(net0, geoRF, dp.vert0,
             dp.azimuthDegs, dp.aperture, dp.actualRes+2, dp.gridTopo,
             dp.gridMetric, "IDGGS", dp.projType, dp.isMixed43, dp.numAp4,
             dp.isSuperfund, dp.isApSeq, dp.apSeq);

   const DgIDGGBase& dgg = idggs->idggBase(dp.actualRes);

   dgcout << "Res " << dgg.outputRes() << " " << dgg.gridStats() << endl;

   // set-up to convert to degrees
   DgGeoSphDegRF::makeRF(geoRF, geoRF.name() + "Deg");

   // set-up the input reference frame
   MainParam::addressTypeToRF(dp, dgg, true);
   if (!dp.pInRF)
      ::report("doTransform(): invalid input RF", DgBase::Fatal);
   const DgRFBase& inRF = *dp.pInRF;

   // set-up the output reference frame
   MainParam::addressTypeToRF(dp, dgg, false);
   if (!dp.pOutRF)
      ::report("doTransform(): invalid output RF", DgBase::Fatal);
   const DgRFBase& outRF = *dp.pOutRF;

   // set the precision

   const_cast<DgRFBase&>(outRF).setPrecision(dp.precision);

   // now process the addresses in the input file

   const int maxLine = 1000;
   char buff[maxLine];
   const char* remainder;

   dgcout << "transforming values..." << endl;

   DgInputStream inFile(dp.inFileName, "", DgBase::Fatal);
   ofstream* pOutFile;
   pOutFile = new DgOutputStream(dp.outFileName, "", DgBase::Fatal);

   ofstream& outFile = *pOutFile;

   char delimStr[2];
   delimStr[0] = dp.inputDelimiter;
   delimStr[1] = '\0';

   while (1)
   {
      // get the next line

      inFile.getline(buff, maxLine);
      if (inFile.eof()) break;

      // parse the address

      DgLocation* loc = NULL;
      if (dp.inSeqNum) {
         char* snStr;
         snStr = strtok(buff, delimStr);
         unsigned long int sNum;
         if (sscanf(snStr, "%lu", &sNum) != 1) {
            ::report("doTransform(): invalid SEQNUM " + string(snStr),
                     DgBase::Fatal);
         }

         loc = static_cast<const DgIDGGBase&>(inRF).bndRF().locFromSeqNum(sNum);
         remainder = &(buff[strlen(snStr) + 1]);
      } else {
         loc = new DgLocation(inRF);
         remainder = loc->fromString(buff, dp.inputDelimiter);
      }

      // convert the address
      outRF.convert(loc);

      // output the converted line

/*
      if (outAIG) {
         const DgIDGGBase& idgg = static_cast<const DgIDGGBase&>(outRF);
         unsigned long int sn = idgg.bndRF().seqNum(*loc);
         DgPolygon verts(idgg);
         idgg.setVertices(*loc, verts, dp.nDensify);
         DgCell cell(geoRF, dgg::util::to_string(sn), *loc, new DgPolygon(verts));
         static_cast<DgOutAIGenFile&>(outFile) << cell;
      } else
*/

         if (dp.outSeqNum) {
            outFile << static_cast<const DgIDGGBase&>(outRF).bndRF().seqNum(*loc);
         } else {
            outFile << loc->asString(dp.outputDelimiter);
         }

         while (remainder && isspace(*remainder)) remainder++;
         if (remainder && strlen(remainder) > 0)
            outFile << dp.outputDelimiter << remainder << endl;
         else
            outFile << endl;

      delete loc;

   }

   inFile.close();
   outFile.close();
   delete pOutFile;

} // void doTransform

////////////////////////////////////////////////////////////////////////////////
void doTransforms (TransformParam& dp, DgGridPList& plist)
{
   for (dp.curGrid = 1; dp.curGrid <= dp.numGrids; dp.curGrid++)
   {
      // first get the grid placement

      dp.outFileName = dp.outFileNameBase;
      dp.metaOutFileName = dp.metaOutFileNameBase;

      orientGrid(dp, plist);

      if (dp.numGrids > 1)
      {
         string suffix = string(".") + dgg::util::to_string(dp.curGrid, 4);
         dp.metaOutFileName = dp.metaOutFileName + suffix;
         dp.outFileName = dp.outFileName + suffix;
      }

      if (dp.numGrids > 1 || dp.placeRandom)
      {
         ofstream metaOutFile;
         metaOutFile.open(dp.metaOutFileName.c_str());
         metaOutFile.setf(ios::fixed, ios::floatfield);
         metaOutFile.precision(12);
         metaOutFile << plist;
         metaOutFile.close();
      }

      // now do it

      if (dp.curGrid == dp.numGrids) dp.lastGrid = true;

      doTransform(dp);

      dgcout << endl;
   }

   dgcout << "** transformation complete **" << endl;

} // void doBinVals

////////////////////////////////////////////////////////////////////////////////
