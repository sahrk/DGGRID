/*******************************************************************************
    Copyright (C) 2018 Kevin Sahr

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
// gridgen.h: gridgen class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef GRIDGEN_H
#define GRIDGEN_H

#include <set>
#include <map>
#include <limits>
#include "clipper.hpp"
#include "DgIVec2D.h"
#include "DgProjGnomonicRF.h"
#include "DgInShapefileAtt.h"

class GridGenParam;
class DgIDGGSBase;
class DgIDGGBase;
class DgContCartRF;
class DgDiscRF2D;
class DgContCartRF;

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

      vector<ClipperLib::Paths>& clpPolys (void) { return clpPolys_; }
      vector < set<DgDBFfield> >& polyFields (void) { return polyFields_; }

      set<DgIVec2D>& points (void) { return points_; }
      map<DgIVec2D, set<DgDBFfield> >& ptFields (void) { return ptFields_; }

      bool isQuadUsed (void) const { return isQuadUsed_; }
      void setIsQuadUsed (bool isQuadUsedIn) { isQuadUsed_ = isQuadUsedIn; }

      const DgIVec2D& offset (void) const { return offset_; }
      void setOffset (const DgIVec2D& offsetIn) { offset_ = offsetIn; }

      const DgIVec2D& upperRight (void) const { return upperRight_; }
      void setUpperRight (const DgIVec2D& upperRightIn) 
             { upperRight_ = upperRightIn; }

      const DgProjGnomonicRF& gnomProj (void) const { return *gnomProj_; }
      void setGnomProj (DgProjGnomonicRF* gnomProjIn) 
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

      bool isQuadUsed_; // indicates which quads intersect the region
      
      vector<ClipperLib::Paths> clpPolys_; // clipper region intersection with 
                               // quads in quad Snyder space

      vector < set<DgDBFfield> > polyFields_; // shapefile attribute fields

      set<DgIVec2D> points_; // points that fall on this quad

      map<DgIVec2D, set<DgDBFfield> > ptFields_; // shapefile attribute fields

      DgProjGnomonicRF* gnomProj_; // gnomonic proj centered on this quad

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

      GridGenParam& dp;
      const DgIDGGBase& dgg;
      const DgContCartRF& cc1;
      const DgDiscRF2D& grid; 
      DgQuadClipRegion& clipRegion;
      set<DgIVec2D>& overageSet; 
      map<DgIVec2D, set<DgDBFfield> > overageFields;
      const DgContCartRF& deg;
      const DgIVec2D& lLeft;
      const DgIVec2D& uRight;

      DgEvalData (GridGenParam& dpIn, const DgIDGGBase& dggIn, 
               const DgContCartRF& cc1In, const DgDiscRF2D& gridIn, 
               DgQuadClipRegion& clipRegionIn, set<DgIVec2D>& overageSetIn,
               map<DgIVec2D, set<DgDBFfield> >& overageFieldsIn,
               const DgContCartRF& degIn, const DgIVec2D& lLeftIn, 
               const DgIVec2D& uRightIn)
      : dp (dpIn), dgg (dggIn), cc1 (cc1In), grid (gridIn), 
               clipRegion (clipRegionIn), overageSet (overageSetIn),
               overageFields (overageFieldsIn),
               deg (degIn), lLeft (lLeftIn), uRight (uRightIn) 
        { }
};

// function prototypes

void outputCell (GridGenParam& dp, const DgIDGGSBase& dggs, const DgIDGGBase& dgg,
                   const DgLocation& add2D, const DgPolygon& verts,
                   const DgContCartRF& deg, const string& label);

bool evalCell (DgEvalData* data, DgIVec2D& add2D);

bool evalCell (GridGenParam& dp,  const DgIDGGBase& dgg, const DgContCartRF& cc1,
               const DgDiscRF2D& grid, DgQuadClipRegion& clipRegion,
               const DgIVec2D& add2D);

void createClipRegions (GridGenParam& dp, const DgIDGGBase& dgg,
                DgQuadClipRegion clipRegions[], set<DgIVec2D> overageSet[],
                map<DgIVec2D, set<DgDBFfield> > overageFields[]);

#endif
