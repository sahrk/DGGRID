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
// DgInterleaveRF.h: DgInterleaveRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGINTERLEAVERF_H
#define DGINTERLEAVERF_H

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgContCartRF.h>
#include <dglib/DgConverter.h>
#include <dglib/DgDiscRF2D.h>
#include <dglib/DgDiscRFS2D.h>
#include <dglib/DgDVec2D.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgHexC1Grid2D.h>
#include <dglib/DgIcosaMap.h>
#include <dglib/DgIcosaProj.h>
#include <dglib/DgIVec2D.h>
#include <dglib/DgLocation.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgProjTriRF.h>
#include <dglib/DgRF.h>
#include <dglib/DgUtil.h>

#include <climits>
#include <iostream>

class DgQ2DICoord;
class DgPolygon;
class DgBoundedIDGG;
class DgIDGGBase;
class DgInterleaveCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgQ2DItoInterleaveConverter :
        public DgConverter<DgQ2DICoord, long long int, DgInterleaveCoord, long long int>
{
   public:

      DgQ2DItoInterleaveConverter (const DgRF<DgQ2DICoord, long long int>& from,
                                   const DgRF<DgInterleaveCoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgInterleaveCoord convertTypedAddress
                                (const DgQ2DICoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class DgInterleaveToQ2DIConverter :
        public DgConverter<DgInterleaveCoord, long long int, DgQ2DICoord, long long int>
{
   public:

      DgInterleaveToQ2DIConverter (const DgRF<DgInterleaveCoord, long long int>& from,
                                   const DgRF<DgQ2DICoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgQ2DICoord convertTypedAddress
                                (const DgInterleaveCoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and radix string
//
class DgInterleaveCoord  {

   public:

      static const DgInterleaveCoord undefDgInterleaveCoord;

      DgInterleaveCoord (void) { }

      DgInterleaveCoord (const DgInterleaveCoord& coord)
              { valString_ = coord.valString(); }

      void setValString (const string strIn) { valString_ = strIn; }

      const string& valString (void) const { return valString_; }

      operator string (void) const { return valString(); }

      bool operator== (const DgInterleaveCoord& c) const
          { return valString() == c.valString(); }

      bool operator!= (const DgInterleaveCoord& c) const
          { return !(*this == c); }

      DgInterleaveCoord& operator= (const DgInterleaveCoord& add)
          {
             if (add != *this) setValString(add.valString());

             return *this;
          }

   private:

      string valString_;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgInterleaveCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgInterleaveRF : public DgRF<DgInterleaveCoord, long long int> {

   public:

      static DgInterleaveRF* makeRF (DgRFNetwork& networkIn, const string& nameIn)
         { return new DgInterleaveRF (networkIn, nameIn); }

      virtual long long int dist (const DgInterleaveCoord& add1,
                        const DgInterleaveCoord& add2) const
                       { return 0; }

      virtual string add2str (const DgInterleaveCoord& add) const
                       { return string(add); }

      virtual string add2str (const DgInterleaveCoord& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual const char* str2add (DgInterleaveCoord* add, const char* str,
                                   char delimiter) const;

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgInterleaveCoord& undefAddress (void) const
                       { static DgInterleaveCoord undef; return undef; }

   protected:

      DgInterleaveRF (DgRFNetwork& networkIn, const string& nameIn)
         : DgRF<DgInterleaveCoord, long long int>(networkIn, nameIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
