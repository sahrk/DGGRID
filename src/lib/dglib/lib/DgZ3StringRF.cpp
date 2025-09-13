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
// DgZ3StringRF.cpp: DgZ3StringRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include <dglib/DgZ3StringRF.h>
#include <dglib/DgIVec3D.h>
#include <dglib/DgRadixString.h>

////////////////////////////////////////////////////////////////////////////////
//const DgZ3StringCoord DgZ3StringCoord::undefDgZ3StringCoord(0xffffffffffffffff);

////////////////////////////////////////////////////////////////////////////////
DgZ3StringRF::DgZ3StringRF (const DgHierNdxSystemRFBase& sysIn, int resIn, 
              const std::string& nameIn)
   : DgHierNdxStringRF(sysIn, resIn, nameIn), unitScaleClassIres_ (0)
{
    //number of Class I resolutions
    effRes_ = (dgg().res() + 1) / 2;
    effRadix_ = 3;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord
DgZ3StringRF::quantify (const DgQ2DICoord& addIn) const
{
    // we need to find the correct base cell for this H3 index;
    // start with the passed in quad and resolution res ijk coordinates
    // in that quad's coordinate system
    DgIVec3D ijk = addIn.coord();
    int baseCell = addIn.quadNum();
    DgIVec3D baseCellIjk = ijk;
    int res = dgg().res();
    bool isClassIII = res % 2; // odd resolutions are Class III
    // for Class III effective res of q2di is the Class I substrate
    int effectiveRes = (isClassIII) ? res + 1 : res;

    // build the Z3 index from finest res up
    // adjust r for the fact that the res 0 base cell offsets the indexing
    // digits
    DgIVec3D::Direction* digits =
        (DgIVec3D::Direction*) malloc((res + 1) * sizeof(DgIVec3D::Direction));
    for (int r = 0; r < res+1; r++) digits[r] = DgIVec3D::INVALID_DIGIT;
    bool first = true;
    for (int r = effectiveRes; r >= 0; r--) {
        DgIVec3D lastIJK = ijk;
        DgIVec3D lastCenter;
        // why is H3 different?
        //if ((r + 1) % 2) { // finer res is Class III
        //if (!((r + 1) % 2)) { // finer res is Class I
        if (r % 2) { // Class III
            // rotate ccw
            ijk.upAp7();
            lastCenter = ijk;
            lastCenter.downAp7();
        } else {
            // rotate cw
            ijk.upAp7r();
            lastCenter = ijk;
            lastCenter.downAp7r();
        }

        // res 1 address is our lookup for adjacent base cells
        if (r == 1)
            baseCellIjk = ijk;

        // no digit generated for Class I substrate
        if (first && isClassIII) {
           first = false;
           continue;
        }

        DgIVec3D diff = lastIJK.diffVec(lastCenter);
        // don't need to normalize; done in unitIjkPlusToDigit
        //diff.ijkPlusNormalize();

        digits[r] = diff.unitIjkPlusToDigit();
        //H3_SET_INDEX_DIGIT(h, r + 1, _unitIjkToDigit(&diff));
    }

    // adjust the base cell if necessary
    // {i, j} = {0,0}, {1, 0}, {1, 1}, and {0,1} respectively
    const int adjacentBaseCellTable[12][4] = {
        { 0, 0, 0, 0 },
        { 1, 6, 2, 0 },
        { 2, 7, 3, 0 },
        { 3, 8, 4, 0 },
        { 4, 9, 5, 0 },
        { 5, 10, 1, 0 },
        { 6, 11, 7, 2 },
        { 7, 11, 8, 3 },
        { 8, 11, 9, 4 },
        { 9, 11, 10, 5 },
        { 10, 11, 6, 1 },
        { 11, 11, 0, 0 }
    };

    int quadOriginBaseCell = baseCell;
    if (baseCellIjk.i() == 1) {
        if (baseCellIjk.j() == 0) // { 1, 0 }
            baseCell = adjacentBaseCellTable[baseCell][1];
        else // better be 1
            baseCell = adjacentBaseCellTable[baseCell][2];
    } else if (baseCellIjk.j() == 1) // { 0, 1 }
        baseCell = adjacentBaseCellTable[baseCell][3];

    // all base cells should be correct except for 0 and 11
    // Base Cell 0 maps to all 5’s, rotate into correct subdigit, skip 2
    // BC 1 - 5 skip subsequence 2
    // BC 6 - 10 skip subsequence 5
    // BC 11 maps to all 2’s, rotate into position, skip 5

    // handle the single-cell quads 0 and 11
    if (baseCell != quadOriginBaseCell) {
        if (baseCell == 0) {
            // must be quad 1 - 5
            // rotate once for each quad past 1
            for (int q = 1; q < quadOriginBaseCell; q++) {
                DgIVec3D::rotateDigitVecCCW(digits, res, DgIVec3D::PENTAGON_SKIPPED_DIGIT_TYPE1);
            }
        } else if (baseCell == 11) {
            // must be quad 6 - 10
            // rotate once for each quad less than 10
            int numRots = 10 - quadOriginBaseCell;
            for (int q = 0; q < numRots; q++) {
                DgIVec3D::rotateDigitVecCCW(digits, res, DgIVec3D::PENTAGON_SKIPPED_DIGIT_TYPE2);
            }
        }
    }

    std::string bcstr = dgg::util::to_string(baseCell, 2);
    std::string addstr = bcstr;
    DgIVec3D::Direction skipDigit = ((baseCell < 6) ? DgIVec3D::PENTAGON_SKIPPED_DIGIT_TYPE1 : DgIVec3D::PENTAGON_SKIPPED_DIGIT_TYPE2);
    int skipRotate = false;
    int firstNonZero = false;
    for (int r = 1; r < res+1; r++) {
        DgIVec3D::Direction d = digits[r];
        if (!firstNonZero && d != DgIVec3D::CENTER_DIGIT) {
            firstNonZero = true;
            if (d == skipDigit)
                skipRotate = true;
        }
         
        if (skipRotate) {
            d = DgIVec3D::rotate60ccw(d);
        }

        addstr = addstr + to_string((int) d);
    }
        
    free(digits);
    digits = NULL;

    DgHierNdxStringCoord c;
    c.setValue(addstr);

   return c;
}

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord 
DgZ3StringRF::invQuantify (const DgHierNdxStringCoord& addIn) const
{
    std::string addstr = addIn.valString();

    // first get the quad number
    std::string qstr = addstr.substr(0, 2);
    if (qstr[0] == '0') // leading 0
       qstr = qstr.substr(1, 1);
    int quadNum = std::stoi(qstr);

    // res 0 is just the quad number
    if (effRes_ == 0)
       return DgQ2DICoord(quadNum, DgIVec2D(0, 0));

    int index = 2; // skip the two quad digits

    // the rest is the Z3 digit string
    std::string z3str = addstr.substr(index);

 //dgcout << "z3str in: " << z3str;

    // adjust if Class II (odd res)
    if (z3str.length() % 2)
       z3str += "0";
 //dgcout << " adjusted: " << z3str << std::endl;

    // build the digit string for i and j from the two-digit
    // z3 codes
    std::string radStr1 = "";
    std::string radStr2 = "";
    for (int i = 0; i < z3str.length(); i += 2) {
       std::string z3code = z3str.substr(i, 2);
       if (z3code == "00") {
          radStr1 += "0";
          radStr2 += "0";
       } else if (z3code == "22") {
          radStr1 += "0";
          radStr2 += "1";
       } else if (z3code == "21") {
          radStr1 += "0";
          radStr2 += "2";
       } else if (z3code == "01") {
          radStr1 += "1";
          radStr2 += "0";
       } else if (z3code == "02") {
          radStr1 += "1";
          radStr2 += "1";
       } else if (z3code == "20") {
          radStr1 += "1";
          radStr2 += "2";
       } else if (z3code == "12") {
          radStr1 += "2";
          radStr2 += "0";
       } else if (z3code == "10") {
          radStr1 += "2";
          radStr2 += "1";
       } else if (z3code == "11") {
          radStr1 += "2";
          radStr2 += "2";
       }
 //      dgcout << "z3code: " << z3code << " radStr1: " << radStr1
 //             << " radStr2: " << radStr2 << std::endl;
    }

    DgRadixString rad1(effRadix_, radStr1);
    DgRadixString rad2(effRadix_, radStr2);

 //   dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << std::endl;

    DgQ2DICoord q2di(quadNum, DgIVec2D(rad1.value(), rad2.value()));
    //dgcout << "q2di: " << q2di << std::endl;

    return q2di;
} 

////////////////////////////////////////////////////////////////////////////////
