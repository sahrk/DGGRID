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
#include <dglib/DgZ3System.h>
#include <dglib/DgHierNdxSystemRFSBase.h>
#include <dglib/DgAuthalicConverter.h>
#include <dglib/DgSphereConverter.h>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

#include "OpBasic.h"
#include "SubOpDGG.h"

// the DYMAXION orientation preset (see setupOp); H3's cells have the same
// geometry as this orientation with the GNOMONIC projection
static const char* const kDymaxionLon = "10.5361990754676369448";
static const char* const kDymaxionLat = "64.7000001279348804496";
static const char* const kDymaxionAzimuth = "125.687650893546277353";

using namespace dgg::topo;
using namespace dgg::addtype;

namespace {
std::string coordinateString(long double coordinate)
{
   std::ostringstream text;
   text << std::setprecision(std::numeric_limits<long double>::max_digits10)
        << coordinate;
   return text.str();
}

// the datum choice values, as used by input_datum, output_datum,
// orientation_datum and sphere_radius_type
const std::vector<std::string> datumChoices =
   {"WGS84", "AUTHALIC_SPHERE", "CUSTOM_SPHERE"};

SubOpDGG::DatumMode datumFromString(const std::string& datum)
{
   if (datum == "WGS84") return SubOpDGG::DatumMode::WGS84;
   if (datum == "CUSTOM_SPHERE") return SubOpDGG::DatumMode::CustomSphere;
   return SubOpDGG::DatumMode::AuthalicSphere;
}

const char* datumString(SubOpDGG::DatumMode datum)
{
   switch (datum) {
      case SubOpDGG::DatumMode::WGS84: return "WGS84";
      case SubOpDGG::DatumMode::CustomSphere: return "CUSTOM_SPHERE";
      default: return "AUTHALIC_SPHERE";
   }
}
}

const int SubOpDGG::MAX_DGG_RES = 35;

////////////////////////////////////////////////////////////////////////////////
const DgEllipsoidRF&
SubOpDGG::datumGeoRF (DatumMode datum) const
{
   if (datum == DatumMode::WGS84) return *_pWGS84RF;
   if (datum == gridSphereDatum()) return *_pGeoRF;
   return *_pOtherSphereRF;
}

////////////////////////////////////////////////////////////////////////////////
const DgGeoDegRF&
SubOpDGG::datumDeg (DatumMode datum) const
{
   if (datum == DatumMode::WGS84) return *_pWGS84Deg;
   if (datum == gridSphereDatum()) return *_pDeg;
   return *_pOtherSphereDeg;
}

////////////////////////////////////////////////////////////////////////////////
SubOpDGG::SubOpDGG (OpBasic& op, bool _activate)
   : SubOpBasic (op, _activate),
     dggsType (""), gridTopo (dgg::topo::InvalidTopo),
     gridMetric (dgg::topo::InvalidMetric), aperture (4),
     projType ("ISEA"), res (5), actualRes (5),
     placeRandom (false), orientCenter (false), orientRand (0),
     numGrids (1), curGrid (0), lastGrid (false), sampleCount(0), nSamplePts(0),
     azimuthDegs (0.0), earthRadius (DEFAULT_RADIUS_KM),
     customSphereRadius (DEFAULT_RADIUS_KM), apertureType (""),
     isMixed43 (false), isSuperfund (false), isApSeq (false), isH3Preset (false), h3Geometry (false),
     hierNdxSysType (dgg::addtype::InvalidHierNdxSysType)
{
}

