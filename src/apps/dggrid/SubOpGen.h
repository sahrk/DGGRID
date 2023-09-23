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
// SubOpGen.h: this file defines the generate grid operation
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPGEN_H
#define SUBOPGEN_H

#include <set>
#include <map>
#include <limits>
// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL
#include <gdal.h>
#include <ogrsf_frmts.h>
#endif
#include "clipper.hpp"
#include <dglib/DgIVec2D.h>
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgInShapefileAtt.h>
#include "SubOpBasicMulti.h"

class DgIDGGSBase;
class DgIDGGBase;
class DgContCartRF;
class DgDiscRF2D;
class DgIVec2D;

struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Data types used by the grid generation operation.

////////////////////////////////////////////////////////////////////////////////
#ifdef USE_GDAL
struct DgClippingHole {

   // is projection quad gnomonic or quad snyder?
   bool isGnomonic;

   // the hole
   OGRPolygon hole;
};
#endif

////////////////////////////////////////////////////////////////////////////////
// clipper region intersection with quads in quad Snyder space with holes
struct DgClippingPoly {

      // the exterior polygon(s) for clipper intersection
      ClipperLib::Paths exterior;

#ifdef USE_GDAL
      // the holes for gdal containment
      vector<DgClippingHole> holes;
#endif

};

////////////////////////////////////////////////////////////////////////////////
class DgQuadClipRegion {

   public:

      DgQuadClipRegion (void)
           : isQuadUsed_ (false), gnomProj_ (0),
	     overI_ (false), overJ_ (false),
             minx_ (numeric_limits<long double>::max()),
             miny_ (numeric_limits<long double>::max()),
	     maxx_ (numeric_limits<long double>::min()),
             maxy_ (numeric_limits<long double>::min()) { }

     ~DgQuadClipRegion (void) { }

      vector<DgClippingPoly>& clpPolys (void) { return clpPolys_; }
      vector < set<DgDBFfield> >& polyFields (void) { return polyFields_; }

      set<DgIVec2D>& points (void) { return points_; }
      map<DgIVec2D, set<DgDBFfield> >& ptFields (void) { return ptFields_; }

      int quadNum (void) const { return quadNum_; }
      void setQuadNum (int q) { quadNum_ = q; }

      bool isQuadUsed (void) const { return isQuadUsed_; }
      void setIsQuadUsed (bool isQuadUsedIn) { isQuadUsed_ = isQuadUsedIn; }

      const DgIVec2D& offset (void) const { return offset_; }
      void setOffset (const DgIVec2D& offsetIn) { offset_ = offsetIn; }

      const DgIVec2D& upperRight (void) const { return upperRight_; }
      void setUpperRight (const DgIVec2D& upperRightIn)
             { upperRight_ = upperRightIn; }

      const DgProjGnomonicRF& gnomProj (void) const { return *gnomProj_; }
      void setGnomProj (const DgProjGnomonicRF* gnomProjIn)
                                  { gnomProj_ = gnomProjIn; }

      ClipperLib::Paths& gnomBndry (void) { return gnomBndry_; }

      // bounding box boundaries
      long double minx (void) const { return minx_; }
      long double miny (void) const { return miny_; }
      long double maxx (void) const { return maxx_; }
      long double maxy (void) const { return maxy_; }

      void setMinx (long double minx) { minx_ = minx; }
      void setMiny (long double miny) { miny_ = miny; }
      void setMaxx (long double maxx) { maxx_ = maxx; }
      void setMaxy (long double maxy) { maxy_ = maxy; }

      bool overI (void) const { return overI_; }
      bool overJ (void) const { return overJ_; }

      void setOverI (const bool overIIn) { overI_ = overIIn; }
      void setOverJ (const bool overJIn) { overJ_ = overJIn; }

   private:

      int quadNum_;

      bool isQuadUsed_; // does input intersect this quad?

      vector<DgClippingPoly> clpPolys_; // clipper region intersection with
                               // quad with holes in quad Snyder space

      vector < set<DgDBFfield> > polyFields_; // shapefile attribute fields

      set<DgIVec2D> points_; // points that fall on this quad

      map<DgIVec2D, set<DgDBFfield> > ptFields_; // shapefile attribute fields

