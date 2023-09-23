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
// SubOpOut.cpp: SubOpOut class implementation
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
#include "DgHexSF.h"

#include "OpBasic.h"
#include "SubOpOut.h"

using namespace std;
using namespace dgg::addtype;

////////////////////////////////////////////////////////////////////////////////
void
SubOpOut::genRandPts (const DgQ2DICoord& add2D, const string& label)
{
   const DgIDGGBase& dgg = op.dggOp.dgg();
   const DgContCartRF& deg = op.dggOp.deg();
   const DgDiscRF2D& grid = dgg.grid2D();
   const DgRF<DgDVec2D, long double>& ccRF = grid.backFrame();
   int q = add2D.quadNum();

   DgLocation* cp = new DgLocation();
   grid.setPoint(add2D.coord(), *cp);
   ccRF.convert(cp);

   if (op.mainOp.megaVerbose) dgcout << "RP: " << add2D << " " << *cp;

   DgLocVector rpts(dgg.q2ddRF());
   DgDVec2D cpv = *ccRF.getAddress(*cp);

   grid.setPoint(DgIVec2D(0, 0), *cp);
   DgDVec2D cpv00 = *ccRF.getAddress(*cp);

   if (op.mainOp.megaVerbose) dgcout << " " << *cp;

   delete cp;

   if (op.mainOp.megaVerbose)
   {
      DgDVec2D tvec = cpv - cpv00;
      dgcout << " " << tvec << endl;
   }

   DgDVec2D anchor;
   if (op.dggOp.gridTopo == Hexagon) {
      anchor = cpv; // use center
   } else { // Diamond or Triangle happen to be the same
      DgPolygon verts;
      grid.setVertices(add2D.coord(), verts);
      ccRF.convert(verts);
      anchor = *ccRF.getAddress(verts[0]); // use lower left corner
   }
   if (op.mainOp.megaVerbose) dgcout << "anchor: " << anchor << endl;

   for (int i = 0; i < nRandPts; i++)
   {
      // first generate point on (0,0) diamond

      DgDVec2D rp(ptsRand->randInRange(0.0, 1.0),
         ptsRand->randInRange(0.0, 2.0 * DgDmdD4Grid2D::yOff()));

      if (op.mainOp.megaVerbose) dgcout << i << " " << rp;

      rp.setX(rp.x() - (sqrt(3.0) / 3.0) * rp.y()); // shear

      if (op.mainOp.megaVerbose) dgcout << " " << rp;

      DgLocation* nloc = 0;
      if (op.dggOp.gridTopo != Diamond)
      {
         // force into (0, 0) triangle

         if (rp.y() > sqrt(3.0) * rp.x()) rp.rotate(-60.0);

         if (op.dggOp.gridTopo == Triangle)
         {
            rp *= M_SQRT3; // scale correctly

            const DgTriGrid2D& triG0 =
                             dynamic_cast<const DgTriGrid2D&>(grid);

            if (!triG0.isUp(add2D.coord())) rp.rotate(-60.0);

         }
         else if (op.dggOp.gridTopo == Hexagon)
         {
            DgDVec2D origrp(rp);
            bool goodPt = false;

            while (!goodPt)
            {
               rp = origrp * (1.0 / sqrt(3.0)); // scale correctly

               // position in one of the subtris

               int ndx = ptsRand->nextInt() % 6;
               rp.rotate(-60.0 * ndx);

               // adjust for class I

               if (dgg.isClassI()) rp.rotate(30.0);

               // check for possible pentagon non-keeper

               goodPt = true;
               if (add2D.coord() == DgIVec2D(0, 0)) // pentagon
               {
                  DgLocation* tLoc =
                         dgg.q2ddRF().makeLocation(DgQ2DDCoord(q, rp));

                  dgg.vertexRF().convert(tLoc);

                  const DgAddress<DgVertex2DDCoord>& vc =
                                        *dgg.vertexRF().getAddress(*tLoc);

                  if (!vc.address().keep()) goodPt = false;

                  delete tLoc;
               }
            }

            if (op.mainOp.megaVerbose) dgcout << rp << endl;
         }

         rp += anchor;
         if (op.mainOp.megaVerbose) dgcout << " =final rp: " << rp;

         DgLocation* tloc = ccRF.makeLocation(rp);
         dgg.ccFrame().convert(tloc);

         if (op.mainOp.megaVerbose) dgcout << "->" << *tloc;

         nloc = dgg.q2ddRF().makeLocation(
                       DgQ2DDCoord(q, *dgg.ccFrame().getAddress(*tloc)));

         delete tloc;

         if (op.mainOp.megaVerbose) dgcout << "->" << *nloc << endl;

         rpts.push_back(*nloc);
         delete nloc;
      }
   }

   // convert

   if (op.mainOp.megaVerbose) dgcout << "rpts: " << rpts << endl;

   dgg.geoRF().convert(rpts);

   if (op.mainOp.megaVerbose) dgcout << "-> " << rpts << endl;

   deg.convert(rpts);

   if (op.mainOp.megaVerbose) dgcout << "-> " << rpts << endl;

   DgLocList pts;
   for (int i = 0; i < rpts.size(); i++) {
      op.dggOp.nSamplePts++;   // pt # in this grid
      op.dggOp.sampleCount++;  // pt # overall

      DgCell cell(deg, dgg::util::to_string(op.dggOp.sampleCount), rpts[i]);

      if (!randPtsOutType.compare("TEXT")) {
          // pack more info into the cell label
          ostringstream os;
          os << op.dggOp.sampleCount << ", " << op.dggOp.curGrid << ", "
                   << label << ", " << op.dggOp.nSamplePts << ", ";

          cell.setLabel(os.str());
      }

      *randPtsOut << cell;
   }

} // void SubOpOut::genRandPts