////////////////////////////////////////////////////////////////////////////////
// choose a reference frame based on address type
// returns whether or not seq nums are used
bool
SubOpDGG::addressTypeToRF (DgAddressType type, DgHierNdxSysType hierNdxSysType, DgHierNdxFormType hierNdxForm,
             const DgRFBase** rf, const DgHierNdxSystemRFSBase** hierNdxSysOut, const DgRFBase** chdRF, const DgRFBase** prtRF,
             int forceRes, GeographicBoundary boundary)
{
   const DgIDGGBase* dgg = &this->dgg();
   const DgIDGGBase* chdDgg = &this->chdDgg();
   const DgIDGGBase* prtDgg = this->prtDgg(); // could be null
   if (forceRes >= 0) {
      dgg = &dggs().idggBase(forceRes);
      chdDgg = &dggs().idggBase(forceRes + 1);
      if (forceRes > 0)
         prtDgg = &dggs().idggBase(forceRes - 1);
      else
         prtDgg = nullptr;
   }

   bool seqNum = false;
   if (rf) *rf = nullptr;
   if (chdRF) *chdRF = nullptr;
   if (prtRF) *prtRF = nullptr;
   if (hierNdxSysOut) *hierNdxSysOut = nullptr;

   if (type == HierNdx) {
       DgHierNdxSystemRFSBase* hierNdxSys = DgHierNdxSystemRFSBase::makeSystem(dggs(), hierNdxSysType, hierNdxForm);
       if (hierNdxSysOut) // caller wants the hier ndx system
           *hierNdxSysOut = hierNdxSys;

       if (hierNdxSys) {
          int r = (forceRes >= 0) ? forceRes : dgg->res();
          if (!hierNdxSys->isNdxRes(r)) {
             std::string msg = dgg::addtype::to_string(hierNdxSysType) +
                " has no index at DGGS resolution " + dgg::util::to_string(r) +
                "; indexed DGGS resolutions are " +
                dgg::util::to_string(hierNdxSys->firstNdxRes()) + "-" +
                dgg::util::to_string(hierNdxSys->lastNdxRes());
             if (hierNdxSysType == dgg::addtype::H3)
                msg += " (H3 resolutions 0-15 are DGGS resolutions 2-17)";
             ::report(msg, DgBase::Fatal);
             return seqNum;
          }

          if (rf) *rf = &hierNdxSys->sysRF(r);
          // null if the cell has no indexing child/parent system
          if (chdRF && hierNdxSys->ndxChildRes(r) >= 0)
             *chdRF = &hierNdxSys->sysRF(r + 1);
          if (prtRF && hierNdxSys->ndxParentRes(r) >= 0)
             *prtRF = &hierNdxSys->sysRF(r - 1);
       }
   } else {
      switch (type) {
         case Geo:
         {
            // A geographic frame is resolution independent. Preserve the
            // existing child/parent spherical adapters for legacy callers.
            const DatumMode datum = boundary == GeographicBoundary::Input
                                    ? inputDatumMode : outputDatumMode;
            const bool useDatumFrame = datum != gridSphereDatum();
            const DgRFBase& geographicRF = datumDeg(datum);
            if (rf) *rf = &geographicRF;
            if (chdRF) *chdRF = useDatumFrame
                 ? &geographicRF : static_cast<const DgRFBase*>(&chdDeg());
            if (prtRF) *prtRF = !prtDgg ? nullptr
                 : useDatumFrame
                    ? &geographicRF
                    : static_cast<const DgRFBase*>(prtDeg());
            break;
         }

         case Plane:
            if (rf) *rf = &dgg->planeRF();
            if (chdRF) *chdRF = chdDgg ? &chdDgg->planeRF() : nullptr;
            // prtDgg is null at resolution 0 (no parent)
            if (prtRF) *prtRF = prtDgg ? &prtDgg->planeRF() : nullptr;
            break;

         case ProjTri:
            if (rf) *rf = &dgg->projTriRF();
            if (chdRF) *chdRF = chdDgg ? &chdDgg->projTriRF() : nullptr;
            if (prtRF) *prtRF = prtDgg ? &prtDgg->projTriRF() : nullptr;
            break;

         case Q2DD:
            if (rf) *rf = &dgg->q2ddRF();
            if (chdRF) *chdRF = chdDgg ? &chdDgg->q2ddRF() : nullptr;
            if (prtRF) *prtRF = prtDgg ? &prtDgg->q2ddRF() : nullptr;
            break;

         case Q2DI:
            if (rf) *rf = dgg;
            if (chdRF) *chdRF = chdDgg;
            if (prtRF) *prtRF = prtDgg;
            break;

         case SeqNum:
            seqNum = true;
            if (rf) *rf = dgg;
            if (chdRF) *chdRF = chdDgg;
            if (prtRF) *prtRF = prtDgg;
            break;

         case Vertex2DD:
            if (rf) *rf = &dgg->vertexRF();
            if (chdRF) *chdRF = chdDgg ? &chdDgg->vertexRF() : nullptr;
            if (prtRF) *prtRF = prtDgg ? &prtDgg->vertexRF() : nullptr;
            break;

         case HierNdx: // should be caught above
         case InvalidAddressType:
         default:
            ::report("addressTypeToRF(): invalid address type", DgBase::Fatal);
      }
   }

   return seqNum;

} // void SubOpDGG::addressTypeToRF

