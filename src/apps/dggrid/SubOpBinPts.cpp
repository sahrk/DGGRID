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
// SubOpBinPts.cpp: SubOpBinPts class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgCell.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgInputStream.h>
#include <dglib/DgOutputStream.h>
#include <dglib/DgDataField.h>
#include <dglib/DgInLocTextFile.h>

#include "OpBasic.h"
#include "SubOpBinPts.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// values for a single cell

struct Val {

   int nVals;
   double total;
   double mean;
   bool* presVec;
};

////////////////////////////////////////////////////////////////////////////////
SubOpBinPts::SubOpBinPts (OpBasic& _op, bool _activate)
   : SubOpBasicMulti (_op, _activate)
{
   // turn-on/off the available sub operations
   op.mainOp.active = true;
   op.dggOp.active = true;
   op.inOp.active = true;
   op.outOp.active = true;

   op.inOp.isPointInput = true;

   // set-up I/O defaults based on the operation
   // GENERATE_GRID_FROM_POINTS uses the defaults
   useValInput = false;
   outputMeanDefault = false;
   outputPresVecDefault = false;
   if (op.mainOp.operation == "BIN_POINT_VALS") {
      useValInput = true;
      outputMeanDefault = true;
   } else if (op.mainOp.operation == "BIN_POINT_PRESENCE") {
      outputPresVecDefault = true;
   }

   // these may get set later as parameters
   outputMean = outputMeanDefault;
   outputPresVec = outputPresVecDefault;
   outputTotal = false;
   outputCount = false;
   outputNumClasses = false;

} // SubOpBinPts::SubOpBinPts

////////////////////////////////////////////////////////////////////////////////
int
SubOpBinPts::presVecToString (const bool* presVec, int allClasses,
                  string& vecStr) const
{
   int numClasses = 0;
   vecStr = "";
   if (presVec) {
      for (int i = 0; i < allClasses; i++) {
         vecStr += ((presVec[i]) ? "1" : "0");
         if (presVec[i]) numClasses++;
      }
   }

   return numClasses;

} // int SubOpBinPts::presVecToString

////////////////////////////////////////////////////////////////////////////////
void
SubOpBinPts::outputCell (unsigned long int sNum, const Val& val) const {

      DgLocation* loc = op.dggOp.dgg().bndRF().locFromSeqNum(sNum);
      outputCell(*loc, val);
      delete loc;
}

////////////////////////////////////////////////////////////////////////////////
void
SubOpBinPts::outputCell(const DgLocation& loc, const Val& val) const
{
      // create the data fields
      DgDataList* data = new DgDataList();
      if (outputCount) {
         DgDataFieldInt* fld =
              new DgDataFieldInt(outputCountFldName, val.nVals);
         data->addField(fld);
      }

      if (outputTotal) {
         DgDataFieldDouble* fld =
              new DgDataFieldDouble(outputTotalFldName, val.total);
         data->addField(fld);
      }

      if (outputMean) {
         DgDataFieldDouble* fld =
           new DgDataFieldDouble(outputMeanFldName, val.mean, valFmtStr);
         data->addField(fld);
      }

      if (op.mainOp.operation == "BIN_POINT_PRESENCE") {
         string vecStr;
         int numClasses = presVecToString(val.presVec, op.inOp.inputFiles.size(), vecStr);
         if (outputNumClasses) {
            DgDataFieldInt* fld =
                 new DgDataFieldInt(outputNumClassesFldName, numClasses);
            data->addField(fld);
         }

         if (outputPresVec) {
            DgDataFieldString* fld =
              new DgDataFieldString(outputPresVecFldName, vecStr);
            data->addField(fld);
         }
      }

      op.outOp.outputCellAdd2D(loc, nullptr, data);
}