////////////////////////////////////////////////////////////////////////////////
void
SubOpOut::outputCellAdd2D (const DgLocation& add2D, const string* labelIn,
               DgDataList* dataList)
{
   const DgIDGGBase& dgg = op.dggOp.dgg();

   // create the output label
   string label;
   if (labelIn)
      label = *labelIn;
   else {
      if (outSeqNum) {
         unsigned long long int sn = op.dggOp.dgg().bndRF().seqNum(add2D);
         label = dgg::util::to_string(sn);
      } else if (useEnumLbl)
         label = dgg::util::to_string(nCellsAccepted);
      else {
         DgLocation tmpLoc(add2D);
         pOutRF->convert(&tmpLoc);
         label = tmpLoc.asString(outputDelimiter);
      }
   }

   // start new files if needed
   if (maxCellsPerFile && nCellsOutputToFile >= maxCellsPerFile)
      execute(true);

   nCellsOutputToFile++;

   DgLocation* tmpLoc = new DgLocation(add2D);

   DgPolygon verts(dgg);
   dgg.setVertices(add2D, verts, op.outOp.nDensify);

   // unwrap the cell east/west if applicable
   DgPolygon* unwrappedVerts = new DgPolygon(verts);
   if (op.mainOp.megaVerbose) dgcout << "before unwrap: " << *unwrappedVerts << endl;
   int wrapped = DgGeoSphRF::lonWrap(*unwrappedVerts, lonWrapMode);
   if (op.mainOp.megaVerbose) dgcout << "unwrapped: " << *unwrappedVerts << endl;

   // wrap the cell center point if the boundary was wrapped
   if (unwrapPts && wrapped) {
      dgg.geoRF().convert(tmpLoc);
      DgGeoCoord g = *(dgg.geoRF().getAddress(*tmpLoc));
      int testWrap = DgGeoSphRF::lonWrap(g, lonWrapMode);
      if (testWrap) {
         delete tmpLoc;
         tmpLoc = dgg.geoRF().makeLocation(g);
      }
   }

   // create the cell to output
   DgCell cell(dgg.geoRF(), label, *tmpLoc, unwrappedVerts, dataList, false);

   // output cell area
//  dgg.geoRF().convert(tmpLoc);
//  long double area =
//        DgGeoCoord::geoPolyArea(verts, *dgg.geoRF().getAddress(*tmpLoc));
//  dgcout << std::setprecision(15);
//  dgcout << label << " " << area << " "
//       << area * dgg.geoRF().earthRadiusKM() * dgg.geoRF().earthRadiusKM()
//       << endl;

   delete tmpLoc;

   if (op.mainOp.megaVerbose)
      dgcout << "accepted " << label << " " << add2D << endl;

   if (dataOut)
      *dataOut << label << op.primarySubOp->dataToOutStr(dataList) << endl;

   const DgQ2DICoord& q2di = *dgg.getAddress(add2D);
   if (cellOut) {
      if (op.mainOp.megaVerbose)
         dgcout << "outputting region: " << cell << endl;

      if (prCellOut) {
         prCellOut->insert(cell.region(), &(cell.label()));
      } else {
         if (outCellAttributes)
            cellOutShp->setCurFields(curFields);

         *cellOut << cell;
      }
   }
	
   if (ptOut) {
      if (op.mainOp.megaVerbose)
         dgcout << "outputting point: " << cell << endl;

      if (outPointAttributes)
         ptOutShp->setCurFields(curFields);

      *ptOut << cell;
   }

   ///// generate random points if applicable ///
   if (doRandPts)
      genRandPts(*dgg.getAddress(add2D), cell.label());

   ///// neighbor/children output files /////
   DgLocVector neighbors;
   DgLocation ctrGeo = cell.node();
   //if (nbrOut || neighborOutType == "GDAL_COLLECTION")
   if (neighborsOutType != "NONE") {

      if (dgg.gridTopo() == Triangle)
         ::report("Neighbors not implemented for Triangle grids", DgBase::Fatal);

      dgg.setNeighbors(add2D, neighbors);
      if (nbrOut)
         nbrOut->insert(add2D, neighbors);

      for (int i = 0; i < neighbors.size(); i++)
         runStats.push(dgg.geoRF().dist(ctrGeo, neighbors[i]));
   }

   //const DgHexIDGG& hexdgg = static_cast<const DgHexIDGG&>(dgg);
   DgResAdd<DgQ2DICoord> q2diR(q2di, dgg.res());
   DgLocVector children;
   if (childrenOutType != "NONE") {

      op.dggOp.dggs().setAllChildren(q2diR, children);

      if (chdOut)
         chdOut->insert(add2D, children);
   }

   if (collectOut) {
      collectOut->insert(dgg, cell,
            (pointOutType == "GDAL_COLLECTION"),
            (cellOutType == "GDAL_COLLECTION"),
            op.dggOp.chdDgg(),
            ((outSeqNum || useEnumLbl) ? NULL : pOutRF),
            ((outSeqNum || useEnumLbl) ? NULL : pChdOutRF),
            ((neighborsOutType == "GDAL_COLLECTION") ? &neighbors : NULL),
            ((childrenOutType == "GDAL_COLLECTION") ? &children : NULL));
   }

} // void SubOpOut::outputCell

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
SubOpOut::SubOpOut (OpBasic& op, bool _activate)
   : SubOpBasic (op, _activate),
     pOutRF (0), pChdOutRF (0),
     outAddType (dgg::addtype::InvalidAddressType),
     outSeqNum (false), outputDelimiter (' '), nDensify (1),
     lonWrapMode (DgGeoSphRF::Wrap), unwrapPts (true),
     doRandPts (true), ptsRand (0), nRandPts (0),
     nCellsTested(0), nCellsAccepted (0),
     dataOut (0), cellOut (0), ptOut (0), collectOut (0), randPtsOut (0),
     cellOutShp (0), ptOutShp (0), prCellOut (0), nbrOut (0), chdOut (0),
     concatPtOut (true), useEnumLbl (false),
     nOutputFile (0), nCellsOutputToFile (0)
{ }

