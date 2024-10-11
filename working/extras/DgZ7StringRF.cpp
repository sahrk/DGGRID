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
// DgZ7StringRF.cpp: DgZ7StringRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZ7StringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgIVec3D.h>

////////////////////////////////////////////////////////////////////////////////
const DgZ7StringCoord DgZ7StringCoord::undefDgZ7StringCoord("99");

////////////////////////////////////////////////////////////////////////////////
const char*
DgZ7StringRF::str2add (DgZ7StringCoord* add, const char* str,
                         char delimiter) const
{
   if (!add) add = new DgZ7StringCoord();

   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);
   char* tok = strtok(tmpStr, delimStr);

   add->setValString(tok);

   unsigned long offset = strlen(tok) + 1;
   delete[] tmpStr;

   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZ7StringRF::str2add

////////////////////////////////////////////////////////////////////////////////
DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter
                (const DgRF<DgQ2DICoord, long long int>& from,
                 const DgRF<DgZ7StringCoord, long long int>& to)
        : DgConverter<DgQ2DICoord, long long int, DgZ7StringCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&fromFrame());
   if (!pIDGG_) {
      report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         " fromFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZ7StringRF* zRF = dynamic_cast<const DgZ7StringRF*>(&toFrame());
   if (!zRF) {
      report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         " toFrame not of type DgZ7StringRF", DgBase::Fatal);
   }

   if (IDGG().dggs()->aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
       report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         "fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon || IDGG().dggs()->aperture() != 7) {
      report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         "only implemented for aperture 7 hexagon grids", DgBase::Fatal);
   }

   effRadix_ = 7;

   // effRes_ is the number of Class I resolutions
   effRes_ = (IDGG().res() + 1) / 2;

} // DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter

