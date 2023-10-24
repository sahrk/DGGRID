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
// DgZOrderStringRF.cpp: DgZOrderStringRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgRadixString.h>
/*
#include <dglib/DgUtil.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgDmdD4Grid2DS.h>
#include <dglib/DgHexGrid2DS.h>
#include <dglib/DgTriGrid2DS.h>
#include <dglib/DgSeriesConverter.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgProjISEA.h>
#include <dglib/DgProjFuller.h>
#include <dglib/DgString.h>
*/

////////////////////////////////////////////////////////////////////////////////
const DgZOrderStringCoord DgZOrderStringCoord::undefDgZOrderStringCoord("99");

////////////////////////////////////////////////////////////////////////////////
const char*
DgZOrderStringRF::str2add (DgZOrderStringCoord* add, const char* str,
                         char delimiter) const
{
   if (!add) add = new DgZOrderStringCoord();

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

} // const char* DgZOrderStringRF::str2add

////////////////////////////////////////////////////////////////////////////////
DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter
                (const DgRF<DgQ2DICoord, long long int>& from,
                 const DgRF<DgZOrderStringCoord, long long int>& to)
        : DgConverter<DgQ2DICoord, long long int, DgZOrderStringCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&fromFrame());
   if (!pIDGG_) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         " fromFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZOrderStringRF* zRF = dynamic_cast<const DgZOrderStringRF*>(&toFrame());
   if (!zRF) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         " toFrame not of type DgZOrderStringRF", DgBase::Fatal);
   }

   if (IDGG().aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         "only implemented for hexagon grids", DgBase::Fatal);
   }

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = IDGG().radix();   // effective radix
   if (IDGG().aperture() == 3) {
       effRadix_ = 3;
       // effRes_ is the number of Class I resolutions
       effRes_ = (effRes_ + 1) / 2;
   }

   if (IDGG().gridTopo() == Triangle) effRes_++; // adjust for long double j

} // DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringCoord
DgQ2DItoZOrderStringConverter::convertTypedAddress (const DgQ2DICoord& addIn) const
{
   string qstr = dgg::util::to_string(addIn.quadNum(), 2);
   string addstr = qstr;

   if (effRes_ > 0) {
//dgcout << "** addIn " << addIn << endl;
   DgRadixString rs1(effRadix_, (int) addIn.coord().i(), effRes_);
   DgRadixString rs2(effRadix_, (int) addIn.coord().j(), effRes_);

//dgcout << "rs1 " << rs1 << endl;
//dgcout << "rs2 " << rs2 << endl;

/*
   if (IDGG().aperture() == 3) {
      dgcout << "Class " << ((IDGG().isClassI()) ? "I" : "II") << endl;
   }
*/
   addstr = addstr +
     DgRadixString::digitInterleave(rs1, rs2, !((IDGG().aperture() == 3)));

//dgcout << "addstr " << addstr << endl;
   // trim last digit if Class II
   if (IDGG().aperture() == 3 && !IDGG().isClassI() && addstr.length()) {
      addstr.pop_back();
   }
//dgcout << "trimmed " << addstr << endl;
   }

   DgZOrderStringCoord zorder;
   zorder.setValString(addstr);
//dgcout << "zorder " << zorder << endl;

   return zorder;

} // DgZOrderStringCoord DgQ2DItoZOrderStringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringToQ2DIConverter::DgZOrderStringToQ2DIConverter
                (const DgRF<DgZOrderStringCoord, long long int>& from,
                 const DgRF<DgQ2DICoord, long long int>& to)
        : DgConverter<DgZOrderStringCoord, long long int, DgQ2DICoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&toFrame());
   if (!pIDGG_) {
      report("DgZOrderStringToQ2DIConverter::DgZOrderStringToQ2DIConverter(): "
         " toFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   const DgZOrderStringRF* zRF = dynamic_cast<const DgZOrderStringRF*>(&fromFrame());
   if (!zRF) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         " fromFrame not of type DgZOrderStringRF", DgBase::Fatal);
   }

   if (IDGG().aperture() != zRF->aperture() || IDGG().res() != zRF->res()) {
      report("DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon) {
      report("DgZOrderStringToQ2DIConverter::DgZOrderStringToQ2DIConverter(): "
         "only implemented for hexagon grids", DgBase::Fatal);
   }

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = IDGG().radix();   // effective radix
   if (IDGG().aperture() == 3) {
       effRadix_ = 3;
       // effRes_ is the number of Class I resolutions
       effRes_ = (effRes_ + 1) / 2;
   }

   if (IDGG().gridTopo() == Triangle) effRes_++; // adjust for long double j

} // DgQ2DItoZOrderStringConverter::DgQ2DItoZOrderStringConverter

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgZOrderStringToQ2DIConverter::convertTypedAddress (const DgZOrderStringCoord& addIn) const
{
//dgcout << " -> " << addIn << endl;
   string addstr = addIn.valString();

   // first get the quad number
   string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);

   int index = 2; // skip the two quad digits

   // the rest is the radix string
   string radStr = addstr.substr(index);

   // split out the interleaved digits
   string radStr1 = "";
   string radStr2 = "";

   if (IDGG().aperture() == 3) {
      bool isIdigit = true; // first char is an i digit
      int lastIdigit = 0;
      for (const char& digit: radStr) {
         if (isIdigit) {
            radStr1 += dgg::util::to_string(digit);
            lastIdigit = digit - '0';
         } else
            radStr2 += dgg::util::to_string(digit);

         isIdigit = !isIdigit;
      }

      if (!IDGG().isClassI()) {
         // add the last j digit based on the last i digit
         string jDigits[] = { "0", "2", "1" };
         radStr2 += jDigits[lastIdigit];
      }

   } else {

      for (const char& digit: radStr) {

         // break out the interleaved digits
         int d = digit - '0'; // convert to int
         int c1 = (int) (d / effRadix_);

         int c2 = (d % effRadix_);

         radStr1 += dgg::util::to_string(c1);
         radStr2 += dgg::util::to_string(c2);
      }
   }

   DgRadixString rad1(effRadix_, radStr1);
   DgRadixString rad2(effRadix_, radStr2);

   //dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << endl;

   DgQ2DICoord q2di(quadNum, DgIVec2D(rad1.value(), rad2.value()));
   //dgcout << "q2di: " << q2di << endl;

   return q2di;

} // DgQ2DICoord DgZOrderStringToQ2DIConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
