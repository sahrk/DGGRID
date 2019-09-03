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
// DgIDGGS43H.h: DgIDGGS43H class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGIDGGS43H_H 
#define DGIDGGS43H_H

#include "DgRF.h"
#include "DgLocVector.h"
#include "DgIDGGS.h"
#include "DgIVec2D.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgIDGGS43H : public DgIDGGS {

   public:

      DgIDGGS43H (DgRFNetwork& networkIn, const DgGeoSphRF& backFrameIn,
             const DgGeoCoord& vert0, long double azDegs, int nResIn = 1,
             const string& nameIn = "ISEA43H", const string& projType = "ISEA",
             int numAp4 = 0, bool isSuperfund = false)
         : DgIDGGS (networkIn, backFrameIn, vert0, azDegs, 3, nResIn,
                "HEXAGON", nameIn, projType, true, numAp4, isSuperfund)
         { frequency_ = sqrtl(aperture()); }

      DgIDGGS43H (const DgIDGGS43H& rf);

     ~DgIDGGS43H (void);

      DgIDGGS43H& operator= (const DgIDGGS43H& rf);

      long double frequency   (void) const { return frequency_; }

   protected:

      long double frequency_;

      // pure virtual functions from DgDiscRFS

      virtual void setAddParents (const DgResAdd<DgQ2DICoord>& add,
                                  DgLocVector& vec) const;

      virtual void setAddInteriorChildren (const DgResAdd<DgQ2DICoord>& add,
                                           DgLocVector& vec) const;

      virtual void setAddBoundaryChildren (const DgResAdd<DgQ2DICoord>& add,
                                           DgLocVector& vec) const;

      virtual void setAddAllChildren (const DgResAdd<DgQ2DICoord>& add,
                                      DgLocVector& vec) const;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