////////////////////////////////////////////////////////////////////////////////
int
SubOpOut::initializeOp (void)
{
   vector<string*> choices;
   string def;

   // output_file_name <fileName>
   pList().insertParam(new DgStringParam("output_file_name", "valsout.txt"));

   // output_file_type <NONE | TEXT >
   choices.push_back(new string("NONE"));
   choices.push_back(new string("TEXT"));

   def = ((op.mainOp.operation == "BIN_POINT_VALS" ||
           op.mainOp.operation == "BIN_POINT_PRESENCE" ||
           op.mainOp.operation == "TRANSFORM_POINTS") ? "TEXT" : "NONE");
   //def = "NONE";
   pList().insertParam(new DgStringChoiceParam("output_file_type", def,
               &choices));
   dgg::util::release(choices);

   // output_address_type < GEO | PLANE | PROJTRI | Q2DD | Q2DI |
   //        SEQNUM | VERTEX2DD | ZORDER | ZORDER_STRING >
   for (int i = 0; ; i++) {
      if (addTypeStrings[i] == "INVALID")
         break;
      choices.push_back(new string(addTypeStrings[i]));
   }
   pList().insertParam(new DgStringChoiceParam("output_address_type",
               "SEQNUM", &choices));
   dgg::util::release(choices);

   // output_delimiter <v is any character in double quotes>
   pList().insertParam(new DgStringParam("output_delimiter", "\" \"", true, false));

   // densification <int> (0 <= v <= 500)
   pList().insertParam(new DgIntParam("densification", 0, 0, 500));

   // longitude_wrap_mode < WRAP | UNWRAP_WEST | UNWRAP_EAST >
   choices.push_back(new string("WRAP"));
   choices.push_back(new string("UNWRAP_WEST"));
   choices.push_back(new string("UNWRAP_EAST"));
   pList().insertParam(new DgStringChoiceParam("longitude_wrap_mode", "WRAP",
               &choices));
   dgg::util::release(choices);

   // unwrap_points <TRUE | FALSE> (true indicates unwrap center point
   //        longitude to match the cell boundary, false allow to wrap)
   pList().insertParam(new DgBoolParam("unwrap_points", true));

   // output_cell_label_type <GLOBAL_SEQUENCE | ENUMERATION | SUPERFUND | OUTPUT_ADDRESS_TYPE >
   choices.push_back(new string("GLOBAL_SEQUENCE"));
   choices.push_back(new string("ENUMERATION"));
   choices.push_back(new string("SUPERFUND"));
   choices.push_back(new string("OUTPUT_ADDRESS_TYPE"));

   string outLblDef("GLOBAL_SEQUENCE");
   if (op.mainOp.operation == "TRANSFORM_POINTS")
      outLblDef = "OUTPUT_ADDRESS_TYPE";
   pList().insertParam(new DgStringChoiceParam("output_cell_label_type", outLblDef,
               &choices));
   dgg::util::release(choices);

   ////// output parameters //////

   // cell_output_type <NONE | AIGEN | GDAL | KML | GEOJSON | SHAPEFILE | GDAL_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("AIGEN"));
#ifdef USE_GDAL
   choices.push_back(new string("GDAL"));
#endif
   choices.push_back(new string("KML"));
   choices.push_back(new string("GEOJSON"));
   choices.push_back(new string("SHAPEFILE"));
   choices.push_back(new string("GDAL_COLLECTION"));
   //choices.push_back(new string("TEXT"));
   def = ((op.mainOp.operation == "GENERATE_GRID") ? "AIGEN" : "NONE");
   pList().insertParam(new DgStringChoiceParam("cell_output_type", def, &choices));
   dgg::util::release(choices);

   // point_output_type <NONE | AIGEN | GDAL | KML | GEOJSON | SHAPEFILE | TEXT | GDAL_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("AIGEN"));
#ifdef USE_GDAL
   choices.push_back(new string("GDAL"));
#endif
   choices.push_back(new string("KML"));
   choices.push_back(new string("GEOJSON"));
   choices.push_back(new string("SHAPEFILE"));
   choices.push_back(new string("TEXT"));
   choices.push_back(new string("GDAL_COLLECTION"));
   pList().insertParam(new DgStringChoiceParam("point_output_type", "NONE", &choices));
   dgg::util::release(choices);

   // randpts_output_type <NONE | AIGEN | GDAL | KML | GEOJSON | SHAPEFILE | TEXT>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("AIGEN"));
#ifdef USE_GDAL
   choices.push_back(new string("GDAL"));
#endif
   choices.push_back(new string("KML"));
   choices.push_back(new string("SHAPEFILE"));
   choices.push_back(new string("GEOJSON"));
   choices.push_back(new string("TEXT"));
   pList().insertParam(new DgStringChoiceParam("randpts_output_type", "NONE", &choices));
   dgg::util::release(choices);

#ifdef USE_GDAL
   // cell_output_gdal_format <gdal driver type>
   pList().insertParam(new DgStringParam("cell_output_gdal_format", "GeoJSON"));

   // point_output_gdal_format <gdal driver type>
   pList().insertParam(new DgStringParam("point_output_gdal_format", "GeoJSON"));

   // collection_output_gdal_format <gdal driver type>
   pList().insertParam(new DgStringParam("collection_output_gdal_format", "GeoJSON"));

#endif

   // cell_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("cell_output_file_name", "cells"));

   // point_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("point_output_file_name", "centers"));

   // randpts_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("randpts_output_file_name", "randPts"));

   // collection_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("collection_output_file_name", "cells"));

   // dggs_orient_output_file_name <whatever.meta>
   pList().insertParam(new DgStringParam("dggs_orient_output_file_name",
                     "grid.meta"));

   // shapefile_id_field_length <int> (1 <= v <= 50)
   pList().insertParam(new DgIntParam("shapefile_id_field_length", 11, 1, 50));

   // kml_default_width <int> (1 <= v <= 100)
   pList().insertParam(new DgIntParam("kml_default_width",
               DgOutLocFile::defaultKMLWidth, 1, 100));

   // kml_default_color <string> <hexadecmial color>
   pList().insertParam(new DgStringParam("kml_default_color",
               DgOutLocFile::defaultKMLColor));

   // kml_name <string> <string name>
   pList().insertParam(new DgStringParam("kml_name",
               DgOutLocFile::defaultKMLName));

   // kml_description <string> <string description>
   pList().insertParam(new DgStringParam("kml_description",
               DgOutLocFile::defaultKMLDescription));

   ///// PlanetRisk output formats /////

   // neighbor_output_type <NONE | TEXT | GEOJSON_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("TEXT"));
   choices.push_back(new string("GDAL_COLLECTION"));
   pList().insertParam(new DgStringChoiceParam("neighbor_output_type", "NONE",
               &choices));
   dgg::util::release(choices);

   // neighbor_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("neighbor_output_file_name", "nbr"));

   // children_output_type <NONE | TEXT | GDAL_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("TEXT"));
   choices.push_back(new string("GDAL_COLLECTION"));
   pList().insertParam(new DgStringChoiceParam("children_output_type", "NONE",
               &choices));
   dgg::util::release(choices);

   // children_output_file_name <outputFileName>
   pList().insertParam(new DgStringParam("children_output_file_name", "chld"));

   ///// additional random points parameters /////

   // randpts_concatenate_output <TRUE | FALSE>
   pList().insertParam(new DgBoolParam("randpts_concatenate_output", true));

   // randpts_num_per_cell <int> (v >= 0)
   pList().insertParam(new DgIntParam("randpts_num_per_cell", 0, 0, INT_MAX));

   // randpts_seed <unsigned long int>
   pList().insertParam(new DgULIntParam("randpts_seed", 77316727, 0, ULONG_MAX, true));

   //  max_cells_per_output_file <int> (v >= 0)
   pList().insertParam(new DgULIntParam("max_cells_per_output_file", 0, 0, ULONG_MAX, true));

   //  output_first_seqnum <int> (v >= 0)
   pList().insertParam(new DgULIntParam("output_first_seqnum", 1, 0, ULONG_MAX, true));

   //  output_last_seqnum <int> (v >= 0)
   pList().insertParam(new DgULIntParam("output_last_seqnum", ULONG_MAX, 0, ULONG_MAX, true));

   return 0;

} // int SubOpOut::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpOut::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   string dummy;

   // output address type
   getParamValue(pList(), "output_address_type", dummy, false);
   outAddType = dgg::addtype::stringToAddressType(dummy);

   // output delimiter
   getParamValue(pList(), "output_delimiter", dummy, false);
   if (dummy.length() != 3 || dummy.c_str()[0] != '"' ||
       dummy.c_str()[2] != '"') {
      ::report(
       "invalid output_delimiter; must be a single char in double quotes",
       DgBase::Fatal);
   }
   outputDelimiter = dummy.c_str()[1];

   getParamValue(pList(), "densification", nDensify, false);

   getParamValue(pList(), "longitude_wrap_mode", dummy, false);
   lonWrapMode = DgGeoSphRF::Wrap;
   if (dummy == "WRAP") {
      lonWrapMode = DgGeoSphRF::Wrap;
   } else if (dummy == "UNWRAP_WEST") {
      lonWrapMode = DgGeoSphRF::UnwrapWest;
   } else if (dummy == "UNWRAP_EAST") {
      lonWrapMode = DgGeoSphRF::UnwrapEast;
   } else
      ::report("Unrecognised value for 'longitude_wrap_mode'", DgBase::Fatal);

   getParamValue(pList(), "unwrap_points", unwrapPts, false);

   ///// output files

   getParamValue(pList(), "output_file_type", dataOutType, "NONE");
   getParamValue(pList(), "cell_output_type", cellOutType, "NONE");
