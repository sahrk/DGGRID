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
// DgZOrderRF.h: DgZOrderRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZORDERRF_H
#define DGZORDERRF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgZOrderCoord;
class DgZOrderStringCoord;
class DgIDGGBase;
class DgZOrderCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgZOrderStringtoZOrderConverter :
        public DgConverter<DgZOrderStringCoord, long long int, DgZOrderCoord, long long int>
{
   public:

      DgZOrderStringtoZOrderConverter (const DgRF<DgZOrderStringCoord, long long int>& from,
                                   const DgRF<DgZOrderCoord, long long int>& to);

      virtual DgZOrderCoord convertTypedAddress
                                (const DgZOrderStringCoord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class DgZOrderToZOrderStringConverter :
        public DgConverter<DgZOrderCoord, long long int, DgZOrderStringCoord, long long int>
{
   public:

      DgZOrderToZOrderStringConverter (const DgRF<DgZOrderCoord, long long int>& from,
                                   const DgRF<DgZOrderStringCoord, long long int>& to);

      virtual DgZOrderStringCoord convertTypedAddress
                                (const DgZOrderCoord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZOrderToStringConverter : public Dg2WayConverter {

   public:

      Dg2WayZOrderToStringConverter (
               const DgRF<DgZOrderStringCoord, long long int>& fromFrame,
               const DgRF<DgZOrderCoord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgZOrderStringtoZOrderConverter(fromFrame, toFrame)),
                            *(new DgZOrderToZOrderStringConverter(toFrame, fromFrame)))
                                             {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and digit-interleaved radix string
//
class DgZOrderCoord  {

   public:

      static const DgZOrderCoord undefDgZOrderCoord;

      DgZOrderCoord (void) : value_ (0) { }

      DgZOrderCoord (uint64_t valIn) : value_ (valIn) { }

      DgZOrderCoord (const DgZOrderCoord& coord)
              { value_ = coord.value(); }

      void setValue (uint64_t value) { value_ = value; }

      uint64_t value (void) const { return value_; }

      operator string (void) const { return valString(); }

      string valString (void) const;

      bool operator== (const DgZOrderCoord& c) const
          { return value() == c.value(); }

      bool operator!= (const DgZOrderCoord& c) const
          { return !(*this == c); }

      DgZOrderCoord& operator= (const DgZOrderCoord& add)
          {
             if (add != *this) setValue(add.value());

             return *this;
          }

   private:

      uint64_t value_;
};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZOrderCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZOrderRF : public DgRF<DgZOrderCoord, long long int> {

   public:

      static DgZOrderRF* makeRF (DgRFNetwork& networkIn, const string& nameIn,
                                  int resIn, int apertureIn)
         { return new DgZOrderRF (networkIn, nameIn, resIn, apertureIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

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
                            { return DgZOrderCoord::undefDgZOrderCoord; }

   protected:

      DgZOrderRF (DgRFNetwork& networkIn, const string& nameIn,
                    int resIn, int apertureIn)
         : DgRF<DgZOrderCoord, long long int>(networkIn, nameIn),
           res_ (resIn), aperture_ (apertureIn) { }

      int res_;
      int aperture_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