////////////////////////////////////////////////////////////////////////////////
int
SubOpBinPts::initializeOp (void) {

   vector<string*> choices;

/* not yet used
   // bin_method <ARITHMETIC_MEAN>
   choices.push_back(new string("ARITHMETIC_MEAN"));
   pList().insertParam(new DgStringChoiceParam("bin_method", "ARITHMETIC_MEAN",
                                       &choices));
   dgg::util::release(choices);
*/

   // bin_coverage <GLOBAL | PARTIAL>
   choices.push_back(new string("GLOBAL"));
   choices.push_back(new string("PARTIAL"));
   pList().insertParam(new DgStringChoiceParam("bin_coverage", "GLOBAL", &choices));
   dgg::util::release(choices);

   // output_count <TRUE | FALSE>
   pList().insertParam(new DgBoolParam("output_count", false));

   // output_count_field_name <fieldName>
   pList().insertParam(new DgStringParam("output_count_field_name", "count"));

   // params for reading-in a value field
   if (useValInput) {
      // input_value_field_name
      pList().insertParam(new DgStringParam("input_value_field_name", "value"));

      // output_total <TRUE | FALSE>
      pList().insertParam(new DgBoolParam("output_total", false));

      // output_total_field_name <fieldName>
      pList().insertParam(new DgStringParam("output_total_field_name", "total"));

      // output_mean <TRUE | FALSE>
      pList().insertParam(new DgBoolParam("output_mean", outputMeanDefault));

      // output_mean_field_name <fieldName>
      pList().insertParam(new DgStringParam("output_mean_field_name", "mean"));
   }

   if (op.mainOp.operation == "BIN_POINT_PRESENCE") {
      // output_presence_vector <TRUE | FALSE>
      pList().insertParam(new DgBoolParam("output_presence_vector", outputPresVecDefault));
      pList().insertParam(new DgStringParam("output_presence_vector_field_name", "presVec"));

      // output_num_classes <TRUE | FALSE>
      pList().insertParam(new DgBoolParam("output_num_classes", false));
      pList().insertParam(new DgStringParam("output_num_classes_field_name", "numClass"));
   }


   // cell_output_control <OUTPUT_ALL | OUTPUT_OCCUPIED>
   choices.push_back(new string("OUTPUT_ALL"));
   choices.push_back(new string("OUTPUT_OCCUPIED"));
   pList().insertParam(new DgStringChoiceParam("cell_output_control", "OUTPUT_ALL",
                                       &choices));
   dgg::util::release(choices);

   return 0;

} // int SubOpBinPts::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpBinPts::setupOp (void) {

   /////// fill state variables from the parameter list //////////
   string dummy;

/* not yet used
   getParamValue(plist, "bin_method", dummy, false);
*/

   getParamValue(pList(), "bin_coverage", dummy, false);
   wholeEarth = (dummy == "GLOBAL"); // alternative is PARTIAL

   getParamValue(pList(), "output_count", outputCount, false);
   getParamValue(pList(), "output_count_field_name", outputCountFldName,
                    false);

   if (useValInput) {
      getParamValue(pList(), "input_value_field_name", inValFieldName, false);

      getParamValue(pList(), "output_total", outputTotal, false);
      getParamValue(pList(), "output_total_field_name", outputTotalFldName,
                    false);

      getParamValue(pList(), "output_mean", outputMean, false);
      getParamValue(pList(), "output_mean_field_name", outputMeanFldName,
                    false);
   }

   if (op.mainOp.operation == "BIN_POINT_PRESENCE") {

      getParamValue(pList(), "output_presence_vector", outputPresVec, false);
      getParamValue(pList(), "output_presence_vector_field_name", outputPresVecFldName,
                       false);

      getParamValue(pList(), "output_num_classes", outputNumClasses, false);
      getParamValue(pList(), "output_num_classes_field_name", outputNumClassesFldName,
                       false);
   }

   // cell_output_control
   getParamValue(pList(), "cell_output_control", dummy, false);
   outputAllCells = (dummy == "OUTPUT_ALL");

   // setup the value output format string
   valFmtStr = "%#." + dgg::util::to_string(op.mainOp.precision) + "LF";

   return 0;

} // int SubOpBinPts::setupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpBinPts::cleanupOp (void) {

   return 0;

} // int SubOpBinPts::cleanupOp

