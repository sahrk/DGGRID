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
// SubOpDGG.cpp: SubOpDGG class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgConstants.h>
#include <dglib/DgConverterBase.h>
#include <dglib/DgAddressType.h>
#include <dglib/DgRandom.h>
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgGeoProjConverter.h>
#include <dglib/DgIDGGutil.h>
#include <dglib/DgSuperfund.h>
#include <dglib/DgZOrderRF.h>
#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgZ3RF.h>
#include <dglib/DgZ3StringRF.h>
#include <dglib/DgZ7RF.h>
#include <dglib/DgZ7StringRF.h>

#include "OpBasic.h"
#include "SubOpDGG.h"

using namespace dgg::topo;
using namespace dgg::addtype;

const int SubOpDGG::MAX_DGG_RES = 35;

////////////////////////////////////////////////////////////////////////////////
SubOpDGG::SubOpDGG (OpBasic& op, bool _activate)
   : SubOpBasic (op, _activate),
     dggsType (""), gridTopo (dgg::topo::InvalidTopo),
     gridMetric (dgg::topo::InvalidMetric), aperture (4),
     projType ("ISEA"), res (5), actualRes (5),
     placeRandom (false), orientCenter (false), orientRand (0),
     numGrids (1), curGrid (0), lastGrid (false), sampleCount(0), nSamplePts(0),
     azimuthDegs (0.0), datum (""), apertureType (""),
     isMixed43 (false), isSuperfund (false), isApSeq (false)
{
}