////////////////////////////////////////////////////////////////////////////////
// is the H3 hierarchical indexing system used by the grid, or for input or
// output addresses?
bool
SubOpDGG::usesH3Index (void) const
{
   using namespace dgg::addtype;

   return hierNdxSysType == H3 ||
      (op.inOp.inAddType == HierNdx && op.inOp.inHierNdxSysType == H3) ||
      (op.outOp.outAddType == HierNdx && op.outOp.outHierNdxSysType == H3);
}

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::initializeOp (void)
{
   // dggs_type <CUSTOM | SUPERFUND | PLANETRISK | H3 | IGEO7v1 | IGEO7v2 | IGEO7 |
   //            ISEA3HS | ISEA4HS | ISEA7HS | ISEA43HS | ISEA4TS | ISEA4DS |
   //            ISEA3H | ISEA4H | ISEA7H | ISEA43H | ISEA4T | ISEA4D |
   //            IVEA3HS | IVEA4HS | IVEA7HS | IVEA43HS | IVEA4TS | IVEA4DS |
   //            ISEA3HL | ISEA4HL | ISEA7HL | ISEA43HL | ISEA4TL | ISEA4DL |
   //            IVEA3HL | IVEA4HL | IVEA7HL | IVEA43HL | IVEA4TL | IVEA4DL |
   //            ISEA3HSZ | ISEA4HSZ | ISEA7HSZ | IVEA3HSZ | IVEA4HSZ | IVEA7HSZ |
   //            ISEA3HLZ | ISEA4HLZ | ISEA7HLZ | IVEA3HLZ | IVEA4HLZ | IVEA7HLZ |
   //            FULLER3H | FULLER4H | FULLER7H | FULLER43H | FULLER4T | FULLER4D>
   // A trailing S marks a spherical (authalic sphere) preset and a trailing L
   // its ellipsoidal (WGS84) version. ISEA3H ... ISEA4D are aliases for
   // ISEA3HS ... ISEA4DS, kept for backwards compatibility. A final Z marks
   // the hexagon S and L presets that use hierarchical indexing for input,
   // output and internal processing: Z3 for aperture 3, ZORDER for aperture 4
   // and Z7 for aperture 7. IGEO7v1 is another name for ISEA7HSZ (the IGEO7 of
   // version 8.4), and IGEO7v2 is another name for ISEA7HLZ; the two differ
   // only in datum. IGEO7 is an alias for IGEO7v2 (it was IGEO7v1 before
   // version 9.1) and produces a warning. H3 is the H3 grid (gnomonic projection,
   // aperture sequence 4 3 7 7 ... and the DYMAXION orientation); for it
   // dggs_res_spec is the H3 resolution (0 to 15, DGGS resolutions 2 to 17).
   // H3 is only a choice (here, and for the hierarchical indexing system
   // parameters) when DGGRID is built with the H3 library.
   {
      std::vector<std::string> ch = {"CUSTOM", "SUPERFUND", "PLANETRISK", "H3",
        "IGEO7v1", "IGEO7v2", "IGEO7",
        "ISEA3HS", "ISEA4HS", "ISEA7HS", "ISEA43HS", "ISEA4TS", "ISEA4DS",
        "ISEA3H", "ISEA4H", "ISEA7H", "ISEA43H", "ISEA4T", "ISEA4D",
        "IVEA3HS", "IVEA4HS", "IVEA7HS", "IVEA43HS", "IVEA4TS", "IVEA4DS",
        "ISEA3HL", "ISEA4HL", "ISEA7HL", "ISEA43HL", "ISEA4TL", "ISEA4DL",
        "IVEA3HL", "IVEA4HL", "IVEA7HL", "IVEA43HL", "IVEA4TL", "IVEA4DL",
        "ISEA3HSZ", "ISEA4HSZ", "ISEA7HSZ", "IVEA3HSZ", "IVEA4HSZ", "IVEA7HSZ",
        "ISEA3HLZ", "ISEA4HLZ", "ISEA7HLZ", "IVEA3HLZ", "IVEA4HLZ", "IVEA7HLZ",
        "FULLER3H", "FULLER4H", "FULLER7H", "FULLER43H", "FULLER4T", "FULLER4D"};
#ifndef USE_H3
      // H3 is not a choice unless DGGRID is built with the H3 library
      ch.erase(std::remove(ch.begin(), ch.end(), "H3"), ch.end());
#endif
      pList().insertParam("dggs_type", "CUSTOM", ch);
   }


   // dggs_base_poly <ICOSAHEDRON>
   pList().insertParam("dggs_base_poly", "ICOSAHEDRON", {"ICOSAHEDRON"});

   // dggs_topology <HEXAGON | TRIANGLE | DIAMOND>
   pList().insertParam("dggs_topology", "HEXAGON", {"HEXAGON", "TRIANGLE", "DIAMOND"});

   // dggs_proj <ISEA | IVEA | FULLER | GNOMONIC>
   pList().insertParam("dggs_proj", "ISEA", {"ISEA", "IVEA", "FULLER", "GNOMONIC"});

   // dggs_aperture_type <PURE | MIXED43 | SEQUENCE>
   pList().insertParam("dggs_aperture_type", "PURE", {"PURE", "MIXED43", "SEQUENCE"});

   // dggs_aperture < 3 | 4 | 7 >
   pList().insertParam("dggs_aperture", "4", {"3", "4", "7"});

   // dggs_aperture_sequence < apertureSequence >
   pList().insertParam(new DgStringParam("dggs_aperture_sequence",
                     "333333333333"));

/*
   // dggs_aperture <int>
   pList().insertParam(new DgIntParam("dggs_aperture", 4, 3, 7));
*/

   // dggs_num_aperture_4_res
   pList().insertParam(new DgIntParam("dggs_num_aperture_4_res", 0, 0, MAX_DGG_RES));

   // The datums are WGS84 (the ellipsoid), AUTHALIC_SPHERE (the sphere with the
   // WGS84 authalic radius), and CUSTOM_SPHERE (the sphere with radius
   // custom_sphere_radius).

   // sphere_radius_type <AUTHALIC_SPHERE | CUSTOM_SPHERE>
   // the sphere the grid is built on (never the WGS84 ellipsoid itself)
   pList().insertParam("sphere_radius_type", "AUTHALIC_SPHERE",
                       {"AUTHALIC_SPHERE", "CUSTOM_SPHERE"});

   // custom_sphere_radius <long double: km> (1.0 <= v <= 10000.0)
   pList().insertParam(new DgDoubleParam("custom_sphere_radius", DEFAULT_RADIUS_KM,
               1.0, 10000.0));

   // Interpretation of geographic coordinates at the two application boundaries.
   // input_datum, output_datum <WGS84 | AUTHALIC_SPHERE | CUSTOM_SPHERE>
   pList().insertParam("input_datum", "AUTHALIC_SPHERE", datumChoices);
   pList().insertParam("output_datum", "AUTHALIC_SPHERE", datumChoices);

   // Interpretation of orientation values (dggs_vert0_lon/lat, region_center_lon/lat),
   // independent of input_datum/output_datum: a WGS84 orientation_datum is converted
   // to the grid sphere before use and back to WGS84 when echoed in a generated
   // metafile.
   // orientation_datum <WGS84 | AUTHALIC_SPHERE | CUSTOM_SPHERE>
   pList().insertParam("orientation_datum", "AUTHALIC_SPHERE", datumChoices);

   //// specify the position and orientation

   // dggs_orient_specify_type <RANDOM | SPECIFIED | REGION_CENTER>
   pList().insertParam("dggs_orient_specify_type", "SPECIFIED",
                       {"RANDOM", "SPECIFIED", "REGION_CENTER"});

   // dggs_num_placements <int> (v >= 1)
   pList().insertParam(new DgIntParam("dggs_num_placements", 1, 1, INT_MAX, true));

   // dggs_orient_rand_seed <unsigned long int int>
   pList().insertParam(new DgULIntParam("dggs_orient_rand_seed", 77316727, 0,
                     ULONG_MAX, true));

   // dggs_orient_preset <NONE | ISEA | ISEAL | DYMAXION>
   // a named orientation: sets dggs_vert0_lon, dggs_vert0_lat, dggs_vert0_azimuth
   // and orientation_datum (explicitly set values still take precedence)
   pList().insertParam("dggs_orient_preset", "NONE", {"NONE", "ISEA", "ISEAL", "DYMAXION"});

   // dggs_vert0_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   pList().insertParam(new DgDoubleParam("dggs_vert0_lon", 11.25, -180.0, 180.0));

   // dggs_vert0_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   // default atan(phi) puts the north pole on an icosahedron edge midpoint
   pList().insertParam(new DgDoubleParam("dggs_vert0_lat", M_ICOSA_VERT0_LAT_DEG,
                                         -90.0, 90.0));

   // dggs_vert0_azimuth <long double: decimal degrees> (0.0 <= v < 360.0)
   pList().insertParam(new DgDoubleParam("dggs_vert0_azimuth", 0.0, 0.0, 360.0));

   // region_center_lon <long double: decimal degrees> (-180.0 <= v <= 180.0)
   pList().insertParam(new DgDoubleParam("region_center_lon", 0.0, -180.0, 180.0));

   // region_center_lat <long double: decimal degrees> (-90.0 <= v <= 90.0)
   pList().insertParam(new DgDoubleParam("region_center_lat", 0.0, -90.0, 90.0));

   // dggs_res_specify_type <SPECIFIED | CELL_AREA | INTERCELL_DISTANCE>
   pList().insertParam("dggs_res_specify_type", "SPECIFIED",
                       {"SPECIFIED", "CELL_AREA", "INTERCELL_DISTANCE"});

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

   // hier_indexing_system_type <ZORDER | Z3 | Z7 | H3 | NONE>
   auto hierChoices = dgg::addtype::hierNdxSysTypeChoices();
   hierChoices.push_back("NONE");
#ifndef USE_H3
   // H3 is not a choice unless DGGRID is built with the H3 library
   hierChoices.erase(std::remove(hierChoices.begin(), hierChoices.end(), "H3"),
                     hierChoices.end());
#endif
   pList().insertParam("hier_indexing_system_type", "NONE", hierChoices);

   // z3_invalid_digit < 0 | 1 | 2 | 3 >
   // default changed to "3" in version 9.0b
   pList().insertParam("z3_invalid_digit", "3", {"0", "1", "2", "3"});

   return 0;

} // int SubOpDGG::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpDGG::setupOp (void)
{
   /////// fill state variables from the parameter list //////////

   // setup preset DGGS (if any)
   std::string tmp;
   getParamValue(pList(), "dggs_type", tmp, false);
   std::string tmplc = toLower(tmp);

#ifdef USE_H3
   // h3_boundary_def H3 is accepted only for dggs_type H3
   {
      std::string h3Bnd;
      getParamValue(pList(), "h3_boundary_def", h3Bnd, false);
      if (toLower(h3Bnd) == "h3" && tmplc != "h3")
         ::report("h3_boundary_def H3 requires dggs_type H3", DgBase::Fatal);
   }
#endif
   if (tmplc != "custom") {
      // IGEO7v1 and IGEO7v2 (and its alias IGEO7) are the ISEA7HSZ and ISEA7HLZ
      // presets
      std::string gridName = tmplc;
      if (tmplc == "igeo7v1")
         gridName = "isea7hsz";
      else if (tmplc == "igeo7v2" || tmplc == "igeo7")
         gridName = "isea7hlz";

      if (tmplc == "igeo7")
         ::report("dggs_type IGEO7 is now an alias for IGEO7v2 (ISEA projection "
                  "on the WGS84 ellipsoid); before version 9.1 it was "
                  "IGEO7v1 (ISEA projection on the authalic sphere). "
                  "Use dggs_type IGEO7v1 to get the previous IGEO7 grid.",
                  DgBase::Warning);

      // A final Z (e.g. ISEA7HSZ) marks a preset with hierarchical indexing for
      // all purposes: the grid's own hierarchical indexing system and HIERNDX
      // input and output addresses. Strip it first.
      bool hierNdx = gridName.back() == 'z';
      if (hierNdx)
         gridName.pop_back();

      // Spherical presets end in S (e.g. ISEA3HS) and ellipsoidal ones in L
      // (e.g. ISEA3HL); strip the suffix to get the grid name. The legacy
      // ISEA3H ... ISEA4D names have no suffix and are the spherical presets.
      const bool ellipsoidal = gridName.back() == 'l';
      if (ellipsoidal || gridName.back() == 's')
         gridName.pop_back();

      // the hierarchical indexing system of a Z preset, by aperture
      std::string hierNdxSys;

      // these params are common to all presets
      pList().setPresetParam("dggs_base_poly", "ICOSAHEDRON");
      pList().setPresetParam("dggs_orient_specify_type", "SPECIFIED");
      pList().setPresetParam("dggs_num_placements", "1");

      // All presets are processed on the authalic sphere. The ellipsoidal
      // presets use WGS84 geographic input and output and the ISEAL
      // orientation; all others use the authalic sphere for input and output.
      // The FULLER presets use the DYMAXION orientation and all other
      // non-ellipsoidal presets use the ISEA orientation.
      const char* datum = ellipsoidal ? "WGS84" : "AUTHALIC_SPHERE";
      pList().setPresetParam("sphere_radius_type", "AUTHALIC_SPHERE");
      pList().setPresetParam("input_datum", datum);
      pList().setPresetParam("output_datum", datum);
      // also sets orientation_datum
      const bool fuller = !gridName.compare(0, 6, "fuller");
      isH3Preset = (gridName == "h3");
      pList().setPresetParam("dggs_orient_preset",
                  ellipsoidal ? "ISEAL" :
                  ((fuller || isH3Preset) ? "DYMAXION" : "ISEA"));
      pList().setPresetParam("dggs_res_specify_type", "SPECIFIED");
      pList().setPresetParam("dggs_res_spec", "9");

      if (gridName == "superfund") {
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "FULLER");
         pList().setPresetParam("dggs_num_aperture_4_res", "2");
         pList().setPresetParam("dggs_aperture_type", "MIXED43");
         pList().setPresetParam("output_cell_label_type", "SUPERFUND", true);
      } else if (isH3Preset) {
         // H3 cell geometry: gnomonic projection, DYMAXION orientation and
         // apertures 4, 3, then 7s: DGGS resolution 2 is H3 resolution 0
         // (the 122 base cells). DGGS resolution 18 is the child grid of H3
         // resolution 15.
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "GNOMONIC");
         pList().setPresetParam("dggs_aperture_type", "SEQUENCE");
         pList().setPresetParam("dggs_aperture_sequence", "437777777777777777");
         // dggs_res_spec is the H3 resolution (see below)
         pList().setPresetParam("dggs_res_spec", "9");
         // (dggs_type H3 is only a choice when built with H3)
         hierNdxSys = "H3";
         hierNdx = true;
#ifdef USE_H3
         // cell boundaries as H3 generates them; H3's great circle edges need
         // no densification
         pList().setPresetParam("h3_boundary_def", "H3");
         pList().setPresetParam("densification", "0");
         // clip cells likewise; clip_cell_densification belongs to
         // GENERATE_GRID only
         pList().setPresetParam("clip_cell_densification", "0", true);
         // an H3 index is 15 characters (the default field is 11)
         pList().setPresetParam("shapefile_id_field_length", "15");
#endif
      } else if (gridName == "planetrisk") {
         pList().setPresetParam("dggs_topology", "HEXAGON");
         pList().setPresetParam("dggs_proj", "ISEA");
         pList().setPresetParam("dggs_aperture_type", "SEQUENCE");
         pList().setPresetParam("dggs_aperture_sequence", "43334777777777777777777");
         pList().setPresetParam("dggs_res_spec", "11");
      } else {
         // get the topology
         char topo = gridName[gridName.length() - 1];
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

         // get the projection from the preset name prefix
         static const struct { const char* prefix; const char* proj; }
            presetProjs[] = { {"isea", "ISEA"}, {"ivea", "IVEA"},
                              {"fuller", "FULLER"} };
         int projLen = -1;
         for (const auto& pp : presetProjs) {
            const std::string prefix(pp.prefix);
            if (!gridName.compare(0, prefix.length(), prefix)) {
               pList().setPresetParam("dggs_proj", pp.proj);
               projLen = (int) prefix.length();
               break;
            }
         }

         if (projLen < 0)
            ::report("SubOpDGG::setupOp(): dggs_type " + tmp +
                     " has no known projection prefix", DgBase::Fatal);

         // get the aperture
         const std::string ap =
               gridName.substr(projLen, gridName.length() - projLen - 1);
         if (ap == "43") {
            pList().setPresetParam("dggs_aperture_type", "MIXED43");
         } else {
            pList().setPresetParam("dggs_aperture_type", "PURE");
            pList().setPresetParam("dggs_aperture", ap);

            if (ap == "3")
               hierNdxSys = "Z3";
            else if (ap == "4")
               hierNdxSys = "ZORDER";
            else if (ap == "7")
               hierNdxSys = "Z7";
         }
      }

      if (hierNdx) {
         if (hierNdxSys.empty())
            ::report("SubOpDGG::setupOp(): dggs_type " + tmp +
                     " has no hierarchical indexing system", DgBase::Fatal);

         // Z3, ZORDER and Z7 INT64 indexes are printed as 16 hex digits;
         // H3's canonical indexes are 15 digits.
         if (!isH3Preset)
            pList().setPresetParam("shapefile_id_field_length", "16");
         pList().setPresetParam("hier_indexing_system_type", hierNdxSys);
         pList().setPresetParam("input_address_type", "HIERNDX", true);
         pList().setPresetParam("input_hier_ndx_system", hierNdxSys, true);
         pList().setPresetParam("input_hier_ndx_form", "INT64", true);
         pList().setPresetParam("output_cell_label_type", "OUTPUT_ADDRESS_TYPE", true);
         pList().setPresetParam("output_address_type", "HIERNDX", true);
         pList().setPresetParam("output_hier_ndx_system", hierNdxSys, true);
         pList().setPresetParam("output_hier_ndx_form", "INT64", true);
      }
   }

   // setup preset orientation (if any); applied after the dggs_type preset,
   // which may choose one
   std::string orientPreset;
   getParamValue(pList(), "dggs_orient_preset", orientPreset, false);
   const std::string orientPresetlc = toLower(orientPreset);
   if (orientPresetlc != "none") {
      // DYMAXION is Buckminster Fuller's Dymaxion icosahedron, placed with
      // Robert W. Gray's high-precision vertex coordinates (graypr.c,
      // http://www.rwgrayprojects.com/rbfnotes/maps/docs/graypr.c):
      //   - vert0 is Gray's vertex 1 (in the Norwegian Sea);
      //   - the azimuth is the great-circle bearing from it to Gray's vertex 3
      //     (in the Arabian Sea);
      // which numbers DGGRID's vertices, and therefore its faces, in the same
      // order as H3: DGGRID vertex v is the H3 pentagon base cell
      // { 4, 49, 24, 14, 38, 58, 83, 63, 72, 97, 107, 117 }[v].
      static const struct {
         const char* name; const char* lon; const char* lat; const char* azimuth;
         const char* datum;
      } orientPresets[] = {
         { "isea",  "11.25", nullptr, "0.0", "AUTHALIC_SPHERE" },
         { "iseal", "11.20", nullptr, "0.0", "AUTHALIC_SPHERE" },
         { "dymaxion", kDymaxionLon, kDymaxionLat, kDymaxionAzimuth,
           "AUTHALIC_SPHERE" }
      };

      bool found = false;
      for (const auto& op : orientPresets) {
         if (orientPresetlc == op.name) {
            pList().setPresetParam("dggs_vert0_lon", op.lon);
            // 21 significant digits round-trip an 80-bit long double
            pList().setPresetParam("dggs_vert0_lat", op.lat ? op.lat :
                          dgg::util::to_string(M_ICOSA_VERT0_LAT_DEG, "%.21Lg"));
            pList().setPresetParam("dggs_vert0_azimuth", op.azimuth);
            pList().setPresetParam("orientation_datum", op.datum);
            found = true;
            break;
         }
      }

      if (!found)
         ::report("SubOpDGG::setupOp(): unknown dggs_orient_preset " + orientPreset,
                  DgBase::Fatal);
   }

   std::string gridTopoStr = "";
   getParamValue(pList(), "dggs_topology", gridTopoStr, false);
   gridTopo = dgg::topo::stringToGridTopology(gridTopoStr);