////////////////////////////////////////////////////////////////////////////////
DgLocationData*
SubOpBinPts::inStrToPointLoc (const string& inStr) const {

   DgLocationData* loc = SubOpBasicMulti::inStrToPointLoc(inStr);

   // handle an input value
   if (useValInput) {

      // all fields in the DataList are strings; replace with appropriate types
      // first field is the value field, stored as a string in text input
      // other fields are ignored
      const DgDataFieldString* valField =
           dynamic_cast<const DgDataFieldString*>(loc->dataList()->list()[0]);
      const char* valStr = valField->value();
      double val;
      if (!sscanf(valStr, "%lf", &val)) {
         ::report("inStrToPointLoc(): invalid value field in file", DgBase::Fatal);
      }

      loc->dataList()->clearList();
      loc->dataList()->addField(new DgDataFieldDouble(inValFieldName, val));
   }

   return loc;

} // DgLocationData* inStrToPointLoc(const string& inStr) const


////////////////////////////////////////////////////////////////////////////////
double
SubOpBinPts::getVal (DgLocationData& loc) const {

   // get the value from this feature/input line
   const DgDataFieldBase* valField = loc.dataList()->getFieldByName(inValFieldName);
   if (!valField) {
      ::report("binPtsGlobal(): value field '" + inValFieldName + "' not found in file " +
               op.inOp.inTextFileName, DgBase::Fatal);
   }

   double val = 0.0;
   if (valField->toDouble(val)) {
      ::report("binPtsGlobal(): non-numeric field '" + inValFieldName + "' in file " +
               op.inOp.inTextFileName, DgBase::Fatal);
   }

   return val;

} // double SubOpBinPts::getVal

////////////////////////////////////////////////////////////////////////////////
void
SubOpBinPts::binPtsGlobal (void) {

   const DgIDGGBase& dgg = op.dggOp.dgg();

   // create an array to store the values
   Val* vals = new Val[dgg.bndRF().size()];
   for (unsigned long int i = 0; i < dgg.bndRF().size(); i++)
      initVal(vals[i], outputPresVec);

   // now process the points in each input file
   if (useValInput)
      dgcout << "binning point values..." << endl;
   else
      dgcout << "binning points..." << endl;

   while (1) {

      DgLocationData* loc = op.inOp.getNextLoc();
      if (!loc) break; // reached EOF on last input file

      dgg.convert(loc);
      long int sNum = dgg.bndRF().seqNum(*loc);
      long int ndx = sNum - 1;
      vals[ndx].nVals++;

      if (useValInput)
         vals[ndx].total += getVal(*loc);

      delete loc;

      if (outputPresVec)
         vals[ndx].presVec[op.inOp.fileNum] = true;
   }

   ///// calculate the averages /////

   if (useValInput && outputMean) {
      for (unsigned long int i = 0; i < dgg.bndRF().size(); i++) {
         if (vals[i].nVals > 0) vals[i].mean = (vals[i].total / vals[i].nVals);
      }
   }

   ///// output the cells /////
   for (unsigned long int i = 0; i < dgg.bndRF().size(); i++) {

      if (!outputAllCells && vals[i].nVals <= 0) continue;

      unsigned long int sNum = i + 1;
      outputCell(sNum, vals[i]);
   }

   ///// clean-up /////

   if (outputPresVec)
      for (unsigned long int i = 0; i < dgg.bndRF().size(); i++)
         delete [] vals[i].presVec;

   delete [] vals;

} // void binPtsGlobal

////////////////////////////////////////////////////////////////////////////////
class QuadVals {

   public:

