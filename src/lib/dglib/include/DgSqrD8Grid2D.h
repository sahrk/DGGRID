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
// DgSqrD8Grid2D.h: DgSqrD8Grid2D class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGSQRGRIDD82D_H 
#define DGSQRGRIDD82D_H

#include "DgSqrD4Grid2D.h"

////////////////////////////////////////////////////////////////////////////////
class DgSqrD8Grid2D : public DgSqrD4Grid2D {

   public:

      DgSqrD8Grid2D (DgRFNetwork& networkIn, 
                     const DgRF<DgDVec2D, long double>& contCartFrameIn,
                     const string& nameIn = "Sqr2D")
         : DgSqrD4Grid2D (networkIn, contCartFrameIn, nameIn) { }

      DgSqrD8Grid2D (const DgSqrD8Grid2D& grd) 
         : DgSqrD4Grid2D (grd) {}

      virtual long long int dist (const DgIVec2D& add1, const DgIVec2D& add2) const
           { long long int diffi = abs(add2.i() - add1.i());
             long long int diffj = abs(add2.j() - add1.j());
             return (diffi >= diffj) ? diffi : diffj; }

   protected:

      virtual void setAddNeighbors (const DgIVec2D& add, DgLocVector& vec) 
                                                                        const;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
