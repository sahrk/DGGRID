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
const char*
DgZOrderRF::str2add (DgZOrderCoord* add, const char* str, 
                         char delimiter) const
{
   if (!add) add = new DgZOrderCoord();

   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);
   char* tok = strtok(tmpStr, delimStr);

   add->setValString(tok);

   delete[] tmpStr;

   unsigned long offset = strlen(tok) + 1;
   if (offset >= strlen(str)) return 0;
   else return &str[offset];

} // const char* DgZOrderRF::str2add

////////////////////////////////////////////////////////////////////////////////
DgQ2DItoZOrderConverter::DgQ2DItoZOrderConverter 
                (const DgRF<DgQ2DICoord, long long int>& from,
                 const DgRF<DgZOrderCoord, long long int>& to)
        : DgConverter<DgQ2DICoord, long long int, DgZOrderCoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{ 
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&fromFrame());

   if (!pIDGG_)
   {
      report("DgQ2DItoZOrderConverter::DgQ2DItoZOrderConverter(): "
         " fromFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = IDGG().radix();   // effective radix
   if (IDGG().aperture() == 3)    
   {
       effRadix_ = 3;
       effRes_ = (effRes_ + 1) / 2;
   }

   if (IDGG().gridTopo() == Triangle) effRes_++; // adjust for long double j

} // DgQ2DItoZOrderConverter::DgQ2DItoZOrderConverter 

////////////////////////////////////////////////////////////////////////////////
DgZOrderCoord 
DgQ2DItoZOrderConverter::convertTypedAddress 
                                               (const DgQ2DICoord& addIn) const
{
   string qstr = dgg::util::to_string(addIn.quadNum(), 2);

dgcout << "** addIn " << addIn << endl;
   DgRadixString rs1(effRadix_, (int) addIn.coord().i(), effRes_);
   DgRadixString rs2(effRadix_, (int) addIn.coord().j(), effRes_);

dgcout << "rs1 " << rs1 << endl;
dgcout << "rs2 " << rs2 << endl;

   string addstr = qstr;
   if (IDGG().aperture() == 3)
   {
     if (IDGG().isClassI()) addstr = addstr + string("0");
     else addstr = addstr + string("1");
   }

   addstr = addstr + DgRadixString::digitZOrder(rs1, rs2);

dgcout << "addstr " << addstr << endl;

   DgZOrderCoord zorder;
   zorder.setValString(addstr);
dgcout << "zorder " << zorder << endl;

   return zorder;

} // DgZOrderCoord DgQ2DItoZOrderConverter::convertTypedAddress 

////////////////////////////////////////////////////////////////////////////////
DgZOrderToQ2DIConverter::DgZOrderToQ2DIConverter            
                (const DgRF<DgZOrderCoord, long long int>& from,
                 const DgRF<DgQ2DICoord, long long int>& to)
        : DgConverter<DgZOrderCoord, long long int, DgQ2DICoord, long long int> (from, to),
          pIDGG_ (NULL), effRes_ (0), effRadix_ (0)
{ 
   pIDGG_ = dynamic_cast<const DgIDGGBase*>(&toFrame());

   if (!pIDGG_)
   {
      report("DgZOrderToQ2DIConverter::DgZOrderToQ2DIConverter(): "
         " toFrame not of type DgIDGGBase", DgBase::Fatal);
   }

   effRes_ = IDGG().res();       // effective resolution
   effRadix_ = IDGG().radix();   // effective radix
   if (IDGG().aperture() == 3)    
   {
       effRadix_ = 3;
       effRes_ = (effRes_ + 1) / 2;
   }

   if (IDGG().gridTopo() == Triangle) effRes_++; // adjust for long double j

} // DgQ2DItoZOrderConverter::DgQ2DItoZOrderConverter 

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord 
DgZOrderToQ2DIConverter::convertTypedAddress 
                                       (const DgZOrderCoord& addIn) const
{
dgcout << " -> " << addIn << endl;
   string addstr = addIn.valString();

   // first get the quad number
   string qstr = addstr.substr(0, 2);
   if (qstr[0] == '0') // leading 0
      qstr = qstr.substr(1, 1);
   int quadNum = std::stoi(qstr);

   int index = 2; // skip the two quad digits

   // check for special aperture leading character
   if (IDGG().aperture() == 3) {
      // validate the leading character
      if (IDGG().isClassI()) {
         if (addstr[index] != '0') 
            report(string("invalid zorder index \'") + addstr 
              + string("\'; Class I aperture 3 DGG index must have a leading 0"), 
              DgBase::Fatal);
      } else if (addstr[index] != '1') {
            report(string("invalid zorder index \'") + addstr 
              + string("\'; Class II aperture 3 DGG index must have a leading 1"), 
              DgBase::Fatal);
      }

      index++; // expended first character
   }

   // the rest is the radix string
   string radStr = addstr.substr(index);

   // split out the interleaved digits
   string radStr1 = "";
   string radStr2 = "";
   for (const char& digit: radStr) {

      // break out the interleaved digits
      int d = digit - '0'; // convert to int
      int c1 = (int) (d / effRadix_);

      int c2 = (d % effRadix_); 

      radStr1 += dgg::util::to_string(c1);
      radStr2 += dgg::util::to_string(c2);
   }

   DgRadixString rad1(effRadix_, radStr1);
   DgRadixString rad2(effRadix_, radStr2);

   dgcout << "qstr: " << qstr << " rad1: " << rad1 << " rad2: " << rad2 << endl;

   DgQ2DICoord q2di(quadNum, DgIVec2D(rad1.value(), rad2.value()));
   dgcout << "q2di: " << q2di << endl;

   return q2di;

} // DgQ2DICoord DgZOrderToQ2DIConverter::convertTypedAddress 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
