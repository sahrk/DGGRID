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

#include <dglib/DgHierNdx.h>
#include <dglib/DgZXSystem.h>
#include <dglib/DgHierNdxRF.h>
#include <dglib/DgHierNdxSystemRF.h>

class DgIDGGSBase;

////////////////////////////////////////////////////////////////////////////////
DgZXSystem::DgZXSystem (const DgIDGGSBase& dggsIn, bool outModeIntIn, const string& nameIn)
   : DgHierNdxSystemRFS<DgZXRF, DgZXStringRF>(dggsIn, outModeIntIn, nameIn)
{
    /*
    // all the grids need to be created before we can set the parent/child grids
    // RFs are deallocated by the RFNetwork
    vector<const DgHierNdxSystemRFBase*> rfGrids(nRes(), nullptr);
    for (int r = 0; r < nRes(); r++)
        rfGrids[r] = new DgHierNdxSystemRF<DgZXRF, DgZXStringRF>(*this, r,
                                                                 nameIn + to_string(r));
    
    for (int r = 0; r < nRes(); r++)
        rfGrids[r]->initialize();
    
    // move into our grids_
    for (int r = 0; r < nRes(); r++)
        gridsMutable()[r] = rfGrids[r];
     */
     
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord
DgZXSystem::toIntCoord (const DgHierNdxStringCoord& c) const
{
    DgHierNdxIntCoord add;
    return add;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord 
DgZXSystem::toStringCoord (const DgHierNdxIntCoord& c) const
{
    DgHierNdxStringCoord add;
    return add;
}

////////////////////////////////////////////////////////////////////////////////
void 
DgZXSystem::setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const
{
    // res has already been verified by the caller
    int pRes = add.res() - 1;
    string addStr = add.address().strNdx().value();
    string pStr = addStr.substr(0, addStr.size() - 1);
    
    // build the parent address
        
    DgResAdd<DgHierNdx> pAdd;
    initNdxFromString(pAdd, pRes, pStr);
    forceAddress(&parent, pAdd);
}

////////////////////////////////////////////////////////////////////////////////
void 
DgZXSystem::setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const
{
    int pRes = add.res() - 1;
    string addStr = add.address().strNdx().value();
    string pStr = addStr.substr(0, addStr.size() - 1);
    
    vector<DgAddressBase*>& v = children.addressVec();
    for (int i = 0; i < (int) tmpVec.size(); i++)
    {
       string childString =
       v.push_back(new DgAddress<DgResAdd<DgHierNdx>>();
    }

}

////////////////////////////////////////////////////////////////////////////////
