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
// DgZXSystem.cpp: DgZXSystem class implementation
//
////////////////////////////////////////////////////////////////////////////////

//#define __STDC_FORMAT_MACROS
//#include <inttypes.h>
//#include <cmath>
//#include <climits>
//#include <cfloat>
//#include <string.h>

#include <dglib/DgZXSystem.h>
#include <dglib/DgHierNdxRF.h>

////////////////////////////////////////////////////////////////////////////////
DgZXSystem (const DgIDGGSBase& dggsIn, bool outModeIntIn, const string& nameIn)
   : DgHierNdxSystemRFS<DgZXRF, DgZXStringRF>(dggsIn, outModeIntIn, nameIn)
{
    
    for (int r = 0; r < nRes(); r++)
       grids_[r] = new DgHierNdxSystemRF<DgZXRF, DgZXStringRF>(*this, r,
                                        nameIn + to_string(r));

    // initialize the systems
    for (int r = 0; r < nRes(); r++)
       grids_[r]->initialize();
    }
     
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord 
DgZXSystem::toIntCoord (const DgHierNdxStringCoord& c)
{

}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord 
DgZXSystem::toStringCoord (const DgHierNdxIntCoord& c)
{
}

////////////////////////////////////////////////////////////////////////////////
void 
DgZXSystem::setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const
{
}

////////////////////////////////////////////////////////////////////////////////
void 
DgZXSystem::setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const
{
}

////////////////////////////////////////////////////////////////////////////////
