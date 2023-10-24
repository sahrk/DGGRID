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

#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZ3StringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgRadixString.h>

////////////////////////////////////////////////////////////////////////////////
const DgZ3StringCoord DgZ3StringCoord::undefDgZ3StringCoord("99");

////////////////////////////////////////////////////////////////////////////////
const char*
DgZ3StringRF::str2add (DgZ3StringCoord* add, const char* str,
                         char delimiter) const
{
   if (!add) add = new DgZ3StringCoord();

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

} // const char* DgZ3StringRF::str2add

////////////////////////////////////////////////////////////////////////////////
DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter
                (const DgRF<DgQ2DICoord, long long int>& from,
                 const DgRF<DgZ3StringCoord, long long int>& to)
        : DgConverter<DgQ2DICoord, long long int, DgZ3StringCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&fromFrame());
   if (!pIDGG_) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         " fromFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZ3StringRF* zRF = dynamic_cast<const DgZ3StringRF*>(&toFrame());
   if (!zRF) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         " toFrame not of type DgZ3StringRF", DgBase::Fatal);
   }

   if (IDGG().dggs()->aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         "fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon || IDGG().dggs()->aperture() != 3) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         "only implemented for aperture 3 hexagon grids", DgBase::Fatal);
   }

   effRadix_ = 3;

   // effRes_ is the number of Class I resolutions
   effRes_ = (IDGG().res() + 1) / 2;

} // DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter

////////////////////////////////////////////////////////////////////////////////
DgZ3StringCoord
DgQ2DItoZ3StringConverter::convertTypedAddress (const DgQ2DICoord& addIn) const
{
   string qstr = dgg::util::to_string(addIn.quadNum(), 2);
   string addstr = qstr;

/*
   if (IDGG().dggs()->aperture() == 3) {
      dgcout << "Class " << ((IDGG().isClassI()) ? "I" : "II") << endl;
   }
*/
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

      // convert each coordinate pair into Z3 digits

      // table of Z3 digits corresponding to the
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

   DgZ3StringCoord zorder;
   zorder.setValString(addstr);
//dgcout << "zorder " << zorder << endl;

   return zorder;

} // DgZ3StringCoord DgQ2DItoZ3StringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZ3StringToQ2DIConverter::DgZ3StringToQ2DIConverter
                (const DgRF<DgZ3StringCoord, long long int>& from,
                 const DgRF<DgQ2DICoord, long long int>& to)
        : DgConverter<DgZ3StringCoord, long long int, DgQ2DICoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&toFrame());
   if (!pIDGG_) {
      report("DgZ3StringToQ2DIConverter::DgZ3StringToQ2DIConverter(): "
         " toFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZ3StringRF* zRF = dynamic_cast<const DgZ3StringRF*>(&fromFrame());
   if (!zRF) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         " fromFrame not of type DgZ3StringRF", DgBase::Fatal);
   }

   if (IDGG().dggs()->aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
      report("DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon || IDGG().dggs()->aperture() != 3) {
      report("DgZ3StringToQ2DIConverter::DgZ3StringToQ2DIConverter(): "
         "only implemented for aperture 3 hexagon grids", DgBase::Fatal);
   }

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = 3;
   // effRes_ is the number of Class I resolutions
   effRes_ = (IDGG().res() + 1) / 2;

} // DgQ2DItoZ3StringConverter::DgQ2DItoZ3StringConverter

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgZ3StringToQ2DIConverter::convertTypedAddress (const DgZ3StringCoord& addIn) const
{
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

   // the rest is the Z3 digit string
   string z3str = addstr.substr(index);

//dgcout << "z3str in: " << z3str;

   // adjust if Class II (odd res)
   if (z3str.length() % 2)
      z3str += "0";
//dgcout << " adjusted: " << z3str << endl;

   // build the digit string for i and j from the two-digit
   // z3 codes
   string radStr1 = "";
   string radStr2 = "";
   for (int i = 0; i < z3str.length(); i += 2) {
      string z3code = z3str.substr(i, 2);
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
//             << " radStr2: " << radStr2 << endl;
   }

   DgRadixString rad1(effRadix_, radStr1);
   DgRadixString rad2(effRadix_, radStr2);

//   dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << endl;

   DgQ2DICoord q2di(quadNum, DgIVec2D(rad1.value(), rad2.value()));
   //dgcout << "q2di: " << q2di << endl;

   return q2di;

} // DgQ2DICoord DgZ3StringToQ2DIConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
