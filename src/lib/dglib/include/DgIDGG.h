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
// DgIDGG.h: DgIDGG class definitions
//
// Version 6.1 - Kevin Sahr, 5/23/13
// Version 7.0 - Kevin Sahr, 11/16/14
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGIDGG_H 
#define DGIDGG_H

#include "DgIDGGBase.h"
#include "DgApSeq.h"

class DgIDGGSBase;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  Icosahedral DGG class.
//
class DgIDGG : public DgIDGGBase {

   //using DgDiscRF<DgQ2DICoord, DgGeoCoord, long double>::setVertices;

   public:

      DgIDGG (const DgIDGGSBase* dggs, const DgGeoSphRF& geoRFIn, const DgGeoCoord& vert0,
              long double azDegs, unsigned int apertureIn, int resIn, 
              const string& nameIn = "IDGG", 
              const string& gridTopoIn = string("HEXAGON"),
              const string& projType = string("ISEA"),
              bool isMixedApIn = false, int numAp4 = 0, 
              bool isSuperfund = false, int sfRes = 17, 
              bool isApSeq = false, const DgApSeq& apSeq = DgApSeq::defaultApSeq,
              unsigned int precisionIn = DEFAULT_PRECISION);

      DgIDGG (const DgIDGG& grd);

      virtual const DgGeoSphRF&  geoRF  (void) const { return geoRF_; }
      virtual const DgGeoCoord&  vert0  (void) const { return vert0_; }
      virtual long double        azDegs (void) const { return azDegs_; }

      virtual const string& gridTopo (void) const { return gridTopo_; }
      virtual const string& projType (void) const { return projType_; }

      bool               isApSeq     (void) const { return isApSeq_; }
      const DgApSeq&     apSeq       (void) const { return apSeq_; }
      bool               isMixed43   (void) const { return isMixed43_; }
      bool               isSuperfund (void) const { return isSuperfund_; }
      int                sfRes       (void) const { return sfRes_; }
      int                numAp4      (void) const { return numAp4_; }
      virtual int        outputRes   (void) const 
                               { return (isSuperfund_) ? sfRes_ : res(); }

   protected:

      void initialize (void);

   private:

      const DgGeoSphRF& geoRF_;
      DgGeoCoord vert0_;
      long double azDegs_;
      string gridTopo_;
      string projType_;

      bool isApSeq_;
      DgApSeq apSeq_;
      bool isMixed43_;
      int  numAp4_;
      bool isSuperfund_;
      int  sfRes_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
