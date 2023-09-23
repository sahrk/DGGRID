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
// SubOpIn.h: this file defines the basic input file parameters
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPIN_H
#define SUBOPIN_H

#include <dglib/DgAddressType.h>

#include "SubOpBasic.h"

struct OpBasic;
class DgLocationData;
//class DgInLocTextFile;
class DgInLocStreamFile;

////////////////////////////////////////////////////////////////////////////////
struct SubOpIn : public SubOpBasic {

   SubOpIn (OpBasic& op, bool activate = true);

   // DgApSubOperation virtual methods that use the pList
   virtual int initializeOp (void);
   virtual int setupOp (void);
   virtual int cleanupOp (void);

   // create the DGG
   virtual int executeOp (void);

   // user methods

   void resetInFile (void);

   // returns nullptr if no more input lines to read
   DgLocationData* getNextLoc (void);

   // internal helper methods
   DgInLocStreamFile* makeNewInFile (const DgRFBase& rfIn,
                     const string* fileNameIn = nullptr,
                     DgBase::DgReportLevel failLevel = DgBase::Fatal);

   // the parameters
   vector<string> inputFiles;
   unsigned int fileNum;    // current file if multiple files
   //DgInLocTextFile* inFile;
   DgInLocStreamFile* inFile;
   string inTextFileName;
   const DgRFBase* pInRF;   // RF for input addresses
   dgg::addtype::DgAddressType inAddType; // input address form
   bool isPointInput;
   string pointInputFileType;
   string gdalDriver;
   bool inSeqNum;           // is the input address sequence numbers?
   string addFldType;       // FIRST_FIELD, NAMED_FIELD, or GEO_POINT
   string addFldName;       // used when addFldType is NAMED_FIELD
   char inputDelimiter;
   string inFormatStr;
};

////////////////////////////////////////////////////////////////////////////////

#endif
