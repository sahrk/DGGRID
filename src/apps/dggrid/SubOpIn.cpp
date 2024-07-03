/******************************************************************************
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
// SubOpIn.cpp: SubOpIn class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgLocation.h>
#include <dglib/DgInLocTextFile.h>
#include <dglib/DgInGdalFile.h>

#include "OpBasic.h"
#include "SubOpBasicMulti.h"
#include "SubOpIn.h"

////////////////////////////////////////////////////////////////////////////////
SubOpIn::SubOpIn (OpBasic& op, bool _activate)
   : SubOpBasic (op, _activate),
     inFile (nullptr), pInRF (nullptr),
     inAddType (dgg::addtype::InvalidAddressType), isPointInput (false),
     inSeqNum (false), inputDelimiter (' ')
{
}

////////////////////////////////////////////////////////////////////////////////
int
SubOpIn::initializeOp (void)
{
   vector<string*> choices;

   // input_files <fileName1 fileName2 ... fileNameN>
   pList().insertParam(new DgStringParam("input_files", "vals.txt"));

   // input_file_name <fileName>
   // if user sets this it will override any setting of input_files
   pList().insertParam(new DgStringParam("input_file_name", "valsin.txt"));

/*
   if (op.mainOp.operation == "TRANSFORM_POINTS") {
      // input_address_field_type < FIRST_FIELD | NAMED_FIELD | GEO_POINT >
      // where is the location address in a feature?
      choices.push_back(new string("FIRST_FIELD"));
      choices.push_back(new string("NAMED_FIELD"));
*/
      choices.push_back(new string("GEO_POINT")); // determines the index from the input point geometry
                                               // note this differs from having a geo point in a (text)
                                               // field (that is the intent when fully implemented)
      pList().insertParam(new DgStringChoiceParam("input_address_field_type", "GEO_POINT",
                  &choices));
      dgg::util::release(choices);
/*
      // input_address_field_name <fieldName>
      // used when input_address_field_type is NAMED_FIELD
      pList().insertParam(new DgStringParam("input_address_field_name", "global_id"));
   }
*/
   // point_input_file_type <NONE | TEXT | GDAL>
   choices.push_back(new string("NONE"));
#ifdef USE_GDAL
   choices.push_back(new string("GDAL"));
#endif
   choices.push_back(new string("TEXT"));
   string def = ((op.mainOp.operation == "GENERATE_GRID") ? "NONE" : "TEXT");
   pList().insertParam(new DgStringChoiceParam("point_input_file_type", def, &choices));
   dgg::util::release(choices);

/*
#ifdef USE_GDAL
   // point_input_gdal_format <gdal driver type>
   pList().insertParam(new DgStringParam("point_input_gdal_format", "GeoJSON"));
#endif
*/

   // input_address_type < GEO | PLANE | PROJTRI | Q2DD | Q2DI |
   //        SEQNUM | VERTEX2DD | ZORDER | ZORDER_STRING |
   //        Z3 | Z3_STRING | Z7 | Z7_STRING >
   for (int i = 0; ; i++) {
      if (dgg::addtype::addTypeStrings[i] == "INVALID")
         break;
      choices.push_back(new string(dgg::addtype::addTypeStrings[i]));
   }

   //def = ((op.mainOp.operation != "TRANSFORM_POINTS") ? "GEO" : "SEQNUM");
   def = "GEO";
   pList().insertParam(new DgStringChoiceParam("input_address_type", def, &choices));
   dgg::util::release(choices);

   // input_delimiter <v is any character in long double quotes>
   pList().insertParam(new DgStringParam("input_delimiter", "\" \"", true ,false));

   return 0;

} // int SubOpIn::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpIn::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   string inFileStr;
   getParamValue(pList(), "input_files", inFileStr, false);

   // input file name
   getParamValue(pList(), "input_file_name", inTextFileName, false);

   // check if input_file_name was user set; if it is override input_files
   DgApAssoc* assoc = pList().getParam("input_file_name", false);
   bool isSingleUserSet = assoc && assoc->isUserSet();
   assoc = pList().getParam("input_files", false);
   bool isMultiUserSet = assoc && assoc->isUserSet();
   if (!isMultiUserSet || isSingleUserSet){
      inFileStr = inTextFileName;
   }

   char* names = new char[inFileStr.length() + 1];
   inFileStr.copy(names, string::npos);
   names[inFileStr.length()] = 0;

   char* name = strtok(names, " ");
   while (name != NULL) {
      inputFiles.push_back(string(name));
      name = strtok(NULL, " ");
   }
   delete [] names;

/*
   if (op.mainOp.operation == "TRANSFORM_POINTS") {
      // input address field type
      getParamValue(pList(), "input_address_field_type", addFldType, false);

      // input address field name
      getParamValue(pList(), "input_address_field_name", addFldName, false);
   } else {
*/
      addFldType = "GEO_POINT";
      addFldName = "global_id"; // not currently used
