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
// DgZ3RF.cpp: DgZ3RF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cmath>
#include <climits>
#include <cfloat>
#include <string.h>

#include <dglib/DgZ3RF.h>
#include <dglib/DgZ3StringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgRadixString.h>

/** max Z3 resolution */
#define MAX_Z3_RES 30

/** The number of bits in a Z3 index. */
#define Z3_NUM_BITS 64

/** The bit offset of the max resolution digit in a Z3 index. */
#define Z3_MAX_OFFSET 63

/** The bit offset of the quad number in a Z3 index. */
#define Z3_QUAD_OFFSET 60

/** 1's in the 4 quad number bits, 0's everywhere else. */
#define Z3_QUAD_MASK ((uint64_t)(15) << Z3_QUAD_OFFSET)

/** 0's in the 4 mode bits, 1's everywhere else. */
#define Z3_QUAD_MASK_NEGATIVE (~Z3_QUAD_MASK)

/** The number of bits in a single Z3 resolution digit. */
#define Z3_PER_DIGIT_OFFSET 2

/** 1's in the 2 bits of highest res digit bits, 0's everywhere else. */
#define Z3_DIGIT_MASK ((uint64_t)(3))

/** Gets the integer quad number of a Z3 index. */
#define Z3_GET_QUADNUM(z) ((int)((((z)&Z3_QUAD_MASK) >> Z3_QUAD_OFFSET)))

/** Sets the integer mode of z to v. */
#define Z3_SET_QUADNUM(z, v) \
    (z) = (((z)&Z3_QUAD_MASK_NEGATIVE) | (((uint64_t)(v)) << Z3_QUAD_OFFSET))

/** Gets the resolution res integer digit of z. */
#define Z3_GET_INDEX_DIGIT(z, res)                                        \
    ((int)((((z) >> ((MAX_Z3_RES - (res)) * Z3_PER_DIGIT_OFFSET)) & \
                  Z3_DIGIT_MASK)))

/** Sets the resolution res digit of z to the integer digit */
#define Z3_SET_INDEX_DIGIT(z, res, digit)                                  \
    (z) = (((z) & ~((Z3_DIGIT_MASK                                        \
                       << ((MAX_Z3_RES - (res)) * Z3_PER_DIGIT_OFFSET)))) | \
            (((uint64_t)(digit))                                            \
             << ((MAX_Z3_RES - (res)) * Z3_PER_DIGIT_OFFSET)))

////////////////////////////////////////////////////////////////////////////////
const DgZ3Coord DgZ3Coord::undefDgZ3Coord(0xffffffffffffffff);

////////////////////////////////////////////////////////////////////////////////
DgZ3RF::DgZ3RF (DgRFNetwork& networkIn, const std::string& nameIn, int resIn)
         : DgRF<DgZ3Coord, long long int>(networkIn, nameIn), res_ (resIn)
{
}

////////////////////////////////////////////////////////////////////////////////
const char*
DgZ3RF::str2add (DgZ3Coord* add, const char* str,
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
      report("DgZ3RF::str2add(): invalid Z3 index", DgBase::Fatal);

   if (!add) add = new DgZ3Coord();
   add->setValue(val);

   unsigned long offset = strlen(tok) + 1;
   delete[] tmpStr;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZ3RF::str2add

////////////////////////////////////////////////////////////////////////////////
string
DgZ3Coord::valString (void) const
{
   const int maxStrSize = 17; // max 16 digits plus 1 for the null terminator
   char str[maxStrSize];
   snprintf(str, maxStrSize, "%016" PRIx64, value_);

   return string(str);

} // const string& DgZ3RF::valString

////////////////////////////////////////////////////////////////////////////////
DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter
                (const DgRF<DgZ3StringCoord, long long int>& from,
                 const DgRF<DgZ3Coord, long long int>& to)
   : DgConverter<DgZ3StringCoord, long long int, DgZ3Coord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZ3StringRF* zsRF = dynamic_cast<const DgZ3StringRF*>(&fromFrame());
   if (!zsRF) {
      report("DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter(): "
         " fromFrame not of type DgZ3StringRF", DgBase::Fatal);
   }

   const DgZ3RF* zRF = dynamic_cast<const DgZ3RF*>(&toFrame());
   if (!zRF) {
      report("DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter(): "
         " toFrame not of type DgZ3RF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter

////////////////////////////////////////////////////////////////////////////////
DgZ3Coord
DgZ3StringtoZ3Converter::convertTypedAddress (const DgZ3StringCoord& addIn) const
{
   if (res_ > MAX_Z3_RES) {
      report("DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter(): "
         " input resolution exceeds max Z3 resolution of 30", DgBase::Fatal);
   }

   string addstr = addIn.valString();
   uint64_t z = 0;

   // first get the quad number and add to the val
   string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);
   Z3_SET_QUADNUM(z, quadNum);

   int index = 2; // skip the two quad digits

   // the rest is the radix string
   string radStr = addstr.substr(index);

   // now get the digits
   int r = 1;
   for (const char& digit: radStr) {
      if (r > res_)
         report("DgZ3StringtoZ3Converter::convertTypedAddress(): "
         " incoming index exceeds converter resolution", DgBase::Fatal);

      int d = digit - '0'; // convert to int
      Z3_SET_INDEX_DIGIT(z, r, d);
      r++;
   }

   DgZ3Coord coord;
   coord.setValue(z);

   return coord;

} // DgZ3Coord DgZ3StringtoZ3Converter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZ3ToZ3StringConverter::DgZ3ToZ3StringConverter
                (const DgRF<DgZ3Coord, long long int>& from,
                 const DgRF<DgZ3StringCoord, long long int>& to)
        : DgConverter<DgZ3Coord, long long int, DgZ3StringCoord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZ3RF* zRF = dynamic_cast<const DgZ3RF*>(&fromFrame());
   if (!zRF) {
      report("DgZ3ToZ3StringConverter::DgZ3ToZ3StringConverter(): "
         " fromFrame not of type DgZ3RF", DgBase::Fatal);
   }

   const DgZ3StringRF* zsRF = dynamic_cast<const DgZ3StringRF*>(&toFrame());
   if (!zsRF) {
      report("DgZ3ToZ3StringConverter::DgZ3ToZ3StringConverter(): "
         " toFrame not of type DgZ3StringRF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZ3ToZ3StringConverter::DgZ3ToZ3StringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZ3StringtoZ3Converter::DgZ3StringtoZ3Converter

////////////////////////////////////////////////////////////////////////////////
DgZ3StringCoord
DgZ3ToZ3StringConverter::convertTypedAddress (const DgZ3Coord& addIn) const
{
   uint64_t z = addIn.value();

   int quadNum = Z3_GET_QUADNUM(z);
   string s = dgg::util::to_string(quadNum, 2);

   for (int r = 1; r <= res_; r++) {
      // get the integer digit
      char d = Z3_GET_INDEX_DIGIT(z, r);
      // convert to char
      d += '0';
      // append to index string
      s += d;
   }

   DgZ3StringCoord zStr;
   zStr.setValString(s);

   return zStr;

} // DgZ3StringCoord DgZ3ToZ3StringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
