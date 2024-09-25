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
// DgNdxHierIDGGS.h: DgNdxHierIDGGS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGNDXHIERIDGGS_H
#define DGNDXHIERIDGGS_H

#include <vector>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgNdxHierIDGGSBase.h>

////////////////////////////////////////////////////////////////////////////////
template <class TINT, class TSTR> class DgNdxHierIDGGS :
                                     public DgNdxHierIDGGSBase {

   public:

   protected:

     DgNdxHierIDGGS (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
            const string& nameIn = "NdxHierIDGGS")
        : DgNdxHierIDGGSBase(dggsIn, outModeIntIn, nameIn)
     { 
        for (int r = 0; r < res(); r++) {
           grids_[r] = new DgHierNdxIDGG(*this, r, );

      DgHierNdxIDGG (const DgNdxHierIDGGSBase& ndxHierDggsIn, int resIn,
               bool outModeIntIn = true, const string& nameIn = "HierNdxIDGG");


        }
     }

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                     DgLocVector& children) const = 0;
};

#endif