      bool isUsed;
      DgIVec2D offset;     // offset of min (i, j)
      DgIVec2D upperRight; // (maxi, maxj) relative to offset
      int numI;
      int numJ;

      Val** vals;

};

////////////////////////////////////////////////////////////////////////////////
void
SubOpBinPts::initVal (Val& val, bool allocPresVec) const {

   val.nVals = 0;
   val.total = 0.0;
   val.mean = 0.0;

   int nClasses = (int) op.inOp.inputFiles.size();
   if (allocPresVec)
      val.presVec = new bool[nClasses];
   else
      val.presVec = nullptr;

} // void SubOpBinPts::initVal

////////////////////////////////////////////////////////////////////////////////
void
SubOpBinPts::binPtsPartial (void)
{
   const DgIDGGBase& dgg = op.dggOp.dgg();

   // create a place to store the values by quad

   QuadVals qvals[12];
   for (int q = 0; q < 12; q++) {
      qvals[q].isUsed = false;
      qvals[q].offset = DgIVec2D(dgg.maxI() + 1, dgg.maxJ() + 1);
      qvals[q].upperRight = DgIVec2D(-1, -1);
      qvals[q].numI = 0;
      qvals[q].numJ = 0;
      qvals[q].vals = 0;
   }

   // now make a first pass through the input files and determine what
   // cells are represented
   dgcout << "determing quad bounds..." << endl;
   while (1) {

      DgLocationData* loc = op.inOp.getNextLoc();
      if (!loc) break; // reached EOF on last input file

      dgg.convert(loc);

      int q = dgg.getAddress(*loc)->quadNum();
      const DgIVec2D& coord = dgg.getAddress(*loc)->coord();
      QuadVals& qv = qvals[q];

      qv.isUsed = true;
      if (coord.i() < qv.offset.i()) qv.offset.setI(coord.i());
      if (coord.i() > qv.upperRight.i()) qv.upperRight.setI(coord.i());
      if (coord.j() < qv.offset.j()) qv.offset.setJ(coord.j());
      if (coord.j() > qv.upperRight.j()) qv.upperRight.setJ(coord.j());

      delete loc;
   }

   // now initialize the vals storage in the quads which are used

   for (int q = 0; q < 12; q++) {

      QuadVals& qv = qvals[q];
      if (!qv.isUsed) continue;

      qv.upperRight = qv.upperRight - qv.offset; // make relative

      qv.numI = (int) qv.upperRight.i() + 1;
      qv.numJ = (int) qv.upperRight.j() + 1;
      qv.vals = new Val*[qv.numI];
      for (int i = 0; i < qv.numI; i++) {
         qv.vals[i] = new Val[qv.numJ];
         for (int j = 0; j < qv.numJ; j++)
            initVal(qv.vals[i][j], outputPresVec);
      }
   }

   // now process the points in each input file
   if (useValInput)
      dgcout << "binning point values..." << endl;
   else
      dgcout << "binning points..." << endl;

   op.inOp.resetInFile(); // start again with first input file
   while (1) {

      DgLocationData* loc = op.inOp.getNextLoc();
      if (!loc) break; // reached EOF on last input file
//cout << *loc << endl;

      dgg.convert(loc);
      int q = dgg.getAddress(*loc)->quadNum();
      QuadVals& qv = qvals[q];
      DgIVec2D coord = dgg.getAddress(*loc)->coord() - qv.offset;

      qv.vals[coord.i()][coord.j()].nVals++;

      if (useValInput)
         qv.vals[coord.i()][coord.j()].total += getVal(*loc);

      delete loc;

      if (outputPresVec)
         qv.vals[coord.i()][coord.j()].presVec[op.inOp.fileNum] = true;
   }

   ///// calculate the averages /////

   if (useValInput && outputMean) {
      for (int q = 0; q < 12; q++) {

         QuadVals& qv = qvals[q];
         if (!qv.isUsed) continue;

         for (int i = 0; i < qv.numI; i++) {
            for (int j = 0; j < qv.numJ; j++) {
               if (qv.vals[i][j].nVals > 0)
                  qv.vals[i][j].mean = (qv.vals[i][j].total / qv.vals[i][j].nVals);
            }
         }
      }
   }

   ///// output the values /////

   if (outputAllCells) {
      for (unsigned long int i = 0; i < dgg.bndRF().size(); i++) {
         unsigned long int sNum = i + 1;
         DgLocation* loc = dgg.bndRF().locFromSeqNum(sNum);

         // check to see if there is a value for this cell
         int q = dgg.getAddress(*loc)->quadNum();
         QuadVals& qv = qvals[q];
         Val outVal;
         initVal(outVal, false); // don't allocate presVec
         if (qv.isUsed) {
            DgIVec2D coord = dgg.getAddress(*loc)->coord() - qv.offset;
            if (coord.i() >= 0 && coord.j() >= 0 &&
                   coord.i() <= qv.upperRight.i() &&
                   coord.j() <= qv.upperRight.j()) {
                outVal = qv.vals[coord.i()][coord.j()]; // shallow copy
            }
         }

         // output the value
         outputCell(*loc, outVal);

         delete loc;
      }
   } else {
      for (int q = 0; q < 12; q++) {
         QuadVals& qv = qvals[q];
         if (!qv.isUsed) continue;

         for (int i = 0; i < qv.numI; i++) {
            for (int j = 0; j < qv.numJ; j++) {
               Val outVal;
               initVal(outVal, false); // don't allocate presVec
               outVal = qv.vals[i][j]; // shallow copy
               if (outVal.nVals == 0) continue;

               DgIVec2D coord(qv.offset.i() + i, qv.offset.j() + j);

               DgLocation* loc = dgg.makeLocation(DgQ2DICoord(q, coord));
               outputCell(*loc, outVal);

               delete loc;
            }
         }
      }
   }

   ///// clean-up /////

   for (int q = 0; q < 12; q++) {
      QuadVals& qv = qvals[q];
      if (!qv.isUsed) continue;

      for (int i = 0; i < qv.numI; i++) {

         if (outputPresVec)
            for (int j = 0; j < qv.numJ; j++)
               delete [] qv.vals[i][j].presVec;

         delete [] qv.vals[i];
      }

      delete [] qv.vals;
   }

} // void SubOpBinPts::binPtsPartial

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// values for a single cell