////////////////////////////////////////////////////////////////////////////////
DgZ7StringCoord
DgQ2DItoZ7StringConverter::convertTypedAddress (const DgQ2DICoord& addIn) const
{
   //printf("DgQ2DItoZ7StringConverter::convertTypedAddress\n");
/*
   // check for res 0/base cell
    if (res == 0) {
        H3_SET_BASE_CELL(h, _faceIjkToBaseCell(fijk));
        return h;
    }
*/
    // we need to find the correct base cell for this H3 index;
    // start with the passed in quad and resolution res ijk coordinates
    // in that quad's coordinate system
    DgIVec3D ijk = addIn.coord();
    int baseCell = addIn.quadNum();
    DgIVec3D baseCellIjk = ijk;
    int res = IDGG().res();
    bool isClassIII = res % 2; // odd resolutions are Class III
    // for Class III effective res of q2di is the Class I substrate
    int effectiveRes = (isClassIII) ? res + 1 : res;

    // build the Z7 index from finest res up
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

    string bcstr = dgg::util::to_string(baseCell, 2);
    string addstr = bcstr;
    for (int r = 1; r < res+1; r++) {
         addstr = addstr + to_string((int) digits[r]);
    }

    free(digits);
    digits = NULL;

    DgZ7StringCoord z7str;
    z7str.setValString(addstr);
    //dgcout << "addIn: " << addIn << " baseijk: " << baseCellIjk << " z7str: " << z7str << endl;
    //dgcout << z7str << " " << addIn << endl;

    return z7str;

} // DgZ7StringCoord DgQ2DItoZ7StringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZ7StringToQ2DIConverter::DgZ7StringToQ2DIConverter
                (const DgRF<DgZ7StringCoord, long long int>& from,
                 const DgRF<DgQ2DICoord, long long int>& to)
        : DgConverter<DgZ7StringCoord, long long int, DgQ2DICoord, long long int> (from, to),
          pIDGG_ (NULL), res_ (0), numClassI_ (0), unitScaleClassIres_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&toFrame());
   if (!pIDGG_) {
      report("DgZ7StringToQ2DIConverter::DgZ7StringToQ2DIConverter(): "
         " toFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZ7StringRF* zRF = dynamic_cast<const DgZ7StringRF*>(&fromFrame());
   if (!zRF) {
      report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         " fromFrame not of type DgZ7StringRF", DgBase::Fatal);
   }

   if (IDGG().dggs()->aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
      report("DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon || IDGG().dggs()->aperture() != 7) {
      report("DgZ7StringToQ2DIConverter::DgZ7StringToQ2DIConverter(): "
         "only implemented for aperture 3 hexagon grids", DgBase::Fatal);
   }

   res_ = IDGG().res();
   numClassI_ = (IDGG().res() + 1) / 2;
   unitScaleClassIres_ = 1;
   for (int r = 0; r < numClassI_; r++)
      unitScaleClassIres_ *= 7;

} // DgZ7StringToQ2DIConverter::DgZ7StringToQ2DIConverter

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgZ7StringToQ2DIConverter::convertTypedAddress (const DgZ7StringCoord& addIn) const
{
   //printf("DgZ7StringToQ2DIConverter::convertTypedAddress\n");

   string addstr = addIn.valString();

   // first get the base cell number
   string bstr = addstr.substr(0, 2);
   if (bstr[0] == '0') // leading 0
      bstr = bstr.substr(1, 1);
   int bcNum = std::stoi(bstr);
    if (bcNum < 0 || bcNum > 11) {
        report("DgZ7StringToQ2DIConverter::convertTypedAddress(): "
           "index has invalid base cell number", DgBase::Fatal);
     }

   // the rest is the Z7 digit string
   int index = 2; // skip the two base cell digits
   string z7str = addstr.substr(index);
    int res = (int) z7str.length();
    if (res != res_) {
        report("DgZ7StringToQ2DIConverter::convertTypedAddress(): "
           "index does not match DGG resolution", DgBase::Fatal);
     }

    // res 0 is just the quad number
    if (res == 0)
       return DgQ2DICoord(bcNum, DgIVec2D(0, 0));

   // adjust if Class III (odd res)
    if (res % 2) {
        z7str += "0";
        res++;
    }

   DgIVec3D ijk = { 0, 0, 0 };
   for (int r = 0; r < res; r++) {
       if ((r + 1) % 2) { // first res is 1, not 0
           // Class III == rotate ccw
           ijk.downAp7();
       } else {
           // Class I == rotate cw
           ijk.downAp7r();
       }

       ijk.neighbor((DgIVec3D::Direction) (z7str.c_str()[r] - '0'));
   }

   DgIVec2D ij = DgIVec2D(ijk);
   int quadNum = bcNum;

    if (ij.i() == 0 && ij.j() == 0) {
         return DgQ2DICoord(quadNum, DgIVec2D(0, 0));
    }

    const int inverseAdjacentBaseCellTable[12][2] = {
        { 0,  0 }, // q0
        { 5, 10 }, // q1
        { 1,  6 }, // q2
        { 2,  7 }, // q3
        { 3,  8 }, // q4
        { 4,  9 }, // q5
        { 10, 1 }, // q6
        { 6,  2 }, // q7
        { 7,  3 }, // q8
        { 8,  4 }, // q9
        { 9,  5 }, // q10
        { 11, 11 } // q11
    };

    //dgcout << addIn << " " << ij << " ";

    bool negI = ij.i() < 0;
    bool negJ = ij.j() < 0;
    long int origI = ij.i();
    if (bcNum == 0) {
        if (!negI) {
            if (!negJ) { // +i, +j
                if (ij.i() > ij.j()) {
                    quadNum = 2;
                    ij.setI(ij.j());
                    ij.setJ(unitScaleClassIres_ - (origI - ij.j()));
                } else { // i <= j
                    quadNum = 3;
                    ij.setI(ij.j() - ij.i());
                    ij.setJ(unitScaleClassIres_ - origI);
                }
            } else { // +i, -j}
                quadNum = 1;
                ij.setJ(ij.j() + unitScaleClassIres_);
            }
        } else { //  -i
            if (!negJ) {
                if (ij.j() == 0) {
                    quadNum = 4;
                    ij.setJ(unitScaleClassIres_ + ij.i());
                    ij.setI(0);
                } else { // -i, i > 0
                    quadNum = 3;
                    ij.setI(-ij.i());
                    ij.setJ(unitScaleClassIres_ - ij.j());
                }
            } else { //  -i, j < 0
                if (ij.i() < ij.j()) {
                    quadNum = 4;
                    ij.setI(-ij.j());
                    ij.setJ(unitScaleClassIres_ - (-origI + ij.j()));
                } else { // i >= j
                    quadNum = 5;
                    ij.setI(origI - ij.j());
                    ij.setJ(unitScaleClassIres_ + origI);
                }
            }
        }
    } else if (bcNum == 11) {
        if (!negI) { // +i
            if (!negJ) { // +i, +j
                if (ij.i() == 0) {
                    quadNum = 6;
                    ij.setI(unitScaleClassIres_ - ij.j());
                    ij.setJ(0);
                } else if (ij.j() == 0) {
                    quadNum = 8;
                    ij.setI(unitScaleClassIres_ - ij.i());
                    ij.setJ(0);
                } else if (ij.j() > ij.i()) {
                    quadNum = 6;
                    ij.setI(unitScaleClassIres_ - (ij.j() - ij.i()));
                    ij.setJ(origI);
                } else { // 0 < j <= i
                    quadNum = 7;
                    ij.setI(unitScaleClassIres_ - ij.j());
                    ij.setJ(origI - ij.j());
                }
            } else { // +i, -j
                quadNum = 8;
                ij.setI(unitScaleClassIres_ - ij.i());
                ij.setJ(-ij.j());
            }
        } else { // -i
            if (negJ) {
                if (ij.i() > ij.j()) {
                    quadNum = 8;
                    ij.setI(unitScaleClassIres_ - (-ij.j() + ij.i()));
                    ij.setJ(-origI);
                } else {
                    quadNum = 9;
                    ij.setI(unitScaleClassIres_ + ij.j());
                    ij.setJ(-origI + ij.j());
                }
            } else { // -i, +j
                quadNum = 10;
                ij.setI(unitScaleClassIres_ + ij.i());
            }
        }
    } else if (bcNum < 6) { // 1 - 5
        if (negJ) {
            ij.setJ(ij.j() + unitScaleClassIres_);
            if (negI) { // both negative
                ij.setI(ij.i() + unitScaleClassIres_);
                quadNum = inverseAdjacentBaseCellTable[bcNum][0];
            } else { // only j negative
                quadNum = inverseAdjacentBaseCellTable[bcNum][1];
            }
        }
    } else { // 6 - 10
        if (negI) {
            ij.setI(ij.i() + unitScaleClassIres_);
            if (negJ) { // both negative
                ij.setJ(ij.j() + unitScaleClassIres_);
                quadNum = inverseAdjacentBaseCellTable[bcNum][0];
            } else { // only i negative
                quadNum = inverseAdjacentBaseCellTable[bcNum][1];
            }
        }
    }

   DgQ2DICoord q2di(quadNum, ij);
   //dgcout << addIn << " " << q2di << endl;

   return q2di;

} // DgQ2DICoord DgZ7StringToQ2DIConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
