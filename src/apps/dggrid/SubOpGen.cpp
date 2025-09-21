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
// SubOpGen.cpp: SubOpGen class implementation
//
////////////////////////////////////////////////////////////////////////////////

// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL
#include <gdal.h>
#endif

#include <iostream>
#include <set>
#include <cstdlib>

#include "clipper.hpp"
#include <dglib/DgIVec2D.h>
#include <dglib/DgInputStream.h>
#include <dglib/DgInAIGenFile.h>
#include <dglib/DgInShapefile.h>
#include <dglib/DgOutShapefile.h>
#include <dglib/DgInShapefileAtt.h>
#include <dglib/DgOutLocFile.h>
#include <dglib/DgOutKMLfile.h>
#include <dglib/DgOutGdalFile.h>
#include <dglib/DgOutPRPtsFile.h>
#include <dglib/DgOutPRCellsFile.h>
#include <dglib/DgOutNeighborsFile.h>
#include <dglib/DgOutChildrenFile.h>
#include <dglib/DgHexIDGG.h>
#include <dglib/DgHexIDGGS.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgBoundedRF2D.h>
#include <dgaplib/DgApParamList.h>
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgGeoProjConverter.h>
#include <dglib/DgRandom.h>
#include <dglib/DgCell.h>
#include <dglib/DgLocList.h>
#include <dglib/DgDmdD4Grid2D.h>
#include <dglib/DgDmdD4Grid2DS.h>
#include <dglib/DgTriGrid2D.h>
#include <dglib/DgOutRandPtsText.h>
/*
#include <dglib/DgZOrderRF.h>
#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgZ3RF.h>
#include <dglib/DgZ3StringRF.h>
*/
#include "DgHexSF.h"

#include "OpBasic.h"
#include "SubOpGen.h"

////////////////////////////////////////////////////////////////////////////////
SubOpGen::SubOpGen (OpBasic& op, bool _activate)
   : SubOpBasicMulti (op, _activate),
     wholeEarth (false), regionClip (false),
     //seqToPoly(false), indexToPoly (false),
     pointClip (false),
     coarseCellClip (false), addressGen (false), useGDAL (false),
     clipAIGen (false), clipGDAL(false), clipShape(false), clipCellRes (0),
     addressFiles(false),
     nClipCellDensify (1), nudge (0.001), doPointInPoly (true),
     doPolyIntersect (false)
{
   // turn-on/off the available sub operations
   op.mainOp.active = true;
   op.dggOp.active = true;
   op.inOp.active = true;
   op.outOp.active = true;
}

