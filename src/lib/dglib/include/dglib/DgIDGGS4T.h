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
// DgIDGGS4T.h: DgIDGGS4T class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGIDGGS4T_H
#define DGIDGGS4T_H

#include <dglib/DgRF.h>
#include <dglib/DgLocVector.h>
#include <dglib/DgTriIDGG.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIVec2D.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgIDGGS4T : public DgIDGGS {

   public:

      static const DgIDGGS4T* makeRF (DgRFNetwork& networkIn, const DgGeoSphRF& backFrameIn,
               const DgGeoCoord& vert0, long double azDegs, int nResIn = 1,
               const string& nameIn = "ISEA4T", const string& projType = "ISEA")
         { return new DgIDGGS4T(networkIn, backFrameIn, vert0, azDegs, nResIn,
                                    nameIn, projType); }

     ~DgIDGGS4T (void);

      // copy constructor and operator= not implemented
      DgIDGGS4T& operator= (const DgIDGGS4T& rf);

      const DgTriIDGG& triIdgg (int res) const
             { return static_cast<const DgTriIDGG&>(idggBase(res)); }


   protected:

      DgIDGGS4T (DgRFNetwork& networkIn, const DgGeoSphRF& backFrameIn,
               const DgGeoCoord& vert0, long double azDegs, int nResIn = 1,
               const string& nameIn = "ISEA4T", const string& projType = "ISEA");

      DgIDGGS4T (const DgIDGGS4T& rf);

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
