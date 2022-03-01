/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

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

#include <cmath>
#include <climits>
#include <cfloat>

#include <dglib/DgZOrderRF.h>
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
   if (tok.length() != 15)
      report("DgZOrderRF::str2add(): valid ZORDER indexes are 15 digits long", 
             DgBase::Fatal);

   uint64_t val = 0;
   if (!sscanf(tok, "%" PRIx64, &val)
      report("DgZOrderRF::str2add(): invalid ZORDER index", DgBase::Fatal);

   if (!add) add = new DgZOrderCoord();
   add->setValue(val);

   // cleanup
   delete[] tmpStr;

   unsigned long offset = strlen(tok) + 1;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZOrderRF::str2add

////////////////////////////////////////////////////////////////////////////////
const string& 
DgZOrderRF::valString (void) const
{
   char str[17]; // max 16 digits plus 1 for the null terminator
   sprintf(str, "%" PRIx64, value_);
   return string(str);

} // const string& DgZOrderRF::valString

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter 
                (const DgRF<DgZOrderStringCoord, long long int>& from,
                 const DgRF<DgZOrderCoord, long long int>& to)
        : DgConverter<DgZOrderStringCoord, long long int, DgZOrderCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{ 
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&fromFrame());

   if (!pIDGG_) {
      report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
         " fromFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon) {
      report("DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter(): "
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

} // DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter 

////////////////////////////////////////////////////////////////////////////////
DgZOrderCoord 
DgZOrderStringtoZOrderConverter::convertTypedAddress (const DgZOrderStringCoord& addIn) const
{
//dgcout << " -> " << addIn << endl;

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
      int d = digit - '0'; // convert to int
      ZORDER_SET_INDEX_DIGIT(z, r, d);
      r++;
   }

   //dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << endl;

   DgZOrderCoord coord();
   coord.setValue(z);

   //dgcout << "coord: " << coord << endl;

   return coord;

} // DgZOrderCoord DgZOrderStringtoZOrderConverter::convertTypedAddress 

////////////////////////////////////////////////////////////////////////////////
DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter            
                (const DgRF<DgZOrderCoord, long long int>& from,
                 const DgRF<DgZOrderStringCoord, long long int>& to)
        : DgConverter<DgZOrderCoord, long long int, DgZOrderStringCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{ 
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&toFrame());

   if (!pIDGG_) {
      report("DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter(): "
         " toFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   if (IDGG().gridTopo() != Hexagon) {
      report("DgZOrderToZOrderStringConverter::DgZOrderToZOrderStringConverter(): "
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

} // DgZOrderStringtoZOrderConverter::DgZOrderStringtoZOrderConverter 

////////////////////////////////////////////////////////////////////////////////
DgZOrderStringCoord 
DgZOrderToZOrderStringConverter::convertTypedAddress (const DgZOrderCoord& addIn) const
{
   uint64_t z = addIn.value();

   int quadNum = ZORDER_GET_QUADNUM(z);
   string s = dgg::util::to_string(quadNum, 2);

   for (int r = 1; r <= MAX_ZORDER_RES; r++) {
      char digit = ZORDER_GET_INDEX_DIGIT(z, r);
      string d = string((char) (digit + '0')); 
      s += d;
   }

   DgZOrderStringCoord zStr;
   zStr.setValString(s);

//dgcout << "zorderStr " << zStr << endl;

   return zorder;

} // DgZOrderStringCoord DgZOrderToZOrderStringConverter::convertTypedAddress 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
