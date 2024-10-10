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
// DgHierNdxSystemRFS.h: DgHierNdxSystemRFS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTEMRFS_H
#define DGHIERNDXSYSTEMRFS_H

#include <vector>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgHierNdxSystemRFSBase.h>
#include <dglib/DgHierNdxSystemRF.h>

////////////////////////////////////////////////////////////////////////////////
template <class TINT, class TSTR> class DgHierNdxSystemRFS :
                                     public DgHierNdxSystemRFSBase {

   public:

   protected:

     DgHierNdxSystemRFS (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
            const string& nameIn = "HierNdxIDGGS")
        : DgHierNdxSystemRFSBase(dggsIn, outModeIntIn, nameIn)
     { /*
        // create the systems
        for (int r = 0; r < nRes(); r++)
           grids_[r] = new DgHierNdxSystemRF<TINT, TSTR>(*this, r,
                                            nameIn + to_string(r));

        // initialize the systems
        for (int r = 0; r < nRes(); r++)
           grids_[r]->initialize();
        }
*/
     };

     // pure virtual functions passed down from above

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                     DgLocVector& children) const = 0;
};

#endif