      const DgProjGnomonicRF* gnomProj_; // gnomonic proj centered on this quad

      ClipperLib::Paths gnomBndry_; // quad boundary in gnomonic

      DgIVec2D offset_; // offset of min (i, j)
      DgIVec2D upperRight_; // (maxi, maxj) relative to offset

      bool overI_; // is our maxi > maxI (quad overage)
      bool overJ_; // is our maxj > maxJ (quad overage)

      // bounding box boundaries
      long double minx_;
      long double miny_;
      long double maxx_;
      long double maxy_;
};

////////////////////////////////////////////////////////////////////////////////
// DgEvalData class for Superfund
class DgEvalData {

   public:

      const DgIDGGBase& dgg;
      const DgContCartRF& cc1;
      const DgDiscRF2D& grid;
      DgQuadClipRegion& clipRegion;
      set<DgIVec2D>& overageSet;
      map<DgIVec2D, set<DgDBFfield> > overageFields;
      const DgContCartRF& deg;
      const DgIVec2D& lLeft;
      const DgIVec2D& uRight;

      DgEvalData (const DgIDGGBase& dggIn,
               const DgContCartRF& cc1In, const DgDiscRF2D& gridIn,
               DgQuadClipRegion& clipRegionIn, set<DgIVec2D>& overageSetIn,
               map<DgIVec2D, set<DgDBFfield> >& overageFieldsIn,
               const DgContCartRF& degIn, const DgIVec2D& lLeftIn,
               const DgIVec2D& uRightIn)
      : dgg (dggIn), cc1 (cc1In), grid (gridIn),
               clipRegion (clipRegionIn), overageSet (overageSetIn),
               overageFields (overageFieldsIn),
               deg (degIn), lLeft (lLeftIn), uRight (uRightIn)
        { }
};

////////////////////////////////////////////////////////////////////////////////
struct SubOpGen : public SubOpBasicMulti {

   SubOpGen (OpBasic& op, bool activate = true);

   // virtual methods
   virtual int initializeOp (void);
   virtual int setupOp (void);
   virtual int cleanupOp (void);
   virtual int executeOp (void);

   // internal helper methods
   void outputStatus (bool force = false);
   void genGrid (void);
   bool evalCell (DgEvalData* data, DgIVec2D& add2D);
   bool evalCell (const DgIDGGBase& dgg, const DgContCartRF& cc1,
                  const DgDiscRF2D& grid, DgQuadClipRegion& clipRegion,
                  const DgIVec2D& add2D);
   ClipperLib::Paths* intersectPolyWithQuad (const DgPolygon& v, DgQuadClipRegion& clipRegion);
   void processOneClipPoly (DgPolygon& polyIn, const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], DgInShapefileAtt* pAttributeFile);
   void createClipRegions (const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], set<DgIVec2D> overageSet[],
             map<DgIVec2D, set<DgDBFfield> > overageFields[]);

   // the parameters
   bool wholeEarth;       // generate entire grid?
   bool regionClip;       // whether user wants to generate using regions
   bool seqToPoly;        // whether user wants polys from seqnum
   bool indexToPoly;      // whether user wants polys from any index type
   bool pointClip;        // whether user wants to generate using points
   bool cellClip;         // whether user wants to generate using coarse cells
   bool useGDAL;          // use GDAL for either input or output
   bool clipAIGen;        // clip using AIGen files (or Shapefiles)
   bool clipGDAL;         // clip using GDAL files
   bool clipShape;        // clip using Shapefiles
   vector<string> regionFiles;
   int clipCellRes;       // resolution of the clipping cell indexes
   string clipCellsStr;   // input line of coarse clipping cells
   int nClipCellDensify;  // number of points-per-edge of densification for clipping cells
   //bool clipRandPts;      // clip randpts to polys
   long double nudge;     // adjustment for quad intersection consistency

   bool doPointInPoly;                // perform pt-in-poly intersection
   bool doPolyIntersect;              // perform poly intersection
   unsigned long int clipperFactor;   // clipper scaling factor
   long double invClipperFactor;      // 1.0L / clipper scaling factor
   bool useHoles;                     // handle holes in clipping polygons
   long double geoDens;               // max arc length in radians
};

////////////////////////////////////////////////////////////////////////////////

#endif