////////////////////////////////////////////////////////////////////////////////
int
SubOpGen::initializeOp (void)
{
   std::vector<std::string*> choices;

#ifdef USE_GDAL
   // clip_using_holes <TRUE | FALSE>
   pList().insertParam(new DgBoolParam("clip_using_holes", false));
#endif

   // geodetic_densify <long double: decimal degrees> (v >= 0.0)
   pList().insertParam(new DgDoubleParam("geodetic_densify", 0.0, 0.0, 360.0));

/* v8
   // clip_subset_type <WHOLE_EARTH | AIGEN | SHAPEFILE | GDAL | SEQNUMS | ADDRESSES |
   //                    COARSE_CELLS | INPUT_ADDRESS_TYPE >
*/
   // KEVIN: currently no ADDRESSES or COARSE_CELL_FILES
   // clip_subset_type <WHOLE_EARTH | AIGEN | SHAPEFILE | GDAL |
   //                   ADDRESSES | ADDRESS_FILES | COARSE_CELLS | COARSE_CELL_FILES >
   choices.push_back(new std::string("WHOLE_EARTH"));
   choices.push_back(new std::string("AIGEN"));
   choices.push_back(new std::string("SHAPEFILE"));
#ifdef USE_GDAL
   choices.push_back(new std::string("GDAL"));
#endif
   //choices.push_back(new std::string("ADDRESSES"));
   choices.push_back(new std::string("ADDRESS_FILES"));
   choices.push_back(new std::string("COARSE_CELLS"));
   //choices.push_back(new std::string("COARSE_CELL_FILES"));
   choices.push_back(new std::string("INPUT_ADDRESS_TYPE")); // deprecated
   pList().insertParam(new DgStringChoiceParam("clip_subset_type", "WHOLE_EARTH",
               &choices));
   dgg::util::release(choices);

   // clip_cell_addresses <clipCell1 clipCell2 ... clipCellN>
   pList().insertParam(new DgStringParam("clip_cell_addresses", ""));

   // clip_cell_res <int> (0 < v <= MAX_DGG_RES)
   pList().insertParam(new DgIntParam("clip_cell_res", 1, 1, SubOpDGG::MAX_DGG_RES));

   // clip_cell_densification <int> (0 <= v <= 500)
   pList().insertParam(new DgIntParam("clip_cell_densification", 1, 0, 500));

   // clip_region_files <fileName1 fileName2 ... fileNameN>
   pList().insertParam(new DgStringParam("clip_region_files", "test.gen"));

   // clip_type <POLY_INTERSECT>
   choices.push_back(new std::string("POLY_INTERSECT"));
   pList().insertParam(new DgStringChoiceParam("clip_type", "POLY_INTERSECT",
               &choices));
   dgg::util::release(choices);

   // clipper_scale_factor <unsigned long int>
   pList().insertParam(new DgULIntParam("clipper_scale_factor", 1000000L, 1, ULONG_MAX, true));

   return 0;

} // int SubOpGen::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpGen::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   std::string dummy;
   getParamValue(pList(), "clip_subset_type", dummy, false);
   dummy = dgg::util::toUpper(dummy);

   // handle deprecated value
   if (dummy == "INPUT_ADDRESS_TYPE") {
      dummy = "ADDRESS_FILES";
      ::report(
         "clip_subset_type value INPUT_ADDRESS_TYPE has been renamed ADDRESS_FILES "
         "in version 9.0b. The name INPUT_ADDRESS_TYPE will go away in a future version.", DgBase::Warning);
   }

   wholeEarth = false;
   useGDAL = false;
   clipAIGen = false;
   clipGDAL = false;
   coarseCellClip = false;
   addressGen = false;
   addressFiles = false;
   //clipCellFiles = false;
   //seqToPoly = false;
   //indexToPoly = false;
   if (dummy == "WHOLE_EARTH")
      wholeEarth = true;
   else if (dummy == "AIGEN"){
      regionClip = true;
      clipAIGen  = true;
   } else if (dummy == "SHAPEFILE"){
      regionClip = true;
      clipShape  = true;
// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL
   } else if (dummy == "GDAL") {
      regionClip = true;
      useGDAL = true;
      clipGDAL  = true;
#endif
/*
   } else if (dummy == "SEQNUMS") {
      if (op.dggOp.isApSeq)
         ::report("clip_subset_type of SEQNUMS not supported for dggs_aperture_type of SEQUENCE",
                  DgBase::Fatal);

      seqToPoly = true;
   } else if (dummy == "POINTS") {
      pointClip = true;
*/
//KEVIN: currently no ADDRESSES (on line in metafile)
   } else if (dummy == "ADDRESSES" || dummy == "ADDRESS_FILES") {
      addressGen = true;
      if (dummy == "ADDRESS_FILES")
          addressFiles = true;
//KEVIN: currently no COARSE_CELL_FILES
   } else if (dummy == "COARSE_CELLS" || dummy == "COARSE_CELL_FILES") {
      coarseCellClip = true;
      if (dummy == "COARSE_CELL_FILES")
         addressFiles = true;
   } else
      ::report("Unrecognised value for 'clip_subset_type'", DgBase::Fatal);

   getParamValue(pList(), "clip_cell_res", clipCellRes, false);
   getParamValue(pList(), "clip_cell_densification", nClipCellDensify, false);
   getParamValue(pList(), "clip_cell_addresses", clipCellsStr, false);

   //// region file names

   std::string regFileStr;
   getParamValue(pList(), "clip_region_files", regFileStr, false);

   dgg::util::ssplit(regFileStr, regionFiles);

   getParamValue(pList(), "geodetic_densify", geoDens, false);
   geoDens *= M_PI / 180.0;

   nudge = 0.0000001;
   //getParamValue(pList(), "quad_bndry_nudge", nudge, false);

   getParamValue(pList(), "clip_type", dummy, false);
   dummy = dgg::util::toUpper(dummy);
   if (dummy == "POLY_INTERSECT") {
      doPolyIntersect = true;
      doPointInPoly = false;
   } else {
      doPolyIntersect = false;
      doPointInPoly = true;
   }

#ifdef USE_GDAL
   getParamValue(pList(), "clip_using_holes", useHoles, false);
#endif

   getParamValue(pList(), "clipper_scale_factor", clipperFactor, false);
   invClipperFactor = 1.0L / clipperFactor;

   return 0;

} // SubOpGen::setupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpGen::cleanupOp (void) {

   return 0;

} // SubOpGen::cleanupOp

////////////////////////////////////////////////////////////////////////////////
// executeOp is defined in SubOpGenHelper.cpp

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
