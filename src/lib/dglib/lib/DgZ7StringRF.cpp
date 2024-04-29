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
#include <dglib/DgRadixString.h>

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
   printf("DgQ2DItoZ7StringConverter::convertTypedAddress\n");

   string qstr = dgg::util::to_string(addIn.quadNum(), 2);
   string addstr = qstr;
/*
    // check for res 0/base cell
    if (res == 0) {
        if (fijk->coord.i > MAX_FACE_COORD || fijk->coord.j > MAX_FACE_COORD ||
            fijk->coord.k > MAX_FACE_COORD) {
            // out of range input
            return H3_NULL;
        }

        H3_SET_BASE_CELL(h, _faceIjkToBaseCell(fijk));
        return h;
    }

    // we need to find the correct base cell FaceIJK for this H3 index;
    // start with the passed in face and resolution res ijk coordinates
    // in that face's coordinate system
    FaceIJK fijkBC = *fijk;

    // build the H3Index from finest res up
    // adjust r for the fact that the res 0 base cell offsets the indexing
    // digits
    CoordIJK *ijk = &fijkBC.coord;
    for (int r = res - 1; r >= 0; r--) {
        CoordIJK lastIJK = *ijk;
        CoordIJK lastCenter;
        if (isResolutionClassIII(r + 1)) {
            // rotate ccw
            _upAp7(ijk);
            lastCenter = *ijk;
            _downAp7(&lastCenter);
        } else {
            // rotate cw
            _upAp7r(ijk);
            lastCenter = *ijk;
            _downAp7r(&lastCenter);
        }

        CoordIJK diff;
        _ijkSub(&lastIJK, &lastCenter, &diff);
        _ijkNormalize(&diff);

        H3_SET_INDEX_DIGIT(h, r + 1, _unitIjkToDigit(&diff));
    }

    // fijkBC should now hold the IJK of the base cell in the
    // coordinate system of the current face

    if (fijkBC.coord.i > MAX_FACE_COORD || fijkBC.coord.j > MAX_FACE_COORD ||
        fijkBC.coord.k > MAX_FACE_COORD) {
        // out of range input
        return H3_NULL;
    }

    // lookup the correct base cell
    int baseCell = _faceIjkToBaseCell(&fijkBC);
    H3_SET_BASE_CELL(h, baseCell);

    // rotate if necessary to get canonical base cell orientation
    // for this base cell
    int numRots = _faceIjkToBaseCellCCWrot60(&fijkBC);
    if (_isBaseCellPentagon(baseCell)) {
        // force rotation out of missing k-axes sub-sequence
        if (_h3LeadingNonZeroDigit(h) == K_AXES_DIGIT) {
            // check for a cw/ccw offset face; default is ccw
            if (_baseCellIsCwOffset(baseCell, fijkBC.face)) {
                h = _h3Rotate60cw(h);
            } else {
                h = _h3Rotate60ccw(h);
            }
        }

        for (int i = 0; i < numRots; i++) h = _h3RotatePent60ccw(h);
    } else {
        for (int i = 0; i < numRots; i++) {
            h = _h3Rotate60ccw(h);
        }
    }
 */

   // now