#ifdef USE_GDAL
   getParamValue(pList(), "cell_output_gdal_format", gdalCellDriver, "NONE");
   getParamValue(pList(), "collection_output_gdal_format", gdalCollectDriver, "NONE");
#endif
   getParamValue(pList(), "point_output_type", pointOutType, "NONE");
#ifdef USE_GDAL
   getParamValue(pList(), "point_output_gdal_format", gdalPointDriver, "NONE");
#endif
   getParamValue(pList(), "randpts_output_type", randPtsOutType, "NONE");
   getParamValue(pList(), "neighbor_output_type", neighborsOutType, "NONE");
   getParamValue(pList(), "children_output_type", childrenOutType, "NONE");

   getParamValue(pList(), "neighbor_output_file_name", neighborsOutFileNameBase,
                   false);
   getParamValue(pList(), "children_output_file_name", childrenOutFileNameBase,
                   false);

   getParamValue(pList(), "output_file_name", dataOutFileNameBase, false);
   getParamValue(pList(), "cell_output_file_name", cellOutFileNameBase,
                    false);
   getParamValue(pList(), "point_output_file_name", ptOutFileNameBase,
                    false);
   getParamValue(pList(), "randpts_output_file_name", randPtsOutFileNameBase,
                    false);
   getParamValue(pList(), "collection_output_file_name", collectOutFileNameBase,
                    false);
   getParamValue(pList(), "dggs_orient_output_file_name", metaOutFileNameBase,
                 false);
   getParamValue(pList(), "shapefile_id_field_length", shapefileIdLen,
                    false);
   getParamValue(pList(), "kml_default_color", kmlColor, false);
   getParamValue(pList(), "kml_default_width", kmlWidth, false);
   getParamValue(pList(), "kml_name", kmlName, false);
   getParamValue(pList(), "kml_description", kmlDescription, false);

   getParamValue(pList(), "update_frequency", op.mainOp.updateFreq, false);
   getParamValue(pList(), "max_cells_per_output_file", maxCellsPerFile, false);
   getParamValue(pList(), "output_first_seqnum", outFirstSeqNum, false);
   getParamValue(pList(), "output_last_seqnum", outLastSeqNum, false);

   string outLblType;
   getParamValue(pList(), "output_cell_label_type", outLblType, false);

   if (outLblType == "SUPERFUND" && !op.dggOp.isSuperfund)
      report("output_cell_label_type of SUPERFUND"
             " requires dggs_type of SUPERFUND", DgBase::Fatal);

   if (outLblType != "SUPERFUND" && op.dggOp.isSuperfund)
      report("dggs_type of SUPERFUND requires "
                "output_cell_label_type of SUPERFUND", DgBase::Fatal);


   useEnumLbl = false;
   outSeqNum = false;
   if (outLblType == "GLOBAL_SEQUENCE")
      outSeqNum = true;
   if (outLblType == "ENUMERATION")
      useEnumLbl = true;

   doRandPts = false;
   if (randPtsOutType != "NONE") {
      doRandPts = true;
      getParamValue(pList(), "randpts_num_per_cell", nRandPts, false);
      if (nRandPts < 1)
         doRandPts = false;
      else {
         unsigned long int ranSeed = 0;
         getParamValue(pList(), "randpts_seed", ranSeed, false);
         if (op.mainOp.useMother) ptsRand = new DgRandMother(ranSeed);
         else ptsRand = new DgRand(ranSeed);
         getParamValue(pList(), "randpts_concatenate_output", concatPtOut, false);
         //getParamValue(pList(), "clip_randpts", clipRandPts, false);
      }
   }

   snprintf(formatStr, DgRFBase::maxFmtStr, ", %%#.%dLF, %%#.%dLF\n",
            op.mainOp.precision, op.mainOp.precision);

   ///// attribute files

   buildShapeFileAttributes = false;
   outCellAttributes = false;
   outPointAttributes = false;

