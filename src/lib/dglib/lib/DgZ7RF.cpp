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
// DgZ7RF.cpp: DgZ7RF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <climits>
#include <cfloat>
#include <inttypes.h>
#include <string.h>

#include <dglib/DgZ7RF.h>
#include <dglib/DgZ7StringRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgRadixString.h>

/** max ZORDER resolution */
#define MAX_ZORDER_RES 30

/** The number of bits in a Z7 index. */
#define ZORDER_NUM_BITS 64

/** The bit offset of the max resolution digit in a Z7 index. */
#define ZORDER_MAX_OFFSET 63

/** The bit offset of the quad number in a Z7 index. */
#define ZORDER_QUAD_OFFSET 60

/** 1's in the 4 quad number bits, 0's everywhere else. */
#define ZORDER_QUAD_MASK ((uint64_t)(15) << ZORDER_QUAD_OFFSET)

/** 0's in the 4 mode bits, 1's everywhere else. */
#define ZORDER_QUAD_MASK_NEGATIVE (~ZORDER_QUAD_MASK)

/** The number of bits in a single ZORDER resolution digit. */
#define ZORDER_PER_DIGIT_OFFSET 2

/** 1's in the 2 bits of highest res digit bits, 0's everywhere else. */
#define ZORDER_DIGIT_MASK ((uint64_t)(3))

/** Gets the integer quad number of a Z7 index. */
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
const DgZ7Coord DgZ7Coord::undefDgZ7Coord(0xffffffffffffffff);

////////////////////////////////////////////////////////////////////////////////
const char*
DgZ7RF::str2add (DgZ7Coord* add, const char* str,
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
      report("DgZ7RF::str2add(): invalid ZORDER index", DgBase::Fatal);

   if (!add) add = new DgZ7Coord();
   add->setValue(val);

   unsigned long offset = strlen(tok) + 1;
   delete[] tmpStr;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZ7RF::str2add

////////////////////////////////////////////////////////////////////////////////
string
DgZ7Coord::valString (void) const
{
   const int maxStrSize = 17; // max 16 digits plus 1 for the null terminator
   char str[maxStrSize];
   snprintf(str, maxStrSize, "%016" PRIx64, value_);

   return string(str);

} // const string& DgZ7RF::valString

////////////////////////////////////////////////////////////////////////////////
DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter
                (const DgRF<DgZ7StringCoord, long long int>& from,
                 const DgRF<DgZ7Coord, long long int>& to)
   : DgConverter<DgZ7StringCoord, long long int, DgZ7Coord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZ7StringRF* zsRF = dynamic_cast<const DgZ7StringRF*>(&fromFrame());
   if (!zsRF) {
      report("DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter(): "
         " fromFrame not of type DgZ7StringRF", DgBase::Fatal);
   }

   const DgZ7RF* zRF = dynamic_cast<const DgZ7RF*>(&toFrame());
   if (!zRF) {
      report("DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter(): "
         " toFrame not of type DgZ7RF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter

////////////////////////////////////////////////////////////////////////////////
DgZ7Coord
DgZ7StringtoZ7Converter::convertTypedAddress (const DgZ7StringCoord& addIn) const
{
   printf("DgZ7StringtoZ7Converter::convertTypedAddress()\n"); 

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
         report("DgZ7StringtoZ7Converter::convertTypedAddress(): "
         " incoming index exceeds converter resolution", DgBase::Fatal);

      int d = digit - '0'; // convert to int
      ZORDER_SET_INDEX_DIGIT(z, r, d);
      r++;
   }

   DgZ7Coord coord;
   coord.setValue(z);

   return coord;

} // DgZ7Coord DgZ7StringtoZ7Converter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
DgZ7ToZ7StringConverter::DgZ7ToZ7StringConverter
                (const DgRF<DgZ7Coord, long long int>& from,
                 const DgRF<DgZ7StringCoord, long long int>& to)
        : DgConverter<DgZ7Coord, long long int, DgZ7StringCoord, long long int> (from, to)
{
   // validate the to/from RF's
   const DgZ7RF* zRF = dynamic_cast<const DgZ7RF*>(&fromFrame());
   if (!zRF) {
      report("DgZ7ToZ7StringConverter::DgZ7ToZ7StringConverter(): "
         " fromFrame not of type DgZ7RF", DgBase::Fatal);
   }

   const DgZ7StringRF* zsRF = dynamic_cast<const DgZ7StringRF*>(&toFrame());
   if (!zsRF) {
      report("DgZ7ToZ7StringConverter::DgZ7ToZ7StringConverter(): "
         " toFrame not of type DgZ7StringRF", DgBase::Fatal);
   }

   if (zsRF->aperture() != zRF->aperture() || zsRF->res() != zRF->res()) {
      report("DgZ7ToZ7StringConverter::DgZ7ToZ7StringConverter(): "
         " fromFrame and toFrame apertures or resolutions do not match", DgBase::Fatal);
   }

   // store the res
   res_ = zRF->res();

} // DgZ7StringtoZ7Converter::DgZ7StringtoZ7Converter

////////////////////////////////////////////////////////////////////////////////
DgZ7StringCoord
DgZ7ToZ7StringConverter::convertTypedAddress (const DgZ7Coord& addIn) const
{
   printf("DgZ7ToZ7StringConverter::convertTypedAddress\n");

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

   DgZ7StringCoord zStr;
   zStr.setValString(s);

   return zStr;

} // DgZ7StringCoord DgZ7ToZ7StringConverter::convertTypedAddress

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