//   }

   // type of point input file
   getParamValue(pList(), "point_input_file_type", pointInputFileType, false);
   isPointInput = (pointInputFileType != "NONE");

/*
#ifdef USE_GDAL
   // input gdal driver
   getParamValue(pList(), "point_input_gdal_format", gdalDriver, false);
#endif
*/

   // input address type
   string dummy;
   getParamValue(pList(), "input_address_type", dummy, false);
   inAddType = dgg::addtype::stringToAddressType(dummy);

   // input delimiter
   getParamValue(pList(), "input_delimiter", dummy, false);
   if (dummy.length() != 3 || dummy.c_str()[0] != '"' ||
          dummy.c_str()[2] != '"') {
      ::report(
      "invalid input_delimiter; must be a single char in double quotes",
      DgBase::Fatal);
   }
   inputDelimiter = dummy.c_str()[1];

   return 0;

} // SubOpIn::setupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpIn::cleanupOp (void) {

   return 0;

} // SubOpIn::cleanupOp

//////////////////////////////////////////////////////////////////////////////
DgInLocStreamFile*
SubOpIn::makeNewInFile (const DgRFBase& rfIn, const string* fileNameIn,
           DgBase::DgReportLevel failLevel)
{
   DgInLocStreamFile* newFile = nullptr;
   if (pointInputFileType == "TEXT") {
     newFile = new DgInLocTextFile(rfIn, fileNameIn, failLevel);
#ifdef USE_GDAL
   } else if (pointInputFileType == "GDAL") {
      newFile = new DgInGdalFile (rfIn, fileNameIn, failLevel);
#endif
   } else {
      ::report("SubOpIn::makeNewInFile(): invalid point input file type " +
          pointInputFileType, DgBase::Fatal);
   }

   return newFile;

} // DgInLocStreamFile* SubOpIn::makeNewInFile

////////////////////////////////////////////////////////////////////////////////
DgLocationData*
SubOpIn::getNextLoc (void) {

   DgLocationData* loc = nullptr;

   // currently only works on point files
   if (!isPointInput)
      return loc;

   // executeOp should have been called
   if (!inFile)
      return loc;

   while (1) {
      if (pointInputFileType == "TEXT") {
         const int maxLine = 2056;
         char buff[maxLine];
         inFile->getline(buff, maxLine);
         if (!inFile->eof()) {  // we have an input line
            loc = op.primarySubOp->inStrToPointLoc(buff);
            break;
         }
      } else if (pointInputFileType == "GDAL") {
         loc = new DgLocationData();
         *inFile >> *loc;

         if (!inFile->eof()) {  // we have an input line
            break;
         } else { // eof so loc is invalid
            delete loc;
            loc = nullptr;
         }
      } else {
         ::report("SubOpIn::getNextLoc(): invalid point input file type " +
                   pointInputFileType, DgBase::Fatal);
      }

      // if we're here we're at EOF
      inFile->close();
      delete inFile;
      inFile = nullptr;

      // try to go to the next file
      fileNum++;
      if (fileNum < inputFiles.size()) {
         // open next file and try to read again
         inTextFileName = inputFiles[fileNum];
         inFile = makeNewInFile(*pInRF, &inTextFileName, DgBase::Fatal);
      } else { // at EOF on last file
        break;
      }
   }

#if DGDEBUG
   if (loc) dgcout << "INPUT: " << *loc << endl;
#endif
   return loc;

} // DgLocationData* SubOpIn::getNextLoc

////////////////////////////////////////////////////////////////////////////////
void
SubOpIn::resetInFile (void) {

   // currently only for point files
   if (!isPointInput) return;

   if (inFile) {
      inFile->close();
      delete inFile;
      inFile = nullptr;
   }

   fileNum = 0; // use first file
   inTextFileName = inputFiles[fileNum];
   inFile = makeNewInFile(*pInRF, &inTextFileName, DgBase::Fatal);

} // void SubOpIn::resetInFile

////////////////////////////////////////////////////////////////////////////////
int
SubOpIn::executeOp (void) {

   // currently only setup for point files
   if (!isPointInput)
      return 0;

   const DgIDGGBase& dgg = op.dggOp.dgg();

   // set-up the input reference frame
    if (inSeqNum) {
        pInRF = &dgg;
    } else if (!op.dggOp.isSuperfund) { // use input address type

      inSeqNum = op.dggOp.addressTypeToRF(inAddType, &pInRF);
      if (!pInRF)
         ::report("SubOpIn::executeOp(): invalid input RF", DgBase::Fatal);
   }

    if (op.dggOp.isApSeq && inSeqNum)
        ::report("SubOpIn::executeOp(): sequence number input not supported for aperture sequence DGGS", DgBase::Fatal);

   // open first file
   resetInFile();

   return 0;

} // SubOpIn::executeOp


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