/*** shapefile attribute stuff
   getParamValue(pList(), "build_shapefile_attributes", buildShapeFileAttributes, false);
///// alternate stuff
>       getParamValue(pList(), "build_shapefile_attributes_source", dummy, false);
>       if (dummy == string("CLIP_FILES"))
>       {
>          buildShapeFileAttributes = true;
>          buildClipFileAttributes = true;
>       }
>       else if (dummy == string("SHAPEFILES"))
>       {
>          buildShapeFileAttributes = true;
>          buildClipFileAttributes = false;
>       }


   if (buildShapeFileAttributes)
   {
      if (!clipShape)
         report("build_shapefile_attributes "
                "requires clip_subset_type of SHAPEFILE", DgBase::Fatal);

      if (cellOutType == "SHAPEFILE")
         outCellAttributes = true;

      if (pointOutType == "SHAPEFILE")
         outPointAttributes = true;

      if (!outCellAttributes && !outPointAttributes)
         report("using build_shapefile_attributes requires SHAPEFILE "
                "cell or point output.", DgBase::Fatal);
   }

   getParamValue(pList(), "shapefile_attribute_default_int",
                  shapefileDefaultInt, false);
   getParamValue(pList(), "shapefile_attribute_default_double",
                  shapefileDefaultDouble, false);
   getParamValue(pList(), "shapefile_attribute_default_string",
                  shapefileDefaultString, false);

>       if (buildShapeFileAttributes && !buildClipFileAttributes)
>       {
>          getParamValue(pList(), "attribute_files", regFileStr, false);
>          util::ssplit(regFileStr, attributeFiles);
>       }

***/
   return 0;

} // SubOpOut::setupOp

