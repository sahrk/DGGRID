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
// DgZOrderRF.h: DgZOrderRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZORDERRF_H
#define DGZORDERRF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgQ2DICoord;
class DgIDGGBase;
class DgZOrderCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgQ2DItoZOrderConverter :
        public DgConverter<DgQ2DICoord, long long int, DgZOrderCoord, long long int>
{
   public:

      DgQ2DItoZOrderConverter (const DgRF<DgQ2DICoord, long long int>& from,
                                   const DgRF<DgZOrderCoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgZOrderCoord convertTypedAddress
                                (const DgQ2DICoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class DgZOrderToQ2DIConverter :
        public DgConverter<DgZOrderCoord, long long int, DgQ2DICoord, long long int>
{
   public:

      DgZOrderToQ2DIConverter (const DgRF<DgZOrderCoord, long long int>& from,
                                   const DgRF<DgQ2DICoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgQ2DICoord convertTypedAddress
                                (const DgZOrderCoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZOrderConverter : public Dg2WayConverter {

   public:

      Dg2WayZOrderConverter (const DgRF<DgQ2DICoord, long long int>& fromFrame,
                                 const DgRF<DgZOrderCoord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgQ2DItoZOrderConverter(fromFrame, toFrame)),
                            *(new DgZOrderToQ2DIConverter(toFrame, fromFrame)))
                                             {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and radix string
//
class DgZOrderCoord  {

   public:

      static const DgZOrderCoord undefDgZOrderCoord;

      DgZOrderCoord (void) { }

      DgZOrderCoord (const DgZOrderCoord& coord)
              { valString_ = coord.valString(); }

      void setValString (const string strIn) { valString_ = strIn; }

      const string& valString (void) const { return valString_; }

      operator string (void) const { return valString(); }

      bool operator== (const DgZOrderCoord& c) const
          { return valString() == c.valString(); }

      bool operator!= (const DgZOrderCoord& c) const
          { return !(*this == c); }

      DgZOrderCoord& operator= (const DgZOrderCoord& add)
          {
             if (add != *this) setValString(add.valString());

             return *this;
          }

   private:

      string valString_;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZOrderCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZOrderRF : public DgRF<DgZOrderCoord, long long int> {

   public:

      static DgZOrderRF* makeRF (DgRFNetwork& networkIn, const string& nameIn)
         { return new DgZOrderRF (networkIn, nameIn); }

      virtual long long int dist (const DgZOrderCoord& add1,
                        const DgZOrderCoord& add2) const
                       { return 0; }

      virtual string add2str (const DgZOrderCoord& add) const
                       { return string(add); }

      virtual string add2str (const DgZOrderCoord& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual const char* str2add (DgZOrderCoord* add, const char* str,
                                   char delimiter) const;

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZOrderCoord& undefAddress (void) const
                       { static DgZOrderCoord undef; return undef; }

   protected:

      DgZOrderRF (DgRFNetwork& networkIn, const string& nameIn)
         : DgRF<DgZOrderCoord, long long int>(networkIn, nameIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
