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
// DgDiscRFS2D.h: DgDiscRFS2D class definitions
//
// Version 7.0 - Kevin Sahr, 12/14/14
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDISCRFS2D_H 
#define DGDISCRFS2D_H

#include <cmath>

#include "DgDiscRFS.h"
#include "DgDVec2D.h"
#include "DgIVec2D.h"
#include "DgApSeq.h"

////////////////////////////////////////////////////////////////////////////////
class DgDiscRFS2D : public DgDiscRFS<DgIVec2D, DgDVec2D, long double> {

   public:

      static DgDiscRFS2D* makeRF (DgRFNetwork& network, 
                   const DgRF<DgDVec2D, long double>& backFrame,
                   int nRes = 1, unsigned int aperture = 4,
                   bool isCongruent = true, bool isAligned = false,
                   const string& name = "DiscRFS2D", 
                   const string geometry = "sqr8", bool isMixed43 = false,
                   int numAp4 = 0, bool isSuperfund = false,
                   bool isApSeq = false, const DgApSeq& apSeq = DgApSeq::defaultApSeq);

      DgDiscRFS2D (DgRFNetwork& network, 
                   const DgRF<DgDVec2D, long double>& backFrame,
                   int nRes = 1, unsigned int aperture = 4,
                   bool isCongruent = true, bool isAligned = false,
                   const string& name = "DiscRFS2D")
        : DgDiscRFS<DgIVec2D, DgDVec2D, long double> 
              (network, backFrame, nRes, aperture, 
                               isCongruent, isAligned, name)
           { undefLoc_ = makeLocation(undefAddress()); }

      DgDiscRFS2D (const DgDiscRFS2D& grd) 
        : DgDiscRFS<DgIVec2D, DgDVec2D, long double> (grd) 
           { undefLoc_ = makeLocation(undefAddress()); }

      DgDiscRFS2D& operator= (const DgDiscRFS2D& grd)
           { DgDiscRFS<DgIVec2D, DgDVec2D, long double>::operator=(grd); 
               return *this; }

      virtual const DgResAdd<DgIVec2D>& undefAddress (void) const 
           { static DgResAdd<DgIVec2D> undef(DgIVec2D::undefDgIVec2D, -1); 
             return undef; }

   protected:

      // remind users of the pure virtual functions remaining from above

      virtual void setAddParents (const DgResAdd<DgIVec2D>& add,
                                  DgLocVector& vec) const = 0;

      virtual void setAddInteriorChildren (const DgResAdd<DgIVec2D>& add,
                                           DgLocVector& vec) const = 0;

      virtual void setAddBoundaryChildren (const DgResAdd<DgIVec2D>& add,
                                           DgLocVector& vec) const = 0;

      virtual void setAddAllChildren (const DgResAdd<DgIVec2D>& add,
                                      DgLocVector& vec) const = 0;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