////////////////////////////////////////////////////////////////////////////////
void
SubOpOut::resetFiles (void) {

   // reset the file names
   dataOutFileName = dataOutFileNameBase;
   cellOutFileName = cellOutFileNameBase;
   ptOutFileName = ptOutFileNameBase;
   collectOutFileName = collectOutFileNameBase;
   randPtsOutFileName = randPtsOutFileNameBase;
   metaOutFileName = metaOutFileNameBase;
   neighborsOutFileName = neighborsOutFileNameBase;
   childrenOutFileName = childrenOutFileNameBase;

   // Flush and close any input files we may have used:
   delete dataOut; dataOut = NULL;
   delete cellOut; cellOut = NULL;
   delete ptOut; ptOut = NULL;
   delete collectOut; collectOut = NULL;
   delete randPtsOut; randPtsOut = NULL;
   delete prCellOut; prCellOut = NULL;
   delete nbrOut; nbrOut = NULL;
   delete chdOut; chdOut = NULL;

   cellOutShp = NULL; // this is a ptr to cellOut so don't delete
   ptOutShp = NULL; // this is a ptr to ptOut so don't delete

} // SubOpOut::cleanupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpOut::cleanupOp (void) {

   delete ptsRand;

   resetFiles();

   nOutputFile = 0;
   nCellsOutputToFile = 0;
   // reset other fields to 0?

   return 0;

} // SubOpOut::cleanupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpOut::executeOp (void) {

   const DgIDGGBase& dgg = op.dggOp.dgg();

   // starting a new set of outputs
   nOutputFile++;
   nCellsOutputToFile = 0;

   // delete all the old output files
   resetFiles();

   // set-up the output reference frame
   if (outSeqNum || useEnumLbl)
      pOutRF = &dgg;
   else if (!op.dggOp.isSuperfund) { // use input address type

      outSeqNum = op.dggOp.addressTypeToRF(outAddType, &pOutRF, &pChdOutRF);
      if (!pOutRF)
         ::report("SubOpOut::executeOp(): invalid output RF", DgBase::Fatal);
   }

   string suffix = string("");
   if (op.dggOp.numGrids > 1) {
      suffix += string(".") + dgg::util::to_string(op.dggOp.curGrid, 4);
      metaOutFileName += suffix;
   }

   if (maxCellsPerFile)
      suffix += string("_") + dgg::util::to_string(nOutputFile);

   if (!suffix.empty()) {
      dataOutFileName += suffix;
      cellOutFileName += suffix;
      ptOutFileName += suffix;
      collectOutFileName += suffix;
      randPtsOutFileName += suffix;
      neighborsOutFileName += suffix;
      childrenOutFileName += suffix;

      if (!concatPtOut)
         randPtsOutFileName += suffix;
   }

   // are we using a collection file?
   bool makeCollectFile = false;
   if (cellOutType == "GDAL_COLLECTION" || pointOutType == "GDAL_COLLECTION")
      makeCollectFile = true;

   if (!makeCollectFile && ((childrenOutType == "GDAL_COLLECTION") ||
          (neighborsOutType == "GDAL_COLLECTION")))
      ::report("GDAL_COLLECTION must include cell and/or point data",
                DgBase::Fatal);

   // now create all the files

   if (dataOutType == "TEXT") {
      dataOut = new ofstream();
      dataOut->open(dataOutFileName.c_str());
      dataOut->setf(ios::fixed, ios::floatfield);
      dataOut->precision(op.mainOp.precision);
   }

   if (makeCollectFile) {
      collectOut = DgOutLocFile::makeOutLocFile("GDAL_COLLECTION",
             collectOutFileName, gdalCollectDriver, op.dggOp.deg(), false,
             op.mainOp.precision, DgOutLocFile::Collection, shapefileIdLen,
             kmlColor, kmlWidth, kmlName, kmlDescription);
   }

   if (cellOutType == "TEXT") {
      prCellOut = new DgOutPRCellsFile(op.dggOp.deg(), cellOutFileName, op.mainOp.precision);
   } else if (cellOutType != "GDAL_COLLECTION") {
      cellOut = DgOutLocFile::makeOutLocFile(cellOutType, cellOutFileName,
                   gdalCellDriver, op.dggOp.deg(), false, op.mainOp.precision,
                   DgOutLocFile::Polygon, shapefileIdLen,
                   kmlColor, kmlWidth, kmlName, kmlDescription);

      if (outCellAttributes) {
         cellOutShp = static_cast<DgOutShapefile*>(cellOut);
         cellOutShp->setDefIntAttribute(shapefileDefaultInt);
         cellOutShp->setDefDblAttribute(shapefileDefaultDouble);
         cellOutShp->setDefStrAttribute(shapefileDefaultString);
      }
   }

   if (pointOutType != "GDAL_COLLECTION") {
      ptOut = DgOutLocFile::makeOutLocFile(pointOutType,
           ptOutFileName, gdalPointDriver, op.dggOp.deg(), true, op.mainOp.precision,
           DgOutLocFile::Point, shapefileIdLen,
           kmlColor, kmlWidth, kmlName, kmlDescription);

      if (outPointAttributes) {
         ptOutShp = static_cast<DgOutShapefile*>(ptOut);
         ptOutShp->setDefIntAttribute(shapefileDefaultInt);
         ptOutShp->setDefDblAttribute(shapefileDefaultDouble);
         ptOutShp->setDefStrAttribute(shapefileDefaultString);
      }

      if (doRandPts) {

         if (op.dggOp.curGrid == 1 || !concatPtOut) {
         if (!randPtsOutType.compare("TEXT"))
            randPtsOut = new DgOutRandPtsText(op.dggOp.deg(), randPtsOutFileName,
                      op.mainOp.precision);
         else
            randPtsOut = DgOutLocFile::makeOutLocFile(randPtsOutType,
                      randPtsOutFileName, gdalPointDriver, op.dggOp.deg(), true, op.mainOp.precision,
                      DgOutLocFile::Point, shapefileIdLen,
                      kmlColor, kmlWidth, kmlName, kmlDescription);
         }
      }
   }

   ///// children/neighbor output files /////
   if (neighborsOutType == "TEXT") {

      if (op.dggOp.gridTopo == Triangle)
         ::report("Neighbors not implemented for Triangle grids", DgBase::Fatal);

      nbrOut = new DgOutNeighborsFile(neighborsOutFileName, dgg,
             ((outSeqNum || useEnumLbl) ? NULL : pOutRF), "nbr");
   }

   if (childrenOutType == "TEXT") {
      chdOut = new DgOutChildrenFile(childrenOutFileName, dgg, op.dggOp.chdDgg(),
               ((outSeqNum || useEnumLbl) ? NULL : pOutRF), pChdOutRF, "chd");
   }

   return 0;

} // SubOpOut::executeOp

////////////////////////////////////////////////////////////////////////////////