/*
   if (IDGG().dggs()->aperture() == 3) {
      dgcout << "Class " << ((IDGG().isClassI()) ? "I" : "II") << endl;
   }
*/
/*
   if (effRes_ > 0) {
//dgcout << "** addIn " << addIn << endl;
      DgRadixString rs1(effRadix_, (int) addIn.coord().i(), effRes_);
      DgRadixString rs2(effRadix_, (int) addIn.coord().j(), effRes_);
//dgcout << "rs1 " << rs1 << endl;
//dgcout << "rs2 " << rs2 << endl;

      string digits1 = rs1.digits();
      string digits2 = rs2.digits();

      int n1 = (int) digits1.length();
      int n2 = (int) digits2.length();

      // pad shorter one with leading 0's
      if (n2 < n1) {
         for (int i = 0; i < (n1 - n2); i++) digits2 = "0" + digits2;
      } else {
         for (int i = 0; i < (n2 - n1); i++) digits1 = "0" + digits1;
      }

      // convert each coordinate pair into Z7 digits

      // table of Z7 digits corresponding to the
      // class I (i,j) coordinates
      static const string z3digits[3][3] = {
       // j = 0     1     2
           {"00", "22", "21"}, // i == 0
           {"01", "02", "20"}, // i == 1
           {"12", "10", "11"}  // i == 2
      };

      for (unsigned int i = 0; i < digits1.length(); i++) {
         addstr = addstr +
              z3digits[digits1[i] - '0'][digits2[i] - '0'];
      }

      // trim last digit if Class II
      if (!IDGG().isClassI() && addstr.length())
         addstr.pop_back();
   }
*/

   DgZ7StringCoord zorder;
//   zorder.setValString(addstr);
//dgcout << "zorder " << zorder << endl;

   return zorder;

} // DgZ7StringCoord DgQ2DItoZ7StringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZ7StringToQ2DIConverter::DgZ7StringToQ2DIConverter
                (const DgRF<DgZ7StringCoord, long long int>& from,
                 const DgRF<DgQ2DICoord, long long int>& to)
        : DgConverter<DgZ7StringCoord, long long int, DgQ2DICoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
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

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = 7;
   // effRes_ is the number of Class I resolutions
   effRes_ = (IDGG().res() + 1) / 2;

} // DgQ2DItoZ7StringConverter::DgQ2DItoZ7StringConverter

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgZ7StringToQ2DIConverter::convertTypedAddress (const DgZ7StringCoord& addIn) const
{
   printf("DgZ7StringToQ2DIConverter::convertTypedAddress\n");

   string addstr = addIn.valString();

   // first get the quad number
   string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);

   // res 0 is just the quad number
   if (effRes_ == 0)
      return DgQ2DICoord(quadNum, DgIVec2D(0, 0));

   int index = 2; // skip the two quad digits

   // the rest is the Z7 digit string
   string z7str = addstr.substr(index);

//dgcout << "z7str in: " << z7str;

   // adjust if Class II (odd res)
   if (z7str.length() % 2)
      z7str += "0";
//dgcout << " adjusted: " << z7str << endl;

/*
   // build the digit string for i and j from the two-digit
   // z7 codes
   string radStr1 = "";
   string radStr2 = "";
   for (int i = 0; i < z7str.length(); i += 2) {
      string z7code = z7str.substr(i, 2);
      if (z7code == "00") {
         radStr1 += "0";
         radStr2 += "0";
      } else if (z7code == "22") {
         radStr1 += "0";
         radStr2 += "1";
      } else if (z7code == "21") {
         radStr1 += "0";
         radStr2 += "2";
      } else if (z7code == "01") {
         radStr1 += "1";
         radStr2 += "0";
      } else if (z7code == "02") {
         radStr1 += "1";
         radStr2 += "1";
      } else if (z7code == "20") {
         radStr1 += "1";
         radStr2 += "2";
      } else if (z7code == "12") {
         radStr1 += "2";
         radStr2 += "0";
      } else if (z7code == "10") {
         radStr1 += "2";
         radStr2 += "1";
      } else if (z7code == "11") {
         radStr1 += "2";
         radStr2 += "2";
      }
//      dgcout << "z7code: " << z7code << " radStr1: " << radStr1
//             << " radStr2: " << radStr2 << endl;
   }

   DgRadixString rad1(effRadix_, radStr1);
   DgRadixString rad2(effRadix_, radStr2);

//   dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << endl;
*/

   //DgQ2DICoord q2di(quadNum, DgIVec2D(rad1.value(), rad2.value()));
   //dgcout << "q2di: " << q2di << endl;

   DgQ2DICoord q2di(0, DgIVec2D(0, 0));
   return q2di;

} // DgQ2DICoord DgZ7StringToQ2DIConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