////////////////////////////////////////////////////////////////////////////////
// choose a reference frame based on address type
// returns whether or not seq nums are used
bool
SubOpDGG::addressTypeToRF (DgAddressType type, const DgRFBase** rf,
             const DgRFBase** chdRF, int forceRes)
{
   const DgIDGGBase* dgg = &this->dgg();
   const DgIDGGBase* chdDgg = &this->chdDgg();
   if (forceRes >= 0) {
      dgg = &dggs().idggBase(forceRes);
      chdDgg = &dggs().idggBase(forceRes + 1);
   }

   bool seqNum = false;
   *rf = nullptr;
   if (chdRF) *chdRF = nullptr;

   switch (type) {
      case Geo:
         *rf = &this->deg();
         if (chdRF) *chdRF = &this->chdDeg();
/*
         *rf = &dgg->geoRF();
         if (chdRF) *chdRF = &chdDgg->geoRF();
*/
         break;

      case Plane:
         *rf = &dgg->planeRF();
         if (chdRF) *chdRF = &chdDgg->planeRF();
         break;

      case ProjTri:
         *rf = &dgg->projTriRF();
         if (chdRF) *chdRF = &chdDgg->projTriRF();
         break;

      case Q2DD:
         *rf = &dgg->q2ddRF();
         if (chdRF) *chdRF = &chdDgg->q2ddRF();
         break;

      case Q2DI:
         *rf = dgg;
         if (chdRF) *chdRF = chdDgg;
         break;

      case SeqNum:
/*
         if (isInput && dgg->isApSeq)
            ::report("input_address_type of SEQNUM not supported for dggs_aperture_type of SEQUENCE",
                  DgBase::Fatal);
*/

         seqNum = true;
         *rf = dgg;
         if (chdRF) *chdRF = chdDgg;
         break;

      case Vertex2DD:
         *rf = &dgg->vertexRF();
         if (chdRF) *chdRF = &chdDgg->vertexRF();
         break;

      case Z3:
         if (isApSeq)
            ::report("input_address_type of Z3 not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->z3RF()) {
            *rf = dgg->z3RF();
            if (chdRF) *chdRF = chdDgg->z3RF();
         } else
            ::report("addressTypeToRF(): Z3 only supported for aperture 3 hexagon grids",
                     DgBase::Fatal);

         break;

      case Z3String:
         if (isApSeq)
            ::report("input_address_type of Z3_STRING not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->z3StrRF()) {
            *rf = dgg->z3StrRF();
            if (chdRF) *chdRF = chdDgg->z3StrRF();
         } else
            ::report("addressTypeToRF(): Z3_STRING only supported for aperture 3 hexagon grids",
                     DgBase::Fatal);

         break;

      case Z7:
         if (isApSeq)
            ::report("input_address_type of Z7 not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->z7RF()) {
            *rf = dgg->z7RF();
            if (chdRF) *chdRF = chdDgg->z7RF();
         } else
            ::report("addressTypeToRF(): Z7 only supported for aperture 7 hexagon grids",
                     DgBase::Fatal);

         break;

      case Z7String:
         if (isApSeq)
            ::report("input_address_type of Z7_STRING not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->z7StrRF()) {
            *rf = dgg->z7StrRF();
            if (chdRF) *chdRF = chdDgg->z7StrRF();
         } else
            ::report("addressTypeToRF(): Z7_STRING only supported for aperture 7 hexagon grids",
                     DgBase::Fatal);

         break;

      case ZOrder:
         if (isApSeq)
            ::report("input_address_type of ZORDER not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->zorderRF()) {
            *rf = dgg->zorderRF();
            if (chdRF) *chdRF = chdDgg->zorderRF();
         } else
            ::report("addressTypeToRF(): ZORDER only supported for aperture 3 or 4",
                     DgBase::Fatal);

         break;

      case ZOrderString:
         if (isApSeq)
            ::report("input_address_type of ZORDER_STRING not supported for dggs_aperture_type of SEQUENCE",
                     DgBase::Fatal);

         if (dgg->zorderStrRF()) {
            *rf = dgg->zorderStrRF();
            if (chdRF) *chdRF = chdDgg->zorderStrRF();
         } else
            ::report("addressTypeToRF(): ZORDER_STRING only supported for aperture 3 or 4",
                     DgBase::Fatal);

         break;

      case InvalidAddressType:
      default:
         ::report("addressTypeToRF(): invalid address type", DgBase::Fatal);
   }

   return seqNum;

} // void SubOpDGG::addressTypeToRF

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::initializeOp (void)
{
   vector<string*> choices;

   // dggs_type <CUSTOM | SUPERFUND | PLANETRISK | IGEO7 |
   //            ISEA3H | ISEA4H | ISEA7H | ISEA43H | ISEA4T | ISEA4D |
   //            FULLER3H | FULLER4H | FULLER7H | FULLER43H | FULLER4T | FULLER4D>
   choices.push_back(new string("CUSTOM"));
   choices.push_back(new string("SUPERFUND"));
   choices.push_back(new string("PLANETRISK"));
   choices.push_back(new string("IGEO7"));
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
   pList().insertParam(new DgStringChoiceParam("dggs_type", "CUSTOM", &choices));
   dgg::util::release(choices);

   // dggs_base_poly <ICOSAHEDRON>
   choices.push_back(new string("ICOSAHEDRON"));
   pList().insertParam(new DgStringChoiceParam("dggs_base_poly", "ICOSAHEDRON",
               &choices));
   dgg::util::release(choices);

   // dggs_topology <HEXAGON | TRIANGLE | DIAMOND>
   choices.push_back(new string("HEXAGON"));
   choices.push_back(new string("TRIANGLE"));
   choices.push_back(new string("DIAMOND"));
   pList().insertParam(new DgStringChoiceParam("dggs_topology", "HEXAGON", &choices));
   dgg::util::release(choices);

   // dggs_proj <ISEA | FULLER | GNOMONIC>
   choices.push_back(new string("ISEA"));
   choices.push_back(new string("FULLER"));
   //choices.push_back(new string("GNOMONIC"));
   pList().insertParam(new DgStringChoiceParam("dggs_proj", "ISEA", &choices));
   dgg::util::release(choices);

   // dggs_aperture_type <PURE | MIXED43 | SEQUENCE>
   choices.push_back(new string("PURE"));
   choices.push_back(new string("MIXED43"));
   choices.push_back(new string("SEQUENCE"));
   pList().insertParam(new DgStringChoiceParam("dggs_aperture_type", "PURE",
             &choices));
   dgg::util::release(choices);

   // dggs_aperture < 3 | 4 | 7 >
   choices.push_back(new string("3"));
   choices.push_back(new string("4"));
   choices.push_back(new string("7"));
   pList().insertParam(new DgStringChoiceParam("dggs_aperture", "4",
             &choices));
   dgg::util::release(choices);

   // dggs_aperture_sequence < apertureSequence >
   pList().insertParam(new DgStringParam("dggs_aperture_sequence",
                     "333333333333"));

/*
   // dggs_aperture <int>
   pList().insertParam(new DgIntParam("dggs_aperture", 4, 3, 7));
*/

   // dggs_num_aperture_4_res
   pList().insertParam(new DgIntParam("dggs_num_aperture_4_res", 0, 0, MAX_DGG_RES));

   // proj_datum <WGS84_AUTHALIC_SPHERE WGS84_MEAN_SPHERE CUSTOM_SPHERE>
   choices.push_back(new string("WGS84_AUTHALIC_SPHERE"));
   choices.push_back(new string("WGS84_MEAN_SPHERE"));
   choices.push_back(new string("CUSTOM_SPHERE"));
   pList().insertParam(new DgStringChoiceParam("proj_datum",
               "WGS84_AUTHALIC_SPHERE", &choices));
   dgg::util::release(choices);

   // proj_datum_radius <long double: km> (1.0 <= v <= 10000.0)
   pList().insertParam(new DgDoubleParam("proj_datum_radius", DEFAULT_RADIUS_KM,
               1.0, 10000.0));

   //// specify the position and orientation

   // dggs_orient_specify_type <RANDOM | SPECIFIED | REGION_CENTER>
   choices.push_back(new string("RANDOM"));
   choices.push_back(new string("SPECIFIED"));
   choices.push_back(new string("REGION_CENTER"));
   pList().insertParam(new DgStringChoiceParam("dggs_orient_specify_type", "SPECIFIED",
               &choices));
   dgg::util::release(choices);

   // dggs_num_placements <int> (v >= 1)
   pList().insertParam(new DgIntParam("dggs_num_placements", 1, 1, INT_MAX, true));

   // dggs_orient_rand_seed <unsigned long int int>
   pList().insertParam(new DgULIntParam("dggs_orient_rand_seed", 77316727, 0,
                     ULONG_MAX, true));

   // dggs_vert0_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   pList().insertParam(new DgDoubleParam("dggs_vert0_lon", 11.25, -180.0, 180.0));

   // dggs_vert0_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   pList().insertParam(new DgDoubleParam("dggs_vert0_lat", 58.28252559, -90.0, 90.0));

   // dggs_vert0_azimuth <long double: decimal degrees> (0.0 <= v < 360.0)
   pList().insertParam(new DgDoubleParam("dggs_vert0_azimuth", 0.0, 0.0, 360.0));

   // region_center_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   pList().insertParam(new DgDoubleParam("region_center_lon", 0.0, -180.0, 180.0));

   // region_center_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   pList().insertParam(new DgDoubleParam("region_center_lat", 0.0, -90.0, 90.0));

   // dggs_res_specify_type <SPECIFIED | CELL_AREA | INTERCELL_DISTANCE>
   choices.push_back(new string("SPECIFIED"));
   choices.push_back(new string("CELL_AREA"));
   choices.push_back(new string("INTERCELL_DISTANCE"));
   pList().insertParam(new DgStringChoiceParam("dggs_res_specify_type", "SPECIFIED",
               &choices));
   dgg::util::release(choices);

   // dggs_res_specify_area <long double: km^2> (v > 0.0)
   pList().insertParam(new DgDoubleParam("dggs_res_specify_area", 100.0,
                                 0.0, 4.0 * M_PI * 6500.0 * 6500.0));

   // dggs_res_specify_intercell_distance <long double: km> (v > 0.0)
   pList().insertParam(new DgDoubleParam("dggs_res_specify_intercell_distance", 100.0,
                                 0.0, 2.0 * M_PI * 6500.0));

   // dggs_res_specify_rnd_down <TRUE | FALSE> (true indicates round down, false up)
   pList().insertParam(new DgBoolParam("dggs_res_specify_rnd_down", true));

   // dggs_res_spec <int> (0 <= v <= MAX_DGG_RES)
   pList().insertParam(new DgIntParam("dggs_res_spec", 9, 0, MAX_DGG_RES));

   return 0;

} // int SubOpDGG::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   // setup preset DGGS (if any)
   string tmp;
   getParamValue(pList(), "dggs_type", tmp, false);
   string tmplc = toLower(tmp);
   if (tmplc != "custom") {
      // these params are common to all presets
      pList().setPresetParam("dggs_base_poly", "ICOSAHEDRON");
      pList().setPresetParam("dggs_orient_specify_type", "SPECIFIED");
      pList().setPresetParam("dggs_num_placements", "1");
      pList().setPresetParam("dggs_vert0_lon", "11.25");
      pList().setPresetParam("dggs_vert0_lat", "58.28252559");
      pList().setPresetParam("dggs_vert0_azimuth", "0.0");
      pList().setPresetParam("dggs_res_specify_type", "SPECIFIED");
      pList().setPresetParam("dggs_res_spec", "9");

      if (tmplc == "superfund") {
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "FULLER");
         pList().setPresetParam("dggs_num_aperture_4_res", "2");
         pList().setPresetParam("dggs_aperture_type", "MIXED43");
         pList().setPresetParam("output_cell_label_type", "SUPERFUND");
      } else if (tmplc == "planetrisk") {
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "ISEA");
         pList().setPresetParam("dggs_aperture_type", "SEQUENCE");
         pList().setPresetParam("dggs_aperture_sequence", "43334777777777777777777");
         pList().setPresetParam("dggs_res_spec", "11");
      } else if (tmplc == "igeo7") {
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "ISEA");
         pList().setPresetParam("dggs_aperture_type", "PURE");
         pList().setPresetParam("dggs_aperture", "7");
         pList().setPresetParam("dggs_res_spec", "9");
         pList().setPresetParam("input_address_type", "Z7");
         pList().setPresetParam("output_address_type", "Z7");
          pList().setPresetParam("output_cell_label_type", "OUTPUT_ADDRESS_TYPE");
      } else {
         // get the topology
         char topo = tmplc[tmplc.length() - 1];
         switch (topo) {
            case 'h':
               pList().setPresetParam("dggs_topology", "HEXAGON");
               break;
            case 't':
               pList().setPresetParam("dggs_topology", "TRIANGLE");
               break;
            case 'd':
               pList().setPresetParam("dggs_topology", "DIAMOND");
               break;
         }

         // get the projection
         int projLen;

         if (!tmplc.compare(0, 4, "isea")) {
            pList().setPresetParam("dggs_proj", "ISEA");
            projLen = (int) string("isea").length();
         } else { // must be FULLER
            pList().setPresetParam("dggs_proj", "FULLER");
            projLen = (int) string("fuller").length();
         }

         // get the aperture
         tmplc = tmplc.substr(projLen, tmplc.length() - projLen - 1);
         if (tmplc == "43") {
            pList().setPresetParam("dggs_aperture_type", "MIXED43");
         } else {
            pList().setPresetParam("dggs_aperture_type", "PURE");
            pList().setPresetParam("dggs_aperture", tmplc);
         }
      }
   }

   string gridTopoStr = "";
   getParamValue(pList(), "dggs_topology", gridTopoStr, false);
   gridTopo = dgg::topo::stringToGridTopology(gridTopoStr);