/* metric not exposed to user yet; D8 is broken
   std::string gridMetricStr = "";
   getParamValue(pList(), "dggs_metric", gridMetricStr, false);
   gridMetric = std::stringToGridMetric(gridMetricStr);
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
      std::string tmp;
      getParamValue(pList(), "dggs_aperture_sequence", tmp, false);
      apSeq = DgApSeq(tmp);
   } else {
      getParamValue(pList(), "dggs_aperture", tmp, false);
      aperture = dgg::util::from_string<int>(tmp);
   }

   getParamValue(pList(), "dggs_num_placements", numGrids, false);
   getParamValue(pList(), "dggs_proj", projType, false);
   getParamValue(pList(), "dggs_vert0_azimuth", azimuthDegs, false);

   std::string datumName;
   getParamValue(pList(), "sphere_radius_type", datumName, false);
   sphereRadiusType = datumFromString(datumName);
   getParamValue(pList(), "input_datum", datumName, false);
   inputDatumMode = datumFromString(datumName);
   getParamValue(pList(), "output_datum", datumName, false);
   outputDatumMode = datumFromString(datumName);
   getParamValue(pList(), "orientation_datum", datumName, false);
   orientationDatumMode = datumFromString(datumName);
   for (DatumMode datum : {sphereRadiusType, inputDatumMode, outputDatumMode,
                           orientationDatumMode}) {
      if (datum == DatumMode::CustomSphere) {
         getParamValue(pList(), "custom_sphere_radius", customSphereRadius, false);
         break;
      }
   }

   // WGS84 maps equal-area onto its authalic sphere, so the grid must be built
   // on that sphere
   if ((inputWGS84() || outputWGS84() || orientationWGS84()) &&
         gridSphereDatum() != DatumMode::AuthalicSphere)
      ::report("input_datum/output_datum/orientation_datum WGS84 requires "
               "sphere_radius_type AUTHALIC_SPHERE", DgBase::Fatal);

   earthRadius = gridSphereDatum() == DatumMode::CustomSphere ?
                 customSphereRadius : DgWGS84RF::canonicalAuthalicRadiusKM();

   long double lon0, lat0;
   getParamValue(pList(), "dggs_vert0_lon", lon0, false);
   getParamValue(pList(), "dggs_vert0_lat", lat0, false);
   // A partially specified pair uses the other parameter's current default or
   // preset, then the complete pair is interpreted in the selected orientation
   // model. Built-in and preset placement is already spherical and stays untouched.
   if (orientationWGS84() && (pList().getParam("dggs_vert0_lon", false)->isUserSet() ||
                        pList().getParam("dggs_vert0_lat", false)->isUserSet()))
      lat0 = DgAuthalic::geodeticToAuthalicLatitude(lat0 * M_PI_180) * M_180_PI;
   vert0 = DgGeoCoord(lon0, lat0, false);

   if (tmp == "SUPERFUND") {
      isSuperfund = true;

      if (!isMixed43)
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_aperture_type of MIXED43", DgBase::Fatal);

      if (numAp4 != 2)
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_num_aperture_4_res of 2", DgBase::Fatal);

      std::string resType;
      getParamValue(pList(), "dggs_res_specify_type", resType, false);

      if (resType != std::string("SPECIFIED"))
         ::report("SubOpDGG::setupOp() Superfund grid requires "
             "dggs_res_specify_type of SPECIFIED", DgBase::Fatal);

      getParamValue(pList(), "dggs_res_spec", res, false);
      sfRes = res;
      actualRes = res = sfRes2actualRes(sfRes);
   } else if (isH3Preset) {
      // for dggs_type H3 only, dggs_res_spec is the H3 resolution; DGGS
      // resolution 2 is H3 resolution 0
      std::string resType;
      getParamValue(pList(), "dggs_res_specify_type", resType, false);
      if (resType != std::string("SPECIFIED"))
         ::report("SubOpDGG::setupOp() dggs_type H3 requires "
             "dggs_res_specify_type of SPECIFIED", DgBase::Fatal);

      int h3Res;
      getParamValue(pList(), "dggs_res_spec", h3Res, false);
      if (h3Res < 0 || h3Res > 15)
         ::report("SubOpDGG::setupOp() dggs_type H3 requires dggs_res_spec "
             "(the H3 resolution) from 0 to 15", DgBase::Fatal);

      sfRes = 0;
      actualRes = res = h3Res + 2;
   } else { // not superfund
      sfRes = 0;
      determineRes();
      actualRes = res;
   }

   std::string dummy;
   getParamValue(pList(), "dggs_orient_specify_type", dummy, false);
   dummy = dgg::util::toUpper(dummy);
   if (dummy == std::string("SPECIFIED"))
      placeRandom = false;
   else if (dummy == std::string("REGION_CENTER")) {
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

   // hierarchical indexing type
   getParamValue(pList(), "hier_indexing_system_type", dummy, false);
   dummy = dgg::util::toUpper(dummy);
   hierNdxSysType = dgg::addtype::stringToHierNdxSysType(dummy);
   if (hierNdxSysType != dgg::addtype::InvalidHierNdxSysType) {
      if (hierNdxSysType == dgg::addtype::Z7) {
         if (apertureType != "PURE" || aperture != 7)
            ::report("hier_indexing_system_type Z7 "
                     "requires a pure aperture 7 DGGS", DgBase::Fatal);
      } else if (hierNdxSysType == dgg::addtype::Z3) {
          if (apertureType != "PURE" || aperture != 3)
              ::report("hier_indexing_system_type Z3 "
                       "requires a pure aperture 3 DGGS", DgBase::Fatal);
      } else if (hierNdxSysType == dgg::addtype::ZOrder) {
          if (apertureType != "PURE" || (aperture != 3 && aperture != 4))
              ::report("hier_indexing_system_type ZOrder "
                       "requires a pure aperture 3 or 4 DGGS", DgBase::Fatal);
       } else if (hierNdxSysType == dgg::addtype::H3) {
          if (gridTopo != Hexagon)
             ::report("H3 requires HEXAGON topology", DgBase::Fatal);
          // The library factory validates the constructed DGGS and its exact
          // transition apertures, for both internal and on-demand systems.
       } else {
         ::report("SubOpDGG::setupOp() invalid hier_indexing_system_type", DgBase::Fatal);
      }
   }

   // get the digit to fill unused resolutions in Z3
   // this is used by all Z3 values, whether input, output, or hier system
   getParamValue(pList(), "z3_invalid_digit", tmp, false);
   z3invalidDigit = dgg::util::from_string<int>(tmp);
   DgZ3System::defaultInvalidDigit = z3invalidDigit;

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

//cout << "YYY " << curGrid << " " << numGrids << " " << lastGrid << std::endl;

   curGrid++;
   if (curGrid == numGrids) lastGrid = true;

//cout << "ZZZ " << curGrid << " " << numGrids << " " << lastGrid << std::endl;

   if (curGrid == 1) {
      // each frame is named for its datum
      _pGeoRF = DgGeoSphRF::makeRF(net0(), datumString(gridSphereDatum()),
                                   earthRadius);
      if (inputWGS84() || outputWGS84()) {
         _pWGS84RF = DgWGS84RF::makeRF(net0());
         Dg2WayAuthalicConverter(*_pWGS84RF, *_pGeoRF);
      }

      // a boundary on the other sphere datum has its own frame; coordinates
      // carry over unchanged between spheres
      for (DatumMode datum : {inputDatumMode, outputDatumMode}) {
         if (datum == DatumMode::WGS84 || datum == gridSphereDatum() ||
             _pOtherSphereRF)
            continue;

         const long double radius = datum == DatumMode::CustomSphere ?
               customSphereRadius : DgWGS84RF::canonicalAuthalicRadiusKM();
         _pOtherSphereRF = DgGeoSphRF::makeRF(net0(), datumString(datum), radius);
         Dg2WaySphereConverter(*_pOtherSphereRF, *_pGeoRF);
         _pOtherSphereDeg = DgGeoSphDegRF::makeRF(*_pOtherSphereRF,
                                    _pOtherSphereRF->name() + "Deg");
      }
   }

   orientGrid();

   _pDGGS  = DgIDGGSBase::makeRF(net0(), geoRF(), vert0,
             azimuthDegs, aperture, actualRes+2, gridTopo,
             gridMetric, "IDGGS", projType, isApSeq, apSeq,
             isMixed43, numAp4, isSuperfund, hierNdxSysType);

   _pDGG = &dggs().idggBase(actualRes);

   // a hierarchical index may not cover every resolution
   const DgHierNdxSystemRFSBase* ndxSys = dggs().hierNdxSystem();
   if (ndxSys && !ndxSys->isNdxRes(actualRes)) {
      std::string msg = "hier_indexing_system_type " +
         dgg::addtype::to_string(hierNdxSysType) +
         " has no index at DGGS resolution " + dgg::util::to_string(actualRes) +
         "; indexed DGGS resolutions are " +
         dgg::util::to_string(ndxSys->firstNdxRes()) + "-" +
         dgg::util::to_string(ndxSys->lastNdxRes());
      if (hierNdxSysType == dgg::addtype::H3)
         msg += " (H3 resolutions 0-15 are DGGS resolutions 2-17)";
      ::report(msg, DgBase::Fatal);
   }

   // H3 indexes are valid H3 cells whatever the grid, but the cells themselves
   // are H3's only for the GNOMONIC projection and the DYMAXION orientation
   static bool h3GeometryWarned = false;
   {
      const long double tol = 1e-12L;
      h3Geometry = (projType == "GNOMONIC" &&
         fabsl(vert0.lonDegs() - strtold(kDymaxionLon, nullptr)) < tol &&
         fabsl(vert0.latDegs() - strtold(kDymaxionLat, nullptr)) < tol &&
         fabsl(azimuthDegs - strtold(kDymaxionAzimuth, nullptr)) < tol);
   }
   if (!h3GeometryWarned && usesH3Index()) {
      if (!h3Geometry) {
         h3GeometryWarned = true;
         ::report("H3 indexes are topologically valid H3 cells, but cell "
                  "geometry matches H3 only with dggs_proj GNOMONIC and "
                  "dggs_orient_preset DYMAXION (dggs_type H3)",
                  DgBase::Warning);
      }
   }

   // child dgg
   _pChdDgg = &dggs().idggBase(actualRes + 1);

   // parent dgg (for hierarchical indexing); null at resolution 0
   _pPrtDgg = ((actualRes > 0) ? &dggs().idggBase(actualRes - 1) : nullptr);

   // set-up to convert to degrees
   _pDeg = DgGeoSphDegRF::makeRF(geoRF(), _pGeoRF->name() + "Deg");
   if (_pWGS84RF && !_pWGS84Deg)
      _pWGS84Deg = DgGeoDegRF::makeRF(*_pWGS84RF, "WGS84Deg");
   _pChdDeg = DgGeoSphDegRF::makeRF(_pChdDgg->geoRF(), _pChdDgg->geoRF().name() + "Deg");
   if (_pPrtDgg)
      _pPrtDeg = DgGeoSphDegRF::makeRF(_pPrtDgg->geoRF(), _pPrtDgg->geoRF().name() + "Deg");
   else
      _pPrtDeg = nullptr;

   return 0;

} // SubOpDGG::executeOp

////////////////////////////////////////////////////////////////////////////////
void
SubOpDGG::determineRes (void)
{
   int maxRes = (apertureType != "SEQUENCE") ? MAX_DGG_RES : apSeq.lastRes();

   // get the parameters

   std::string resType;
   getParamValue(pList(), "dggs_res_specify_type", resType, false);

   if (resType == std::string("SPECIFIED")) {
      getParamValue(pList(), "dggs_res_spec", res, false);
   } else {
      bool area;
      long double value = 0;
      if (resType == std::string("CELL_AREA")) {
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
             projType, isApSeq, apSeq, isMixed43, numAp4, isSuperfund, hierNdxSysType);

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

      dgcout << "** choosing grid resolution: " << res << std::endl;
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
      pList().setParam("dggs_vert0_lon", coordinateString(vert0.lonDegs()));
      const long double printedLat = orientationWGS84()
          ? DgAuthalic::authalicToGeodeticLatitude(vert0.lat()) * M_180_PI
          : vert0.latDegs();
      pList().setParam("dggs_vert0_lat", coordinateString(printedLat));
      // Print the placement in the selected orientation model so a generated
      // metafile replays the same placement when read back in.
      pList().setParam("orientation_datum", datumString(orientationDatumMode));
      pList().setParam("dggs_vert0_azimuth", coordinateString(azimuthDegs));

      dgcout << "Grid " << curGrid <<
           " #####################################################" << std::endl;
      dgcout << "grid #" << curGrid << " orientation randomized to: " << std::endl;
      dgcout << pList() << std::endl;

   } else if (orientCenter && curGrid == 1) {

      DgRFNetwork netc;
      const DgGeoSphRF& geoRF = *(DgGeoSphRF::makeRF(netc, "GS0", earthRadius));

      long double lonc = 0.0, latc = 0.0;
      getParamValue(pList(), "region_center_lon", lonc, false);
      getParamValue(pList(), "region_center_lat", latc, false);
      if (orientationWGS84())
         latc = DgAuthalic::geodeticToAuthalicLatitude(latc * M_PI_180) * M_180_PI;

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
