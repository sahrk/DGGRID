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
// SubOpDGG.h: this file defines the parameters for creating a DGG in DGGRID
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPDGG_H
#define SUBOPDGG_H

#include <dglib/DgApSeq.h>
#include <dglib/DgGridTopo.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgAddressType.h>

#include "SubOpBasic.h"

class DgRandom;
struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
struct SubOpDGG : public SubOpBasic {

   static const int MAX_DGG_RES;

   SubOpDGG (OpBasic& op, bool activate = true);

   // DGG access methods
   DgRFNetwork&         net0   (void) { return _net0; }
   const DgGeoSphRF&    geoRF  (void) { return *_pGeoRF; }
   const DgIDGGSBase&   dggs   (void) { return *_pDGGS; }
   const DgIDGGBase&    dgg    (void) { return *_pDGG; }
   const DgGeoSphDegRF& deg    (void) { return *_pDeg; }
   const DgIDGGBase&    chdDgg (void) { return *_pChdDgg; }
   const DgGeoSphDegRF& chdDeg (void) { return *_pChdDeg; }

   // set rf and chdRF based on type
   // return if seq num
   bool addressTypeToRF (dgg::addtype::DgAddressType type, const DgRFBase** rf,
             const DgRFBase** chdRF = nullptr, int forceRes = -1);

   // DgApSubOperation virtual methods that use the pList
   virtual int initializeOp (void);
   virtual int setupOp (void);
   virtual int cleanupOp (void);

   // create the DGG
   virtual int executeOp (void);

   // internal helper methods
   void determineRes (void);
   void orientGrid   (void);

   // the created DGG
   DgRFNetwork          _net0;
   const DgGeoSphRF*    _pGeoRF;
   const DgIDGGSBase*   _pDGGS;
   const DgIDGGBase*    _pDGG;
   const DgGeoSphDegRF* _pDeg;
   const DgIDGGBase*    _pChdDgg;   // child res dgg
   const DgGeoSphDegRF* _pChdDeg;

   // the parameters
   string dggsType;              // preset DGGS type
   dgg::topo::DgGridTopology gridTopo;      // Diamond/Hexagon/Triangle
   dgg::topo::DgGridMetric   gridMetric;    // D4/D8
   int aperture;      // aperture
   string projType;   // projection type
   int res;           // resolution (may be adjusted)
   int actualRes;     // original, actual resolution
   bool placeRandom;  // random grid placement?
   bool orientCenter; // grid placement based on quad center point?
   DgRandom* orientRand; // RNG for generating random orientation
   int numGrids;      // number of grids to generate
   int curGrid;       // grid counter
   bool lastGrid;     // last grid?
   long long int sampleCount;         // last sample point sequence number
   unsigned long long int nSamplePts;
   DgGeoCoord vert0;  // placement vert
   long double azimuthDegs; // orientation azimuth
   long double earthRadius; // earth radius in km
   string datum;            // datum used to determine the earthRadius
   string apertureType; // "PURE", "MIXED43", "SUPERFUND", or "SEQUENCE"
   bool   isMixed43;       // are we using mixed43 aperture?
   int numAp4;          // # of leading ap 4 resolutions in a mixed grid
   bool   isSuperfund;
   bool   isApSeq;      // are we using an aperture sequence?
   DgApSeq apSeq;
   int   sfRes; // superfund digit resolution
};

////////////////////////////////////////////////////////////////////////////////

#endif
