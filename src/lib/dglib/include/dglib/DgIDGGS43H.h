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
// DgIDGGS43H.h: DgIDGGS43H class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGIDGGS43H_H
#define DGIDGGS43H_H

#include <dglib/DgHexIDGGS.h>
#include <dglib/DgIVec2D.h>
#include <dglib/DgLocVector.h>
#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgIDGGS43H : public DgHexIDGGS {

   public:

      static const DgIDGGS43H* makeRF (DgRFNetwork& networkIn, const DgGeoSphRF& backFrameIn,
             const DgGeoCoord& vert0, long double azDegs, int nResIn = 1,
             const string& nameIn = "ISEA43H", const string& projType = "ISEA",
             int numAp4 = 0, bool isSuperfund = false)
         { return new DgIDGGS43H(networkIn, backFrameIn, vert0, azDegs, nResIn,
                                             nameIn, projType, numAp4, isSuperfund); }

     ~DgIDGGS43H (void);

      DgIDGGS43H& operator= (const DgIDGGS43H& rf);

      long double frequency   (void) const { return frequency_; }

   protected:

      DgIDGGS43H (DgRFNetwork& networkIn, const DgGeoSphRF& backFrameIn,
             const DgGeoCoord& vert0, long double azDegs, int nResIn = 1,
             const string& nameIn = "ISEA43H", const string& projType = "ISEA",
             int numAp4 = 0, bool isSuperfund = false)
         : DgHexIDGGS (networkIn, backFrameIn, vert0, azDegs, 3, nResIn,
                nameIn, projType, false, DgApSeq::defaultApSeq, true,
                numAp4, isSuperfund)
         { frequency_ = sqrtl(aperture()); }

      DgIDGGS43H (const DgIDGGS43H& rf);

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