/* metric not exposed to user yet; D8 is broken
   string gridMetricStr = "";
   getParamValue(pList(), "dggs_metric", gridMetricStr, false);
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
         ::report("SubOpDGG::setupOp() invalid dggs_topology" +
              gridTopoStr, DgBase::Fatal);
   }

   getParamValue(pList(), "dggs_aperture_type", apertureType, false);
   if (apertureType == "MIXED43")
      isMixed43 = true;
   else if (apertureType == "SEQUENCE")
      isApSeq = true;

   numAp4 = 0;
   if (isMixed43)
      getParamValue(pList(), "dggs_num_aperture_4_res", numAp4, false);
   else if (isApSeq) {
      string tmp;
      getParamValue(pList(), "dggs_aperture_sequence", tmp, false);
      apSeq = DgApSeq(tmp);
   } else {
      getParamValue(pList(), "dggs_aperture", tmp, false);
      aperture = dgg::util::from_string<int>(tmp);
   }

   getParamValue(pList(), "dggs_num_placements", numGrids, false);
   getParamValue(pList(), "dggs_proj", projType, false);
   getParamValue(pList(), "dggs_vert0_azimuth", azimuthDegs, false);

   long double lon0, lat0;
   getParamValue(pList(), "dggs_vert0_lon", lon0, false);
   getParamValue(pList(), "dggs_vert0_lat", lat0, false);
   vert0 = DgGeoCoord(lon0, lat0, false);

   getParamValue(pList(), "proj_datum", datum, false);
   if (datum == "WGS84_AUTHALIC_SPHERE")
      earthRadius = 6371.007180918475L;
   else if (datum == "WGS84_MEAN_SPHERE")
      earthRadius = 6371.0087714L;
   else // datum must be CUSTOM_SPHERE
      getParamValue(pList(), "proj_datum_radius", earthRadius, false);

   if (tmp == "SUPERFUND") {
      isSuperfund = true;

      if (!isMixed43)
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_aperture_type of MIXED43", DgBase::Fatal);

      if (numAp4 != 2)
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_num_aperture_4_res of 2", DgBase::Fatal);

      string resType;
      getParamValue(pList(), "dggs_res_specify_type", resType, false);

      if (resType != string("SPECIFIED"))
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_res_specify_type of SPECIFIED", DgBase::Fatal);

      getParamValue(pList(), "dggs_res_spec", res, false);
      sfRes = res;
      actualRes = res = sfRes2actualRes(sfRes);
   } else { // not superfund
      sfRes = 0;
      determineRes();
      actualRes = res;
   }

   string dummy;
   getParamValue(pList(), "dggs_orient_specify_type", dummy, false);
   if (dummy == string("SPECIFIED"))
      placeRandom = false;
   else if (dummy == string("REGION_CENTER")) {
      placeRandom = false;
      orientCenter = true;
   } else {
      placeRandom = true;

      unsigned long int ranSeed = 0;
      getParamValue(pList(), "dggs_orient_rand_seed", ranSeed, false);
      if (op.mainOp.useMother) {
         orientRand = new DgRandMother(ranSeed);
      } else {
         orientRand = new DgRand(ranSeed);
      }
   }

   curGrid = 0;
   lastGrid = false;

   return 0;

} // SubOpDGG::setupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::cleanupOp (void) {

   delete orientRand;
   return 0;

} // SubOpDGG::cleanupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::executeOp (void) {

//cout << "YYY " << curGrid << " " << numGrids << " " << lastGrid << endl;

   curGrid++;
   if (curGrid == numGrids) lastGrid = true;

//cout << "ZZZ " << curGrid << " " << numGrids << " " << lastGrid << endl;

   orientGrid();

   if (curGrid == 1) {
      _pGeoRF = DgGeoSphRF::makeRF(net0(), datum, earthRadius);
   }

   _pDGGS  = DgIDGGSBase::makeRF(net0(), geoRF(), vert0,
             azimuthDegs, aperture, actualRes+2, gridTopo,
             gridMetric, "IDGGS", projType, isMixed43, numAp4,
             isSuperfund, isApSeq, apSeq);

   _pDGG = &dggs().idggBase(actualRes);

   // child dgg
   _pChdDgg = &dggs().idggBase(actualRes + 1);

   // set-up to convert to degrees
   _pDeg = DgGeoSphDegRF::makeRF(geoRF(), _pGeoRF->name() + "Deg");
   _pChdDeg = DgGeoSphDegRF::makeRF(_pChdDgg->geoRF(), _pChdDgg->geoRF().name() + "Deg");

   return 0;

} // SubOpDGG::executeOp

////////////////////////////////////////////////////////////////////////////////
void
SubOpDGG::determineRes (void)
{
   int maxRes = (apertureType != "SEQUENCE") ? MAX_DGG_RES : apSeq.lastRes();

   // get the parameters

   string resType;
   getParamValue(pList(), "dggs_res_specify_type", resType, false);

   if (resType == string("SPECIFIED")) {
      getParamValue(pList(), "dggs_res_spec", res, false);
   } else {
      bool area;
      long double value = 0;
      if (resType == string("CELL_AREA")) {
         area = true;
         getParamValue(pList(), "dggs_res_specify_area", value, false);
      } else {
         area = false;
         getParamValue(pList(), "dggs_res_specify_intercell_distance", value,
                       false);
      }

      bool roundDown = true;  // round up/down to nearest available cell size
      getParamValue(pList(), "dggs_res_specify_rnd_down", roundDown, false);

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
      ::report("SubOpDGG::determineRes() desired resolution exceeds "
               "maximum resolution for this topology", DgBase::Fatal);
   }
   else if (res < 0) res = 0;

} // void SubOpDGG::determineRes

////////////////////////////////////////////////////////////////////////////////
// Set the orientation parameters if not specified
void
SubOpDGG::orientGrid (void)
{
   if (placeRandom) { // randomize grid orientation

      vert0 = orientRand->nextGeo();

      azimuthDegs = orientRand->randInRange(0.0, 360.0);

      // set the paramlist to match so we can print it back out
      pList().setParam("dggs_orient_specify_type", "SPECIFIED");
      pList().setParam("dggs_num_placements", dgg::util::to_string(1));
      pList().setParam("dggs_vert0_lon", dgg::util::to_string(vert0.lonDegs()));
      pList().setParam("dggs_vert0_lat", dgg::util::to_string(vert0.latDegs()));
      pList().setParam("dggs_vert0_azimuth", dgg::util::to_string(azimuthDegs));

      dgcout << "Grid " << curGrid <<
           " #####################################################" << endl;
      dgcout << "grid #" << curGrid << " orientation randomized to: " << endl;
      dgcout << pList() << endl;

   } else if (orientCenter && curGrid == 1) {

      DgRFNetwork netc;
      const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(netc, "GS0", earthRadius));

      long double lonc = 0.0, latc = 0.0;
      getParamValue(pList(), "region_center_lon", lonc, false);
      getParamValue(pList(), "region_center_lat", latc, false);

      const DgProjGnomonicRF& gnomc =
            *(DgProjGnomonicRF::makeRF(netc, "cgnom", DgGeoCoord(lonc, latc, false)));
      Dg2WayGeoProjConverter(geoRF, gnomc);

      DgLocation* gloc = gnomc.makeLocation(DgDVec2D(-7289214.618283,
                                                      7289214.618283));
      geoRF.convert(gloc);

      DgGeoCoord p0 = *geoRF.getAddress(*gloc);
      delete gloc;

      gloc = gnomc.makeLocation(DgDVec2D(2784232.232959, 2784232.232959));
      geoRF.convert(gloc);
      DgGeoCoord p1 = *geoRF.getAddress(*gloc);
      delete gloc;

      vert0 = p0;
      azimuthDegs = DgGeoSphRF::azimuth(p0, p1, false);
   }

} // void SubOpDGG::orientGrid

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
