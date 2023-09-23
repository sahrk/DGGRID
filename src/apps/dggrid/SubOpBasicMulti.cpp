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
// SubOpBasicMulti.cpp: SubOpBasicMulti class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cstring>

#include <dglib/DgRandom.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgDataField.h>
#include <dglib/DgDataList.h>

#include "SubOpBasicMulti.h"
#include "OpBasic.h"

////////////////////////////////////////////////////////////////////////////////
SubOpBasicMulti::SubOpBasicMulti (OpBasic& _op, bool _activate)
   : SubOpBasic (_op, _activate)
{ }

////////////////////////////////////////////////////////////////////////////////
DgLocationData*
SubOpBasicMulti::inStrToPointLoc (const string& inStr) const {

   // setup for strtok
   char delimStr[2];
   delimStr[0] = op.inOp.inputDelimiter;
   delimStr[1] = '\0';

   // parse the address to get the node
   const char* remainder = nullptr;
   char* buff = new char[inStr.length() + 1];
   strcpy(buff, inStr.c_str());
   DgLocationData* loc = nullptr;
   if (op.inOp.inSeqNum) {
      char* snStr;
      snStr = strtok(buff, delimStr);
      unsigned long int sNum;
      if (sscanf(snStr, "%lu", &sNum) != 1) {
         ::report("doTransform(): invalid SEQNUM " + string(snStr),
                     DgBase::Fatal);
      }

      DgLocation* tmpLoc = static_cast<const DgIDGGBase&>(*op.inOp.pInRF).bndRF().locFromSeqNum(sNum);
      loc = new DgLocationData(*tmpLoc);
      delete tmpLoc;
      remainder = &(buff[strlen(snStr) + 1]);
   } else {
      loc = new DgLocationData(*op.inOp.pInRF);
      remainder = loc->fromString(buff, op.inOp.inputDelimiter);
   }

   // skip any whitespace
   while (remainder && isspace(*remainder)) remainder++;

   // remainder of line is a single data field
   // (this should really parse the line into multiple fields)
   DgDataList* data = nullptr;
   if (remainder && strlen(remainder) > 0) {
      data = new DgDataList();
      string remStr(remainder);
      DgDataFieldString* field = new DgDataFieldString("data", remStr);
      data->list().push_back(field);
   }

   // build the cell
   loc->setDataList(data);

   return loc;

} // DgLocationData* SubOpBasicMulti::inStrToPointLoc

/*
////////////////////////////////////////////////////////////////////////////////
DgCell*
SubOpBasicMulti::inStrToPointCell (const string& inStr) const {

   // setup for strtok
   char delimStr[2];
   delimStr[0] = op.inOp.inputDelimiter;
   delimStr[1] = '\0';

   // parse the address to get the node
   const char* remainder = nullptr;
   char* buff = new char[inStr.length() + 1];
   strcpy(buff, inStr.c_str());
   DgLocation* loc = nullptr;
   if (op.inOp.inSeqNum) {
      char* snStr;
      snStr = strtok(buff, delimStr);
      unsigned long int sNum;
      if (sscanf(snStr, "%lu", &sNum) != 1) {
         ::report("doTransform(): invalid SEQNUM " + string(snStr),
                     DgBase::Fatal);
      }

      loc = static_cast<const DgIDGGBase&>(*op.inOp.pInRF).bndRF().locFromSeqNum(sNum);
      remainder = &(buff[strlen(snStr) + 1]);
   } else {
      loc = new DgLocation(*op.inOp.pInRF);
      remainder = loc->fromString(buff, op.inOp.inputDelimiter);
   }

   // skip any whitespace
   while (remainder && isspace(*remainder)) remainder++;

   // remainder of line is a single data field
   // (this should really parse the line into multiple fields)
   DgDataList* data = nullptr;
   if (remainder && strlen(remainder) > 0) {
      data = new DgDataList();
      string remStr(remainder);
      DgDataFieldString* field = new DgDataFieldString("data", remStr);
      data->list().push_back(field);
   }

   // build the cell
   DgCell* cell = new DgCell();
   cell->setNode(*loc);
   cell->setDataList(data);

   // cleanup
   delete loc;

   return cell;

} // DgCell* SubOpBasicMulti::inStrToPointCell
*/

////////////////////////////////////////////////////////////////////////////////
string
SubOpBasicMulti::dataToOutStr (DgDataList* data) {

   string str;
   if (data) {
      ostringstream os;
      for (auto const& field : data->list())
         os << op.outOp.outputDelimiter << field->valString();
      str = os.str();
   }

   return str;

} // string SubOpBasicMulti::dataToOutStr

////////////////////////////////////////////////////////////////////////////////
// override the definition in DgApSubOperation to loop over multiple grids
int
SubOpBasicMulti::execute (bool force) {

   if (!active) return 0;

   // don't do if already done
   if (state >= EXECUTED && !force) return 1;

   // this part differs from DgApSubOperation

   int result = 0;

   // process grids until lastGrid
   while (true) {

      // the first dgg and all input/output files were already created when the
      // relevant subops were executed by the main operation

      // keep track of the current random seed so it is output with the plist
      if (op.dggOp.placeRandom && op.outOp.ptsRand != 0)
         pList().setParam("randpts_seed",
           dgg::util::to_string(op.outOp.ptsRand->status()));

      // perform the metafile output if needed; all other output files have been
      // created by the outOp

      if (op.dggOp.numGrids > 1 || op.dggOp.placeRandom) {
         ofstream metaOutFile;
         metaOutFile.open(op.outOp.metaOutFileName.c_str());
         metaOutFile.setf(ios::fixed, ios::floatfield);
         metaOutFile.precision(12);
         metaOutFile << pList();
         metaOutFile.close();
      }

      // now do the operation using the current grid
      int newResult = executeOp();
      if (newResult) result = newResult;

      // quit if last grid
      if (op.dggOp.lastGrid) break;

      // create the dgg and output files for the next loop
      op.dggOp.execute(true);
      op.outOp.nOutputFile = 0; // needs to be in SubOpOut
      op.outOp.execute(true);
   }

   // end of differences from DgApSubOperation

   // update my state
   if (!result) {
     state = EXECUTED;
     numExecutions++;
   }

   return result;

} // SubOpBasicMulti::execute

////////////////////////////////////////////////////////////////////////////////
