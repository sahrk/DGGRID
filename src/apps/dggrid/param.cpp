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
// param.cpp: param class implementation
//
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include "dggrid.h"
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgSuperfund.h>
#include <dglib/DgOutKMLfile.h>
#include <dglib/DgZOrderRF.h>
#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgZ3RF.h>
#include <dglib/DgZ3StringRF.h>
#include <dglib/DgAddressType.h>

using namespace dgg::topo;
using namespace dgg::addtype;

// choose a reference frame based on address type
void
MainParam::addressTypeToRF (MainParam& dp, const DgIDGGBase& dgg, bool isInput)
{
   const DgIDGGSBase& dggs = *(dgg.dggs());
   dp.chdDgg = &dggs.idggBase(dgg.res() + 1);

   bool seqNum = false;
   const DgRFBase* rf = NULL;
   dgg::addtype::DgAddressType type = ((isInput) ? dp.inAddType : dp.outAddType);
   switch (type) {
      case Geo:
         rf = &dgg.geoRF();
         dp.pChdOutRF = &dp.chdDgg->geoRF();
         break;

      case Plane:
         rf = &dgg.planeRF();
         dp.pChdOutRF = &dp.chdDgg->planeRF();
         break;

      case ProjTri:
         rf = &dgg.projTriRF();
         dp.pChdOutRF = &dp.chdDgg->projTriRF();
         break;

      case Q2DD:
         rf = &dgg.q2ddRF();
         dp.pChdOutRF = &dp.chdDgg->q2ddRF();
         break;

      case Q2DI:
         rf = &dgg;
         dp.pChdOutRF = dp.chdDgg;
         break;

      case SeqNum:
/*
         if (isInput && dp.isApSeq)
            ::report("input_address_type of SEQNUM not supported for dggs_aperture_type of SEQUENCE",
                  DgBase::Fatal);
*/

         seqNum = true;
         rf = &dgg;
         dp.pChdOutRF = dp.chdDgg;
         break;

      case Vertex2DD:
         rf = &dgg.vertexRF();
         dp.pChdOutRF = &dp.chdDgg->vertexRF();
         break;

      case Z3:
         if (dp.isApSeq)
            ::report("input_address_type of Z3 not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg.z3RF()) {
            rf = dgg.z3RF();
            dp.pChdOutRF = dp.chdDgg->z3RF();
         } else
            ::report("addressTypeToRF(): Z3 only supported for aperture 3 hexagon grids",
                     DgBase::Fatal);

         break;

      case Z3String:
         if (dp.isApSeq)
            ::report("input_address_type of Z3_STRING not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg.z3StrRF()) {
            rf = dgg.z3StrRF();
            dp.pChdOutRF = dp.chdDgg->z3StrRF();
         } else
            ::report("addressTypeToRF(): Z3_STRING only supported for aperture 3 hexagon grids",
                     DgBase::Fatal);

         break;

      case ZOrder:
         if (dp.isApSeq)
            ::report("input_address_type of ZORDER not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg.zorderRF()) {
            rf = dgg.zorderRF();
            dp.pChdOutRF = dp.chdDgg->zorderRF();
         } else
            ::report("addressTypeToRF(): ZORDER only supported for aperture 3 or 4",
                     DgBase::Fatal);

         break;

      case ZOrderString:
         if (dp.isApSeq)
            ::report("input_address_type of ZORDER_STRING not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg.zorderStrRF()) {
            rf = dgg.zorderStrRF();
            dp.pChdOutRF = dp.chdDgg->zorderStrRF();
         } else
            ::report("addressTypeToRF(): ZORDER_STRING only supported for aperture 3 or 4",
                     DgBase::Fatal);

         break;

      case InvalidAddressType:
      default:
         ::report("addressTypeToRF(): invalid address type", DgBase::Fatal);
   }

   if (isInput) {
      dp.inSeqNum = seqNum;
      dp.pInRF = rf;
   } else {
      dp.outSeqNum = seqNum;
      dp.pOutRF = rf;
   }
}

