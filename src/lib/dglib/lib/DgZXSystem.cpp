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
#include <dglib/DgIVec3D.h>

class DgIDGGSBase;

/** max Z7 resolution */
#define MAX_Z7_RES 20

/** The number of bits in a Z7 index. */
#define Z7_NUM_BITS 64

/** The bit offset of the max resolution digit in a Z7 index. */
#define Z7_MAX_OFFSET 63

/** The bit offset of the quad number in a Z7 index. */
#define Z7_QUAD_OFFSET 60

/** 1's in the 4 quad number bits, 0's everywhere else. */
#define Z7_QUAD_MASK ((uint64_t)(15) << Z7_QUAD_OFFSET)

/** 0's in the 4 quad bits, 1's everywhere else. */
#define Z7_QUAD_MASK_NEGATIVE (~Z7_QUAD_MASK)

/** The number of bits in a single Z7 resolution digit. */
#define Z7_PER_DIGIT_OFFSET 3

/** 1's in the 3 bits of highest res digit bits, 0's everywhere else. */
#define Z7_DIGIT_MASK ((uint64_t)(7))

/** Gets the integer quad number of a Z7 index. */
#define Z7_GET_QUADNUM(z) ((int)((((z)&Z7_QUAD_MASK) >> Z7_QUAD_OFFSET)))

/** Sets the integer mode of z to v. */
#define Z7_SET_QUADNUM(z, v) \
    (z) = (((z)&Z7_QUAD_MASK_NEGATIVE) | (((uint64_t)(v)) << Z7_QUAD_OFFSET))

/** Gets the resolution res integer digit of z. */
#define Z7_GET_INDEX_DIGIT(z, res)                                        \
    ((int)((((z) >> ((MAX_Z7_RES - (res)) * Z7_PER_DIGIT_OFFSET)) & \
                  Z7_DIGIT_MASK)))

/** Sets the resolution res digit of z to the integer digit */
#define Z7_SET_INDEX_DIGIT(z, res, digit)                                  \
    (z) = (((z) & ~((Z7_DIGIT_MASK                                        \
                       << ((MAX_Z7_RES - (res)) * Z7_PER_DIGIT_OFFSET)))) | \
            (((uint64_t)(digit))                                            \
             << ((MAX_Z7_RES - (res)) * Z7_PER_DIGIT_OFFSET)))

////////////////////////////////////////////////////////////////////////////////
DgZXSystem::DgZXSystem (const DgIDGGSBase& dggsIn, bool outModeIntIn, const std::string& nameIn)
   : DgHierNdxSystemRFS<DgZXRF, DgZXStringRF>(dggsIn, outModeIntIn, nameIn)
{
    /*
    // all the grids need to be created before we can set the parent/child grids
    // RFs are deallocated by the RFNetwork
    std::vector<const DgHierNdxSystemRFBase*> rfGrids(nRes(), nullptr);
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
DgZXSystem::toIntCoord (const DgHierNdxStringCoord& addIn) const
{
   std::string addstr = addIn.valString();
    if (addstr.size() - 2 > MAX_Z7_RES) {
     report("DgZXSystem::toIntCoord(): "
        " input resolution exceeds max Z7 resolution of 20", DgBase::Fatal);
   }

   uint64_t z = 0;

   // first get the quad number and add to the val
   std::string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);

   // KEVIN: check for deleted sub-sequence

   Z7_SET_QUADNUM(z, quadNum);

   int index = 2; // skip the two quad digits

   // the rest is the radix string
   std::string radStr = addstr.substr(index);

   // now get the digits
   int r = 1;
   for (const char& digit: radStr) {
      int d = digit - '0'; // convert to int
      Z7_SET_INDEX_DIGIT(z, r, d);
      r++;
   }

   while (r <= MAX_Z7_RES) {
       Z7_SET_INDEX_DIGIT(z, r, DgIVec3D::INVALID_DIGIT);
       r++;
    }

   DgHierNdxIntCoord coord;
   coord.setValue(z);

   return coord;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord 
DgZXSystem::toStringCoord (const DgHierNdxIntCoord& addIn) const
{
   uint64_t z = addIn.value();

   int quadNum = Z7_GET_QUADNUM(z);
   std::string s = dgg::util::to_string(quadNum, 2);

   for (int r = 1; r <= MAX_Z7_RES; r++) {
      // get the integer digit
      char d = Z7_GET_INDEX_DIGIT(z, r);
      if (d == DgIVec3D::INVALID_DIGIT)
         continue;

      // convert to char
      d += '0';
      // append to index string
      s += d;
   }

   DgHierNdxStringCoord zStr;
   zStr.setValue(s);

   return zStr;
}

////////////////////////////////////////////////////////////////////////////////
void 
DgZXSystem::setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const
{
    // res has already been verified by the caller
    int pRes = add.res() - 1;
    std::string addStr = add.address().strNdx().value();
    std::string pStr = addStr.substr(0, addStr.size() - 1);
    
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
    int chdRes = add.res() + 1;
    std::string valStr = add.address().strNdx().value();
     
    // first get the base cell number
    std::string quadStr = valStr.substr(0, 2);
    if (quadStr[0] == '0') // leading 0
          quadStr = quadStr.substr(1, 1);
      int quadNum = std::stoi(quadStr);
      if (quadNum < 0 || quadNum > 11) {
           report("DgZXSystem::setAddNdxChildren(): "
              "index has invalid base cell number", DgBase::Fatal);
     }
    
    std::string addStr = valStr.substr(2);
    
    // KEVIN: this should all be done with integers and c_str's
    // assume no skip digit
    int skipDigit = -1;
    // check if current address is all zeros
    size_t pos = addStr.find_first_not_of('0');
    if (pos == std::string::npos) { // all digits are zero
        skipDigit = (quadNum <= 5) ? 2 : 5;
    }
    
    children.clearAddress();
    std::vector<DgAddressBase*>& v = children.addressVec();
    for (int i = 0; i <= 6; i++) {
        // KEVIN pentagon sub-sequence check
        
        // skip pentagon sub-sequence digits as per above
        if (i == skipDigit)
            continue;

        // build the child address
        std::string chdStr = valStr + std::to_string(i);
        DgResAdd<DgHierNdx> chdAdd;
        initNdxFromString(chdAdd, chdRes, chdStr);
        
        v.push_back(new DgAddress<DgResAdd<DgHierNdx>>(chdAdd));
    }
}

////////////////////////////////////////////////////////////////////////////////
