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
// DgZ3RF.h: DgZ3RF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZ3RF_H
#define DGZ3RF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgZ3Coord;
class DgZ3StringCoord;
class DgIDGGBase;
class DgZ3Coord;

//using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgZ3StringtoZ3Converter :
        public DgConverter<DgZ3StringCoord, long long int, DgZ3Coord, long long int>
{
   public:

      DgZ3StringtoZ3Converter (const DgRF<DgZ3StringCoord, long long int>& from,
                                   const DgRF<DgZ3Coord, long long int>& to);

      virtual DgZ3Coord convertTypedAddress
                                (const DgZ3StringCoord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class DgZ3ToZ3StringConverter :
        public DgConverter<DgZ3Coord, long long int, DgZ3StringCoord, long long int>
{
   public:

      DgZ3ToZ3StringConverter (const DgRF<DgZ3Coord, long long int>& from,
                                   const DgRF<DgZ3StringCoord, long long int>& to);

      virtual DgZ3StringCoord convertTypedAddress
                                (const DgZ3Coord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZ3ToStringConverter : public Dg2WayConverter {

   public:

      Dg2WayZ3ToStringConverter (
               const DgRF<DgZ3StringCoord, long long int>& fromFrame,
               const DgRF<DgZ3Coord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgZ3StringtoZ3Converter(fromFrame, toFrame)),
                            *(new DgZ3ToZ3StringConverter(toFrame, fromFrame)))
                                             {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and digit-interleaved radix string
//
class DgZ3Coord  {

   public:

      static const DgZ3Coord undefDgZ3Coord;

      DgZ3Coord (void) : value_ (0) { }

      DgZ3Coord (uint64_t valIn) : value_ (valIn) { }

      DgZ3Coord (const DgZ3Coord& coord)
              { value_ = coord.value(); }

      void setValue (uint64_t value) { value_ = value; }

      uint64_t value (void) const { return value_; }

      operator std::string (void) const { return valString(); }

      std::string valString (void) const;

      bool operator== (const DgZ3Coord& c) const
          { return value() == c.value(); }

      bool operator!= (const DgZ3Coord& c) const
          { return !(*this == c); }

      DgZ3Coord& operator= (const DgZ3Coord& add)
          {
             if (add != *this) setValue(add.value());

             return *this;
          }

   private:

      uint64_t value_;
};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZ3Coord& coord)
{ return stream << std::string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZ3RF : public DgRF<DgZ3Coord, long long int> {

   public:

      static DgZ3RF* makeRF (DgRFNetwork& networkIn, const std::string& nameIn,
                                  int resIn)
         { return new DgZ3RF (networkIn, nameIn, resIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return 3; }

      virtual long long int dist (const DgZ3Coord& add1,
                        const DgZ3Coord& add2) const
                       { return 0; }

      virtual std::string add2str (const DgZ3Coord& add) const
                       { return std::string(add); }

      virtual std::string add2str (const DgZ3Coord& add, char delimiter)
                                                                         const
                       { return std::string(add); }

      virtual const char* str2add (DgZ3Coord* add, const char* str,
                                   char delimiter) const;

      virtual std::string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZ3Coord& undefAddress (void) const
                            { return DgZ3Coord::undefDgZ3Coord; }

   protected:

      DgZ3RF (DgRFNetwork& networkIn, const std::string& nameIn, int resIn);

      int res_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