void
MainParam::determineRes (const DgParamList& plist)
{
   int maxRes = (apertureType != "SEQUENCE") ? MAX_DGG_RES : apSeq.lastRes();

   // get the parameters

   string resType;
   getParamValue(plist, "dggs_res_specify_type", resType, false);

   if (resType == string("SPECIFIED")) {
      getParamValue(plist, "dggs_res_spec", res, false);
   } else {
      bool area;
      long double value = 0;
      if (resType == string("CELL_AREA")) {
         area = true;
         getParamValue(plist, "dggs_res_specify_area", value, false);
      } else {
         area = false;
         getParamValue(plist, "dggs_res_specify_intercell_distance", value,
                       false);
      }

      bool roundDown = true;  // round up/down to nearest available cell size
      getParamValue(plist, "dggs_res_specify_rnd_down", roundDown, false);

      // determine the resolution

      DgRFNetwork net0;
      const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(net0, "GS0", earthRadius));
      const DgIDGGSBase *idggs = DgIDGGSBase::makeRF(net0, geoRF, vert0,
             azimuthDegs, aperture, maxRes, gridTopo, gridMetric, "IDGGS",
             projType, isMixed43, numAp4, isSuperfund, isApSeq, apSeq);

      long double last = 0.0;
      res = maxRes + 1;
      for (int i = 1; i <= maxRes; i++) {
         const DgGridStats& gs = idggs->idggBase(i).gridStats();
         long double next = (area) ? gs.cellAreaKM() : gs.cellDistKM();

         if (value == next) {
            res = i;
            break;
         }

         if (value < last && value > next) {
            if (roundDown) res = i;
            else res = i - 1;
            break;
         }

         last = next;
      }

      dgcout << "** choosing grid resolution: " << res << endl;
   }

   if (res > maxRes) {
      ::report("MainParam::determineRes() desired resolution exceeds "
               "maximum resolution for this topology", DgBase::Fatal);
   }
   else if (res < 0) res = 0;

} // int MainParam::determineRes

