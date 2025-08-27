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
// DgDmdD4Grid2DS.h: DgDmdD4Grid2DS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDMDD4GRID2DS_H
#define DGDMDD4GRID2DS_H

#include <dglib/DgDiscRFS2D.h>
#include <dglib/DgIVec2D.h>
#include <dglib/DgLocVector.h>
#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgDmdD4Grid2DS : public DgDiscRFS2D {

   public:

      static const DgDmdD4Grid2DS* makeRF (DgRFNetwork& networkIn,
                 const DgRF<DgDVec2D, long double>& backFrameIn,
                 int nRes = 1, unsigned int apertureIn = 4,
                 bool isCongruentIn = true, bool isAlignedIn = false,
                 const std::string& nameIn = "4D4")
      { return new DgDmdD4Grid2DS (networkIn, backFrameIn, nRes, apertureIn,
                 isCongruentIn, isAlignedIn, nameIn); }

     ~DgDmdD4Grid2DS (void);

      DgDmdD4Grid2DS& operator= (const DgDmdD4Grid2DS& rf);

      int radix (void) const { return radix_; }

   protected:

      DgDmdD4Grid2DS (DgRFNetwork& networkIn,
                 const DgRF<DgDVec2D, long double>& backFrameIn,
                 int nRes = 1, unsigned int apertureIn = 4,
                 bool isCongruentIn = true, bool isAlignedIn = false,
                 const std::string& nameIn = "4D4");

      DgDmdD4Grid2DS (const DgDmdD4Grid2DS& rf);

      int radix_;

      // pure virtual functions from DgDiscRFS

      virtual void setAddParents (const DgResAdd<DgIVec2D>& add,
                                  DgLocVector& vec) const;

      virtual void setAddInteriorChildren (const DgResAdd<DgIVec2D>& add,
                                           DgLocVector& vec) const;

      virtual void setAddBoundaryChildren (const DgResAdd<DgIVec2D>& add,
                                           DgLocVector& vec) const;

      virtual void setAddAllChildren (const DgResAdd<DgIVec2D>& add,
                                      DgLocVector& vec) const;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
