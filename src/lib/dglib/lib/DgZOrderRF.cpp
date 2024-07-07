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
// DgZOrderRF.cpp: DgZOrderRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZOrderRF.h>
#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgRadixString.h>

/** max ZORDER resolution */
#define MAX_ZORDER_RES 30

/** The number of bits in a ZOrder index. */
#define ZORDER_NUM_BITS 64

/** The bit offset of the max resolution digit in a ZOrder index. */
#define ZORDER_MAX_OFFSET 63

/** The bit offset of the quad number in a ZOrder index. */
#define ZORDER_QUAD_OFFSET 60

/** 1's in the 4 quad number bits, 0's everywhere else. */
#define ZORDER_QUAD_MASK ((uint64_t)(15) << ZORDER_QUAD_OFFSET)

/** 0's in the 4 mode bits, 1's everywhere else. */
#define ZORDER_QUAD_MASK_NEGATIVE (~ZORDER_QUAD_MASK)

/** The number of bits in a single ZORDER resolution digit. */
#define ZORDER_PER_DIGIT_OFFSET 2

/** 1's in the 2 bits of highest res digit bits, 0's everywhere else. */
#define ZORDER_DIGIT_MASK ((uint64_t)(3))

/** Gets the integer quad number of a ZOrder index. */
#define ZORDER_GET_QUADNUM(z) ((int)((((z)&ZORDER_QUAD_MASK) >> ZORDER_QUAD_OFFSET)))

/** Sets the integer mode of z to v. */
#define ZORDER_SET_QUADNUM(z, v) \
    (z) = (((z)&ZORDER_QUAD_MASK_NEGATIVE) | (((uint64_t)(v)) << ZORDER_QUAD_OFFSET))

/** Gets the resolution res integer digit of z. */
#define ZORDER_GET_INDEX_DIGIT(z, res)                                        \
    ((int)((((z) >> ((MAX_ZORDER_RES - (res)) * ZORDER_PER_DIGIT_OFFSET)) & \
                  ZORDER_DIGIT_MASK)))

/** Sets the resolution res digit of z to the integer digit */
#define ZORDER_SET_INDEX_DIGIT(z, res, digit)                                  \
    (z) = (((z) & ~((ZORDER_DIGIT_MASK                                        \
                       << ((MAX_ZORDER_RES - (res)) * ZORDER_PER_DIGIT_OFFSET)))) | \
            (((uint64_t)(digit))                                            \
             << ((MAX_ZORDER_RES - (res)) * ZORDER_PER_DIGIT_OFFSET)))

////////////////////////////////////////////////////////////////////////////////
const DgZOrderCoord DgZOrderCoord::undefDgZOrderCoord(0xffffffffffffffff);

////////////////////////////////////////////////////////////////////////////////
const char*
DgZOrderRF::str2add (DgZOrderCoord* add, const char* str,
                         char delimiter) const
{
   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);
   char* tok = strtok(tmpStr, delimStr);

   // convert to a unit64_t
   uint64_t val = 0;
   if (!sscanf(tok, "%" PRIx64, &val))
      report("DgZOrderRF::str2add(): invalid ZORDER index", DgBase::Fatal);

   if (!add) add = new DgZOrderCoord();
   add->setValue(val);

   unsigned long offset = strlen(tok) + 1;
   delete[] tmpStr;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZOrderRF::str2add

////////////////////////////////////////////////////////////////////////////////
string
DgZOrderCoord::valString (void) const
{
   const int maxStrSize = 17; // max 16 digits plus 1 for the null terminator
   char str[maxStrSize];
   snprintf(str, maxStrSize, "%016" PRIx64, value_);

   return string(str);

} // const string& DgZOrderRF::valString

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter
                (const DgRF<DgZOrderStringCoord, long long int>& from,
                 const DgRF<DgZOrderCoord, long long int>& to)
   : DgConverter<DgZOrderStringCoord, long long int, DgZOrderCoord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZOrderStringRF* zsRF = dynamic_cast<const DgZOrderStringRF*>(&fromFrame());
   if (!zsRF) {
      report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
         " fromFrame not of type DgZOrderStringRF", DgBase::Fatal);
   }

   const DgZOrderRF* zRF = dynamic_cast<const DgZOrderRF*>(&toFrame());
   if (!zRF) {
      report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
         " toFrame not of type DgZOrderRF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter

////////////////////////////////////////////////////////////////////////////////
DgZOrderCoord
DgZOrderStringtoZOrderConverter::convertTypedAddress (const DgZOrderStringCoord& addIn) const
{
   if (res_ > MAX_ZORDER_RES) {
     report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
        " input resolution exceeds max ZORDER resolution of 30", DgBase::Fatal);
   }

   string addstr = addIn.valString();
   uint64_t z = 0;

   // first get the quad number and add to the val
   string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);
   ZORDER_SET_QUADNUM(z, quadNum);

   int index = 2; // skip the two quad digits

   // the rest is the radix string
   string radStr = addstr.substr(index);

   // now get the digits
   int r = 1;
   for (const char& digit: radStr) {
      if (r > res_)
         report("DgZOrderStringtoZOrderConverter::convertTypedAddress(): "
         " incoming index exceeds converter resolution", DgBase::Fatal);

      int d = digit - '0'; // convert to int
      ZORDER_SET_INDEX_DIGIT(z, r, d);
      r++;
   }

   DgZOrderCoord coord;
   coord.setValue(z);

   return coord;

} // DgZOrderCoord DgZOrderStringtoZOrderConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter
                (const DgRF<DgZOrderCoord, long long int>& from,
                 const DgRF<DgZOrderStringCoord, long long int>& to)
        : DgConverter<DgZOrderCoord, long long int, DgZOrderStringCoord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZOrderRF* zRF = dynamic_cast<const DgZOrderRF*>(&fromFrame());
   if (!zRF) {
      report("DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter(): "
         " fromFrame not of type DgZOrderRF", DgBase::Fatal);
   }

   const DgZOrderStringRF* zsRF = dynamic_cast<const DgZOrderStringRF*>(&toFrame());
   if (!zsRF) {
      report("DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter(): "
         " toFrame not of type DgZOrderStringRF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringCoord
DgZOrderToZOrderStringConverter::convertTypedAddress (const DgZOrderCoord& addIn) const
{
   uint64_t z = addIn.value();

   int quadNum = ZORDER_GET_QUADNUM(z);
   string s = dgg::util::to_string(quadNum, 2);

   for (int r = 1; r <= res_; r++) {
      // get the integer digit
      char d = ZORDER_GET_INDEX_DIGIT(z, r);
      // convert to char
      d += '0';
      // append to index string
      s += d;
   }

   DgZOrderStringCoord zStr;
   zStr.setValString(s);

   return zStr;

} // DgZOrderStringCoord DgZOrderToZOrderStringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