class ValStat {

   public:

      ValStat (void)
       : nVals (0), val(0.0), min(DBL_MAX), max(DBL_MIN), leaf(false),
         covered(true) { }

      int nVals;
      double val;
      double min;
      double max;
      bool leaf; // a "leaf" node
      bool covered; // a "covered" node
};

////////////////////////////////////////////////////////////////////////////////
int
SubOpBinPts::executeOp (void)
{
   if (op.inOp.inAddType != dgg::addtype::Geo) {
      ::report("SubOpBinPts::executeOp() input address type must be GEO.",
               DgBase::Fatal);
   }

   char tmpStr[DgRFBase::maxFmtStr];
   snprintf(tmpStr, DgRFBase::maxFmtStr, "%%lf%c%%lf%c%%lf",
             op.inOp.inputDelimiter, op.inOp.inputDelimiter);
   op.inOp.inFormatStr = tmpStr;

   const DgIDGGBase& dgg = op.dggOp.dgg();
   dgcout << "Res " << dgg.outputRes() << " " << dgg.gridStats() << endl;

   if (wholeEarth) binPtsGlobal();
   else binPtsPartial();

   int numFiles = op.inOp.fileNum;
   dgcout << "\nprocessed " << numFiles << " input file"
          << ((numFiles > 1) ? "s." : ".") << endl;
   dgcout << "** binning complete **" << endl;

   return 0;

} // int SubOpBinPts::executeOp

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