MainParam::MainParam (DgParamList& plist)
   : operation (""), dggsType (""), gridTopo (dgg::topo::InvalidTopo),
     gridMetric (dgg::topo::InvalidMetric), aperture (4),
     projType ("ISEA"), res (5), actualRes (5),
     placeRandom (false), orientCenter (false), orientRand (0),
     numGrids (1), curGrid (1), lastGrid (false), azimuthDegs (0.0),
     datum (""), precision (DEFAULT_PRECISION), verbosity (0),
     megaVerbose (false), pauseOnStart (false), pauseBeforeExit (false),
     metaOutFileNameBase (""), metaOutFileName (""),
     pInRF (0), inAddType (dgg::addtype::InvalidAddressType), inSeqNum (false),
     inputDelimiter (' '), pOutRF (0), outAddType (dgg::addtype::InvalidAddressType),
     outSeqNum (false), outputDelimiter (' '), apertureType (""),
     isMixed43 (false), isSuperfund (false), isApSeq (false)
{
   /////// fill state variables from the parameter list //////////

   getParamValue(plist, "dggrid_operation", operation, false);

   string dummy;
   getParamValue(plist, "rng_type", dummy, false);

   if (dummy == "MOTHER") useMother = true;
   else useMother = false;

   // setup preset DGGS (if any)
   string tmp;
   getParamValue(plist, "dggs_type", tmp, false);
   string tmplc = toLower(tmp);
   if (tmplc != "custom")
   {
      // these params are common to all presets
      plist.setPresetParam("dggs_base_poly", "ICOSAHEDRON");
      plist.setPresetParam("dggs_orient_specify_type", "SPECIFIED");
      plist.setPresetParam("dggs_num_placements", "1");
      plist.setPresetParam("dggs_vert0_lon", "11.25");
      plist.setPresetParam("dggs_vert0_lat", "58.28252559");
      plist.setPresetParam("dggs_vert0_azimuth", "0.0");
      plist.setPresetParam("dggs_res_specify_type", "SPECIFIED");
      plist.setPresetParam("dggs_res_spec", "9");

      if (tmplc == "superfund")
      {
         plist.setPresetParam("dggs_topology", "HEXAGON");
         plist.setPresetParam("dggs_proj", "FULLER");
         plist.setPresetParam("dggs_num_aperture_4_res", "2");
         plist.setPresetParam("dggs_aperture_type", "MIXED43");
         plist.setPresetParam("output_cell_label_type", "SUPERFUND");
      }
      else if (tmplc == "planetrisk")
      {
         plist.setPresetParam("dggs_topology", "HEXAGON");
         plist.setPresetParam("dggs_proj", "ISEA");
         plist.setPresetParam("dggs_aperture_type", "SEQUENCE");
         plist.setPresetParam("dggs_aperture_sequence", "43334777777777777777777");
         plist.setPresetParam("dggs_res_spec", "11");
      }
      else
      {
         // get the topology
         char topo = tmplc[tmplc.length() - 1];
         switch (topo)
         {
            case 'h':
               plist.setPresetParam("dggs_topology", "HEXAGON");
               break;
            case 't':
               plist.setPresetParam("dggs_topology", "TRIANGLE");
               break;
            case 'd':
               plist.setPresetParam("dggs_topology", "DIAMOND");
               break;
         }

         // get the projection
         int projLen;

         if (!tmplc.compare(0, 4, "isea"))
         {
            plist.setPresetParam("dggs_proj", "ISEA");
            projLen = (int) string("isea").length();
         }
         else // must be FULLER
         {
            plist.setPresetParam("dggs_proj", "FULLER");
            projLen = (int) string("fuller").length();
         }

         // get the aperture
         tmplc = tmplc.substr(projLen, tmplc.length() - projLen - 1);
         if (tmplc == "43")
         {
            plist.setPresetParam("dggs_aperture_type", "MIXED43");
         }
         else
         {
            plist.setPresetParam("dggs_aperture_type", "PURE");
            plist.setPresetParam("dggs_aperture", tmplc);
         }
      }
   }

   string gridTopoStr = "";
   getParamValue(plist, "dggs_topology", gridTopoStr, false);
   gridTopo = dgg::topo::stringToGridTopology(gridTopoStr);

/* metric not exposed to user yet; D8 is broken
   string gridMetricStr = "";
   getParamValue(plist, "dggs_metric", gridMetricStr, false);
   gridMetric = stringToGridMetric(gridMetricStr);
*/
   switch (gridTopo) {
      case Hexagon:
         gridMetric = D6;
         break;
      case Triangle:
         gridMetric = D3;
         break;
      case Diamond:
         gridMetric = D4;
         break;
      default:
         ::report("MainParam::MainParam() invalid dggs_topology" +
              gridTopoStr, DgBase::Fatal);
   }

   getParamValue(plist, "dggs_aperture_type", apertureType, false);
   if (apertureType == "MIXED43")
      isMixed43 = true;
   else if (apertureType == "SEQUENCE")
      isApSeq = true;

   numAp4 = 0;
   if (isMixed43)
      getParamValue(plist, "dggs_num_aperture_4_res", numAp4, false);
   else if (isApSeq)
   {
      string tmp;
      getParamValue(plist, "dggs_aperture_sequence", tmp, false);
      apSeq = DgApSeq(tmp);
   }
   else
   {
      getParamValue(plist, "dggs_aperture", tmp, false);
      aperture = dgg::util::from_string<int>(tmp);
   }

   // simulate a pure aperture 7 grid using an aperture sequence
   if (aperture == 7 && !isApSeq) {
      isApSeq = true;
      apSeq = DgApSeq("77777777777777777777777777777777777");
   }

   getParamValue(plist, "dggs_num_placements", numGrids, false);
   getParamValue(plist, "dggs_proj", projType, false);
   getParamValue(plist, "dggs_vert0_azimuth", azimuthDegs, false);

   long double lon0, lat0;
   getParamValue(plist, "dggs_vert0_lon", lon0, false);
   getParamValue(plist, "dggs_vert0_lat", lat0, false);
   vert0 = DgGeoCoord(lon0, lat0, false);

   getParamValue(plist, "proj_datum", datum, false);
   if (datum == "WGS84_AUTHALIC_SPHERE")
      earthRadius = 6371.007180918475L;
   else if (datum == "WGS84_MEAN_SPHERE")
      earthRadius = 6371.0087714L;
   else // datum must be CUSTOM_SPHERE
      getParamValue(plist, "proj_datum_radius", earthRadius, false);

   if (tmp == "SUPERFUND")
   {
      isSuperfund = true;

      if (!isMixed43)
         ::report("MainParam::MainParam() Superfund grid requires "
             "dggs_aperture_type of MIXED43", DgBase::Fatal);

      if (numAp4 != 2)
         ::report("MainParam::MainParam() Superfund grid requires "
             "dggs_num_aperture_4_res of 2", DgBase::Fatal);

      string resType;
      getParamValue(plist, "dggs_res_specify_type", resType, false);

      if (resType != string("SPECIFIED"))
         ::report("MainParam::MainParam() Superfund grid requires "
             "dggs_res_specify_type of SPECIFIED", DgBase::Fatal);

      getParamValue(plist, "dggs_res_spec", res, false);
      sfRes = res;
      actualRes = res = sfRes2actualRes(sfRes);
   }
   else // not superfund
   {
      sfRes = 0;
      determineRes(plist);
      actualRes = res;
   }

   getParamValue(plist, "precision", precision, false);

   getParamValue(plist, "verbosity", verbosity, false);
   if (verbosity > 2) {
      megaVerbose = true;
      DgConverterBase::setTraceOn(true);
   }

   getParamValue(plist, "pause_on_startup", pauseOnStart, false);
   getParamValue(plist, "pause_before_exit", pauseBeforeExit, false);

   getParamValue(plist, "dggs_orient_output_file_name", metaOutFileNameBase,
                 false);

   // output address type
   getParamValue(plist, "output_address_type", dummy, false);
   outAddType = dgg::addtype::stringToAddressType(dummy);

   // input address type
   getParamValue(plist, "input_address_type", dummy, false);
   inAddType = dgg::addtype::stringToAddressType(dummy);

   // input delimiter
   getParamValue(plist, "input_delimiter", dummy, false);
   if (dummy.length() != 3 || dummy.c_str()[0] != '"' ||
          dummy.c_str()[2] != '"') {
      ::report(
      "invalid input_delimiter; must be a single char in double quotes",
      DgBase::Fatal);
   }
   inputDelimiter = dummy.c_str()[1];

   // output delimiter
   getParamValue(plist, "output_delimiter", dummy, false);
   if (dummy.length() != 3 || dummy.c_str()[0] != '"' ||
       dummy.c_str()[2] != '"') {
      ::report(
       "invalid output_delimiter; must be a single char in double quotes",
       DgBase::Fatal);
   }
   outputDelimiter = dummy.c_str()[1];

   getParamValue(plist, "dggs_orient_specify_type", dummy, false);
   if (dummy == string("SPECIFIED"))
      placeRandom = false;
   else if (dummy == string("REGION_CENTER")) {
      placeRandom = false;
      orientCenter = true;
   } else {
      placeRandom = true;

      unsigned long int ranSeed = 0;
      getParamValue(plist, "dggs_orient_rand_seed", ranSeed, false);
      if (useMother) {
         orientRand = new DgRandMother(ranSeed);
      } else {
         orientRand = new DgRand(ranSeed);
      }
   }

} // MainParam::MainParam

