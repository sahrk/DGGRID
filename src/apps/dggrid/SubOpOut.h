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
// SubOpOut.h: this file defines the parameters for cell output
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPOUT_H
#define SUBOPOUT_H

#include <dglib/DgGeoSphRF.h>
#include <dglib/DgInShapefileAtt.h>
#include <dglib/DgAddressType.h>
#include <dglib/DgRunningStats.h>

#include "SubOpBasic.h"

class DgIDGGSBase;
class DgIDGGBase;
class DgQ2DICoord;
class DgContCartRF;
class DgLocation;
class DgPolygon;
class DgOutLocFile;
class DgOutShapefile;
class DgOutPRCellsFile;
class DgOutPRPtsFile;
class DgOutNeighborsFile;
class DgOutChildrenFile;
class DgRandom;
class DgDataList;

struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
struct SubOpOut : public SubOpBasic {

   SubOpOut (OpBasic& op, bool activate = true);

   // DgApSubOperation virtual methods that use the pList
   virtual int initializeOp (void);
   virtual int setupOp (void);
   virtual int cleanupOp (void);

   virtual int executeOp (void);

   // helper methods
   void genRandPts (const DgQ2DICoord& add2D, const string& label);
   void outputCellAdd2D (const DgLocation& add2D, const string* labelIn = nullptr,
               DgDataList* dataList = nullptr);

   void resetFiles (void);

   // the parameters
   const DgRFBase* pOutRF;     // RF for output addresses
   const DgRFBase* pChdOutRF;  // RF for output addresses at child resolution
   dgg::addtype::DgAddressType outAddType; // output address form
   bool outSeqNum;             // are the output addresses sequence numbers?
   char outputDelimiter;

   int nDensify;          // number of points-per-edge of densification

   DgGeoSphRF::DgLonWrapMode lonWrapMode;
               // how to handle outputting cells that straddle the anti-meridian
   bool unwrapPts;        // unwrap point longitude to match cell boundary

   bool doRandPts;        // generate random points for the cells
   DgRandom* ptsRand;     // RNG for generating random points
   int nRandPts;          // # of random pts generated for each hex

   unsigned long long int nCellsTested;
   unsigned long long int nCellsAccepted;

   DgRunningStats runStats;

   string dataOutType;
   string dataOutFileNameBase;
   string dataOutFileName;

   string cellOutType;
   string gdalCellDriver;
   string pointOutType;
   string gdalPointDriver;
   string randPtsOutType;
   string gdalCollectDriver;

   string neighborsOutType;
   string childrenOutType;
   string neighborsOutFileNameBase;
   string neighborsOutFileName;
   string childrenOutFileNameBase;
   string childrenOutFileName;

   string cellOutFileNameBase;
   string cellOutFileName;
   string ptOutFileNameBase;
   string ptOutFileName;
   string collectOutFileNameBase;
   string collectOutFileName;
   string randPtsOutFileNameBase;
   string randPtsOutFileName;
   string metaOutFileNameBase;
   string metaOutFileName;

   int    shapefileIdLen;  // global_id string field length

   string kmlColor;
   int    kmlWidth;
   string kmlName;
   string kmlDescription;

   ofstream* dataOut;    // text output with no geometry
   DgOutLocFile *cellOut, *ptOut, *collectOut, *randPtsOut;
   DgOutShapefile *cellOutShp, *ptOutShp;
   DgOutPRCellsFile *prCellOut;
   DgOutNeighborsFile *nbrOut;
   DgOutChildrenFile *chdOut;

   bool concatPtOut;
   char formatStr[50];
   bool useEnumLbl;
   unsigned long int nOutputFile; // # of current output file
   unsigned long long int nCellsOutputToFile; // cells output to current file

   unsigned long int maxCellsPerFile; // max cells in a single output file
   unsigned long int outFirstSeqNum;  // start generating with this seqnum
   unsigned long int outLastSeqNum;   //  generate through this one

   bool buildShapeFileAttributes; // create fields for shapefile output
   bool buildClipFileAttributes;  // use clipping shape files (vs. others)

   int shapefileDefaultInt;
   long double shapefileDefaultDouble;
   string shapefileDefaultString;

   vector<string> attributeFiles;
   bool outCellAttributes; // true if cell output file is a shapefile
   bool outPointAttributes; // true if point output file is a shapefile
   set<DgDBFfield> allFields; // union of all input file attribute fields
   set<DgDBFfield> curFields; // union of all intersected fields for the
                              // current cell
};

////////////////////////////////////////////////////////////////////////////////

#endif