MainParam::~MainParam()
{
   delete orientRand;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void MainParam::dump (void)
{
   dgcout << "BEGIN MAIN PARAMETER DUMP" << endl;

   dgcout << " dggsType: " << dggsType << endl;
   dgcout << " curGrid: " << curGrid << endl;
   dgcout << " lastGrid: " << lastGrid << endl;
   dgcout << " numGrids: " << numGrids << endl;
   dgcout << " gridTopo: " << to_string(gridTopo) << endl;
   dgcout << " gridMetric: " << to_string(gridMetric) << endl;
   dgcout << " aperture: " << aperture << endl;
   dgcout << " projType: " << projType << endl;
   dgcout << " res: " << res << endl;
   dgcout << " superfund res: " << sfRes << endl;
   dgcout << " numAp4: " << numAp4 << endl;
   dgcout << " actualRes: " << actualRes << endl;
   dgcout << " placeRandom: " << placeRandom << endl;
   dgcout << " orientCenter: " << orientCenter << endl;
   dgcout << " vert0: " << vert0 << endl;
   dgcout << " azimuthDegs: " << azimuthDegs << endl;
   dgcout << " earthRadius: " << earthRadius << endl;
   dgcout << " precision: " << precision << endl;
   dgcout << " verbosity: " << verbosity << endl;
   dgcout << " megaVerbose: " << megaVerbose << endl;

   dgcout << " *orientRand: ";
   if (orientRand)
      dgcout << *orientRand << endl;
   else
      dgcout << "null" << endl;

   dgcout << " metaOutFileNameBase: " << metaOutFileNameBase << endl;

   dgcout << "END MAIN PARAMETER DUMP" << endl;

} // void MainParam::dump

DgGridPList::DgGridPList (void)
{
   vector<string*> choices;

   // dggrid_operation <GENERATE_GRID | BIN_POINT_VALS | BIN_POINT_PRESENCE |
   //                   TRANSFORM_POINTS | OUTPUT_STATS>
   choices.push_back(new string("GENERATE_GRID"));
   choices.push_back(new string("BIN_POINT_VALS"));
   choices.push_back(new string("BIN_POINT_PRESENCE"));
   choices.push_back(new string("TRANSFORM_POINTS"));
   choices.push_back(new string("OUTPUT_STATS"));
   insertParam(new DgStringChoiceParam("dggrid_operation", "GENERATE_GRID",
               &choices));
   dgg::util::release(choices);

   // dggs_type <CUSTOM | SUPERFUND | PLANETRISK |
   //            ISEA3H | ISEA4H | ISEA7H | ISEA43H | ISEA4T | ISEA4D |
   //            FULLER3H | FULLER4H | FULLER7H | FULLER43H | FULLER4T | FULLER4D>
   choices.push_back(new string("CUSTOM"));
   choices.push_back(new string("SUPERFUND"));
   choices.push_back(new string("PLANETRISK"));
   choices.push_back(new string("ISEA3H"));
   choices.push_back(new string("ISEA4H"));
   choices.push_back(new string("ISEA7H"));
   choices.push_back(new string("ISEA43H"));
   choices.push_back(new string("ISEA4T"));
   choices.push_back(new string("ISEA4D"));
   choices.push_back(new string("FULLER3H"));
   choices.push_back(new string("FULLER4H"));
   choices.push_back(new string("FULLER7H"));
   choices.push_back(new string("FULLER43H"));
   choices.push_back(new string("FULLER4T"));
   choices.push_back(new string("FULLER4D"));
   insertParam(new DgStringChoiceParam("dggs_type", "CUSTOM", &choices));
   dgg::util::release(choices);

   // dggs_base_poly <ICOSAHEDRON>
   choices.push_back(new string("ICOSAHEDRON"));
   insertParam(new DgStringChoiceParam("dggs_base_poly", "ICOSAHEDRON",
               &choices));
   dgg::util::release(choices);

   // dggs_topology <HEXAGON | TRIANGLE | DIAMOND>
   choices.push_back(new string("HEXAGON"));
   choices.push_back(new string("TRIANGLE"));
   choices.push_back(new string("DIAMOND"));
   insertParam(new DgStringChoiceParam("dggs_topology", "HEXAGON", &choices));
   dgg::util::release(choices);

   // dggs_proj <ISEA | FULLER | GNOMONIC>
   choices.push_back(new string("ISEA"));
   choices.push_back(new string("FULLER"));
   //choices.push_back(new string("GNOMONIC"));
   insertParam(new DgStringChoiceParam("dggs_proj", "ISEA", &choices));
   dgg::util::release(choices);

   // dggs_aperture_type <PURE | MIXED43 | SEQUENCE>
   choices.push_back(new string("PURE"));
   choices.push_back(new string("MIXED43"));
   choices.push_back(new string("SEQUENCE"));
   insertParam(new DgStringChoiceParam("dggs_aperture_type", "PURE",
             &choices));
   dgg::util::release(choices);

   // dggs_aperture < 3 | 4 | 7 >
   choices.push_back(new string("3"));
   choices.push_back(new string("4"));
   choices.push_back(new string("7"));
   insertParam(new DgStringChoiceParam("dggs_aperture", "4",
             &choices));
   dgg::util::release(choices);

   // dggs_aperture_sequence < apertureSequence >
   insertParam(new DgStringParam("dggs_aperture_sequence",
                     "333333333333"));

/*
   // dggs_aperture <int>
   insertParam(new DgIntParam("dggs_aperture", 4, 3, 7));
*/

   // dggs_num_aperture_4_res
   insertParam(new DgIntParam("dggs_num_aperture_4_res", 0, 0, MAX_DGG_RES));

   // proj_datum <WGS84_AUTHALIC_SPHERE WGS84_MEAN_SPHERE CUSTOM_SPHERE>
   choices.push_back(new string("WGS84_AUTHALIC_SPHERE"));
   choices.push_back(new string("WGS84_MEAN_SPHERE"));
   choices.push_back(new string("CUSTOM_SPHERE"));
   insertParam(new DgStringChoiceParam("proj_datum",
               "WGS84_AUTHALIC_SPHERE", &choices));
   dgg::util::release(choices);

   // proj_datum_radius <long double: km> (1.0 <= v <= 10000.0)
   insertParam(new DgDoubleParam("proj_datum_radius", DEFAULT_RADIUS_KM,
               1.0, 10000.0));

   //// specify the position and orientation

   // dggs_orient_specify_type <RANDOM | SPECIFIED | REGION_CENTER>
   choices.push_back(new string("RANDOM"));
   choices.push_back(new string("SPECIFIED"));
   choices.push_back(new string("REGION_CENTER"));
   insertParam(new DgStringChoiceParam("dggs_orient_specify_type", "SPECIFIED",
               &choices));
   dgg::util::release(choices);

   // dggs_num_placements <int> (v >= 1)
   insertParam(new DgIntParam("dggs_num_placements", 1, 1, INT_MAX, true));

   // dggs_orient_output_file_name <whatever.meta>
   insertParam(new DgStringParam("dggs_orient_output_file_name",
                     "grid.meta"));

   // dggs_orient_rand_seed <unsigned long int int>
   insertParam(new DgULIntParam("dggs_orient_rand_seed", 77316727, 0,
                     ULONG_MAX, true));

   // dggs_vert0_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   insertParam(new DgDoubleParam("dggs_vert0_lon", 11.25, -180.0, 180.0));

   // dggs_vert0_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   insertParam(new DgDoubleParam("dggs_vert0_lat", 58.28252559, -90.0, 90.0));

   // dggs_vert0_azimuth <long double: decimal degrees> (0.0 <= v < 360.0)
   insertParam(new DgDoubleParam("dggs_vert0_azimuth", 0.0, 0.0, 360.0));

   // region_center_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   insertParam(new DgDoubleParam("region_center_lon", 0.0, -180.0, 180.0));

   // region_center_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   insertParam(new DgDoubleParam("region_center_lat", 0.0, -90.0, 90.0));

   // dggs_res_specify_type <SPECIFIED | CELL_AREA | INTERCELL_DISTANCE>
   choices.push_back(new string("SPECIFIED"));
   choices.push_back(new string("CELL_AREA"));
   choices.push_back(new string("INTERCELL_DISTANCE"));
   insertParam(new DgStringChoiceParam("dggs_res_specify_type", "SPECIFIED",
               &choices));
   dgg::util::release(choices);

   // dggs_res_specify_area <long double: km^2> (v > 0.0)
   insertParam(new DgDoubleParam("dggs_res_specify_area", 100.0,
                                 0.0, 4.0 * M_PI * 6500.0 * 6500.0));

   // dggs_res_specify_intercell_distance <long double: km> (v > 0.0)
   insertParam(new DgDoubleParam("dggs_res_specify_intercell_distance", 100.0,
                                 0.0, 2.0 * M_PI * 6500.0));

   // dggs_res_specify_rnd_down <TRUE | FALSE> (true indicates round down, false up)
   insertParam(new DgBoolParam("dggs_res_specify_rnd_down", true));

   // dggs_res_spec <int> (0 <= v <= MAX_DGG_RES)
   insertParam(new DgIntParam("dggs_res_spec", 9, 0, MAX_DGG_RES));

   // rng_type <RAND | MOTHER>
   choices.push_back(new string("RAND"));
   choices.push_back(new string("MOTHER"));
   insertParam(new DgStringChoiceParam("rng_type", "RAND", &choices));
   dgg::util::release(choices);

   // geodetic_densify <long double: decimal degrees> (v >= 0.0)
   insertParam(new DgDoubleParam("geodetic_densify", 0.0, 0.0, 360.0));

   // clip_subset_type <WHOLE_EARTH | AIGEN | SHAPEFILE | GDAL | SEQNUMS | ADDRESSES |
   //                    COARSE_CELLS | INPUT_ADDRESS_TYPE >
   choices.push_back(new string("WHOLE_EARTH"));
   choices.push_back(new string("AIGEN"));
   choices.push_back(new string("SHAPEFILE"));
#ifdef USE_GDAL
   choices.push_back(new string("GDAL"));
#endif
   choices.push_back(new string("SEQNUMS"));
   choices.push_back(new string("ADDRESSES"));
   choices.push_back(new string("POINTS"));
   choices.push_back(new string("COARSE_CELLS"));
   choices.push_back(new string("INPUT_ADDRESS_TYPE"));
   insertParam(new DgStringChoiceParam("clip_subset_type", "WHOLE_EARTH",
               &choices));
   dgg::util::release(choices);

   // clip_cell_addresses <clipCell1 clipCell2 ... clipCellN>
   insertParam(new DgStringParam("clip_cell_addresses", ""));

   // clip_cell_res <int> (0 < v <= MAX_DGG_RES)
   insertParam(new DgIntParam("clip_cell_res", 1, 1, MAX_DGG_RES));

   // clip_cell_densification <int> (0 <= v <= 500)
   insertParam(new DgIntParam("clip_cell_densification", 1, 0, 500));

   // clip_region_files <fileName1 fileName2 ... fileNameN>
   insertParam(new DgStringParam("clip_region_files", "test.gen"));

   // clip_type <POLY_INTERSECT>
   choices.push_back(new string("POLY_INTERSECT"));
   insertParam(new DgStringChoiceParam("clip_type", "POLY_INTERSECT",
               &choices));
   dgg::util::release(choices);

   // clipper_scale_factor <unsigned long int>
   insertParam(new DgULIntParam("clipper_scale_factor", 1000000L, 1, ULONG_MAX, true));
   //insertParam(new DgULIntParam("clipper_scale_factor", 1000000000L, 1, ULONG_MAX, true));

   init2();

} // DgGridPList::DgGridPList

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
DgGridPList::init2 (void)
//
// This method was created solely because gcc optimiziation choked when
// everything was in the constructor.
//
{
   vector<string*> choices;

   // densification <int> (0 <= v <= 500)
   insertParam(new DgIntParam("densification", 0, 0, 500));

   // longitude_wrap_mode < WRAP | UNWRAP_WEST | UNWRAP_EAST >
   choices.push_back(new string("WRAP"));
   choices.push_back(new string("UNWRAP_WEST"));
   choices.push_back(new string("UNWRAP_EAST"));
   insertParam(new DgStringChoiceParam("longitude_wrap_mode", "WRAP",
               &choices));
   dgg::util::release(choices);

   // unwrap_points <TRUE | FALSE> (true indicates unwrap center point
   //        longitude to match the cell boundary, false allow to wrap)
   insertParam(new DgBoolParam("unwrap_points", true));

   // precision <int> (0 <= v <= 30)
   insertParam(new DgIntParam("precision", DEFAULT_PRECISION, 0, INT_MAX));

   // output_cell_label_type <GLOBAL_SEQUENCE | ENUMERATION | SUPERFUND | OUTPUT_ADDRESS_TYPE >
   choices.push_back(new string("GLOBAL_SEQUENCE"));
   choices.push_back(new string("ENUMERATION"));
   choices.push_back(new string("SUPERFUND"));
   choices.push_back(new string("OUTPUT_ADDRESS_TYPE"));
   insertParam(new DgStringChoiceParam("output_cell_label_type", "GLOBAL_SEQUENCE",
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
   insertParam(new DgStringChoiceParam("cell_output_type", "AIGEN",
               &choices));
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
   insertParam(new DgStringChoiceParam("point_output_type", "NONE",
               &choices));
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
   insertParam(new DgStringChoiceParam("randpts_output_type", "NONE",
               &choices));
   dgg::util::release(choices);

#ifdef USE_GDAL
   // cell_output_gdal_format <gdal driver type>
   insertParam(new DgStringParam("cell_output_gdal_format", "GeoJSON"));

   // point_output_gdal_format <gdal driver type>
   insertParam(new DgStringParam("point_output_gdal_format", "GeoJSON"));

   // collection_output_gdal_format <gdal driver type>
   insertParam(new DgStringParam("collection_output_gdal_format", "GeoJSON"));

   // clip_using_holes <TRUE | FALSE>
   insertParam(new DgBoolParam("clip_using_holes", false));
#endif

   // cell_output_file_name <outputFileName>
   insertParam(new DgStringParam("cell_output_file_name", "cells"));

   // point_output_file_name <outputFileName>
   insertParam(new DgStringParam("point_output_file_name", "centers"));

   // randpts_output_file_name <outputFileName>
   insertParam(new DgStringParam("randpts_output_file_name", "randPts"));

   // collection_output_file_name <outputFileName>
   insertParam(new DgStringParam("collection_output_file_name", "cells"));

   // shapefile_id_field_length <int> (1 <= v <= 50)
   insertParam(new DgIntParam("shapefile_id_field_length", 11, 1, 50));

   // kml_default_width <int> (1 <= v <= 100)
   insertParam(new DgIntParam("kml_default_width",
               DgOutLocFile::defaultKMLWidth, 1, 100));

   // kml_default_color <string> <hexadecmial color>
   insertParam(new DgStringParam("kml_default_color",
               DgOutLocFile::defaultKMLColor));

   // kml_name <string> <string name>
   insertParam(new DgStringParam("kml_name",
               DgOutLocFile::defaultKMLName));

   // kml_description <string> <string description>
   insertParam(new DgStringParam("kml_description",
               DgOutLocFile::defaultKMLDescription));

   ///// PlanetRisk output formats /////

   // neighbor_output_type <NONE | TEXT | GEOJSON_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("TEXT"));
   choices.push_back(new string("GDAL_COLLECTION"));
   insertParam(new DgStringChoiceParam("neighbor_output_type", "NONE",
               &choices));
   dgg::util::release(choices);

   // neighbor_output_file_name <outputFileName>
   insertParam(new DgStringParam("neighbor_output_file_name", "nbr"));

   // children_output_type <NONE | TEXT | GDAL_COLLECTION>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("TEXT"));
   choices.push_back(new string("GDAL_COLLECTION"));
   insertParam(new DgStringChoiceParam("children_output_type", "NONE",
               &choices));
   dgg::util::release(choices);

   // children_output_file_name <outputFileName>
   insertParam(new DgStringParam("children_output_file_name", "chld"));

   ///// additional random points parameters /////

   // randpts_concatenate_output <TRUE | FALSE>
   insertParam(new DgBoolParam("randpts_concatenate_output", true));

   // randpts_num_per_cell <int> (v >= 0)
   insertParam(new DgIntParam("randpts_num_per_cell", 0, 0, INT_MAX));

   // randpts_seed <unsigned long int>
   insertParam(new DgULIntParam("randpts_seed", 77316727, 0, ULONG_MAX, true));

/*
   // clip_randpts <TRUE | FALSE>
   insertParam(new DgBoolParam("clip_randpts", true));

   // randpts_clipped_file_name <outputFileName>
   insertParam(new DgStringParam("randpts_clipped_file_name", "clippedRandPts"));
*/

/* // quad_bndry_nudge <long double> (0.0 <= v < 1.0)
   insertParam(new DgDoubleParam("quad_bndry_nudge", 0.001, 0.0, 1.0));
*/
/*** shapefile attribute stuff
   ///// shapefile attribute parameters
   // build_shapefile_attributes <TRUE | FALSE>
   insertParam(new DgBoolParam("build_shapefile_attributes", false));

   // shapefile_attribute_default_int <int>
   insertParam(new DgIntParam("shapefile_attribute_default_int", 0, INT_MIN, INT_MAX));

   // shapefile_attribute_default_double <double>
   insertParam(new DgDoubleParam("shapefile_attribute_default_double", 0.0, -DBL_MIN, DBL_MAX));

   // shapefile_attribute_default_string <defaultString>
   insertParam(new DgStringParam("shapefile_attribute_default_string", "X"));

   // build_shapefile_attributes_source <NONE | CLIP_FILES | SHAPEFILES>
   choices.push_back(new string("NONE"));
   choices.push_back(new string("CLIP_FILES"));
   choices.push_back(new string("SHAPEFILES"));
   insertParam(new DgStringChoiceParam("build_shapefile_attributes_source",
               "NONE", &choices));
   dgg::util::release(choices);

   // attribute_files <fileName1 fileName2 ... fileNameN>
   insertParam(new DgStringParam("attribute_files", "test.shp"));
****/

   //  update_frequency <int> (v >= 0)
   insertParam(new DgULIntParam("update_frequency", 100000, 0, ULONG_MAX, true));

   //  max_cells_per_output_file <int> (v >= 0)
   insertParam(new DgULIntParam("max_cells_per_output_file", 0, 0, ULONG_MAX, true));

   //  output_first_seqnum <int> (v >= 0)
   insertParam(new DgULIntParam("output_first_seqnum", 1, 0, ULONG_MAX, true));

   //  output_last_seqnum <int> (v >= 0)
   insertParam(new DgULIntParam("output_last_seqnum", ULONG_MAX, 0, ULONG_MAX, true));

   //  verbosity <int> (0 <= v <= 3)
   insertParam(new DgIntParam("verbosity", 0, 0, 3));

   // pause_on_startup <TRUE | FALSE>
   insertParam(new DgBoolParam("pause_on_startup", false));

   // pause_before_exit <TRUE | FALSE>
   insertParam(new DgBoolParam("pause_before_exit", false));

   ///// binvals parameters /////

   // bin_method <ARITHMETIC_MEAN>
   choices.push_back(new string("ARITHMETIC_MEAN"));
   insertParam(new DgStringChoiceParam("bin_method", "ARITHMETIC_MEAN",
                                       &choices));
   dgg::util::release(choices);

   // bin_coverage <GLOBAL | PARTIAL>
   choices.push_back(new string("GLOBAL"));
   choices.push_back(new string("PARTIAL"));
   insertParam(new DgStringChoiceParam("bin_coverage", "GLOBAL", &choices));
   dgg::util::release(choices);

   // input_files <fileName1 fileName2 ... fileNameN>
   insertParam(new DgStringParam("input_files", "vals.txt"));

   // input_delimiter <v is any character in long double quotes>
   insertParam(new DgStringParam("input_delimiter", "\" \""));

   // output_file_name <fileName>

   insertParam(new DgStringParam("output_file_name", "valsout.txt"));

   // output_address_type < GEO | PLANE | PROJTRI | Q2DD | Q2DI |
   //        SEQNUM | VERTEX2DD | ZORDER | ZORDER_STRING >
   for (int i = 0; ; i++) {
      if (addTypeStrings[i] == "INVALID")
         break;
      choices.push_back(new string(addTypeStrings[i]));
   }
   insertParam(new DgStringChoiceParam("output_address_type",
               "SEQNUM", &choices));
   dgg::util::release(choices);

   // output_delimiter <v is any character in double quotes>
   insertParam(new DgStringParam("output_delimiter", "\" \""));

   // cell_output_control <OUTPUT_ALL | OUTPUT_OCCUPIED>
   choices.push_back(new string("OUTPUT_ALL"));
   choices.push_back(new string("OUTPUT_OCCUPIED"));
   insertParam(new DgStringChoiceParam("cell_output_control", "OUTPUT_ALL",
                                       &choices));
   dgg::util::release(choices);

   ///// transform parameters (that haven't already been inserted) /////

   // input_file_name <fileName>
   insertParam(new DgStringParam("input_file_name", "valsin.txt"));

   // input_address_type < GEO | PLANE | PROJTRI | Q2DD | Q2DI |
   //        SEQNUM | VERTEX2DD | ZORDER | ZORDER_STRING >
   for (int i = 0; ; i++) {
      if (addTypeStrings[i] == "INVALID")
         break;
      choices.push_back(new string(addTypeStrings[i]));
   }
   insertParam(new DgStringChoiceParam("input_address_type",
               "SEQNUM", &choices));
   dgg::util::release(choices);

   // output_count <TRUE | FALSE>
   insertParam(new DgBoolParam("output_count", true));

} // DgGridPList::init2

