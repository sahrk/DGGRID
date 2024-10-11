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
// DgZ7RF.h: DgZ7RF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZ7RF_H
#define DGZ7RF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgZ7Coord;
class DgZ7StringRF;
class DgZ7StringCoord;
class DgIDGGBase;
class DgZ7Coord;

//using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgZ7StringtoZ7Converter :
        public DgConverter<DgZ7StringCoord, long long int, DgZ7Coord, long long int>
{
   public:

      DgZ7StringtoZ7Converter (const DgRF<DgZ7StringCoord, long long int>& from,
                                   const DgRF<DgZ7Coord, long long int>& to);

      virtual DgZ7Coord convertTypedAddress
                                (const DgZ7StringCoord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class DgZ7ToZ7StringConverter :
        public DgConverter<DgZ7Coord, long long int, DgZ7StringCoord, long long int>
{
   public:

      DgZ7ToZ7StringConverter (const DgRF<DgZ7Coord, long long int>& from,
                                   const DgRF<DgZ7StringCoord, long long int>& to);

      virtual DgZ7StringCoord convertTypedAddress
                                (const DgZ7Coord& addIn) const;

   private:

      int res_;
};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZ7ToStringConverter : public Dg2WayConverter {

   public:

      Dg2WayZ7ToStringConverter (
               const DgRF<DgZ7StringCoord, long long int>& fromFrame,
               const DgRF<DgZ7Coord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgZ7StringtoZ7Converter(fromFrame, toFrame)),
                            *(new DgZ7ToZ7StringConverter(toFrame, fromFrame)))
                                             {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 7 hex
//        level indicator), and digit-interleaved radix string
//
class DgZ7Coord  {

   public:

      static const DgZ7Coord undefDgZ7Coord;

      DgZ7Coord (void) : value_ (0) { }

      DgZ7Coord (uint64_t valIn) : value_ (valIn) { }

      DgZ7Coord (const DgZ7Coord& coord)
              { value_ = coord.value(); }

      void setValue (uint64_t value) { value_ = value; }

      uint64_t value (void) const { return value_; }

      operator std::string (void) const { return valString(); }

      std::string valString (void) const;

      bool operator== (const DgZ7Coord& c) const
          { return value() == c.value(); }

      bool operator!= (const DgZ7Coord& c) const
          { return !(*this == c); }

      DgZ7Coord& operator= (const DgZ7Coord& add)
          {
             if (add != *this) setValue(add.value());

             return *this;
          }

   private:

      uint64_t value_;
};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZ7Coord& coord)
{ return stream << std::string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZ7RF : public DgRF<DgZ7Coord, long long int> {

   public:

      static DgZ7RF* makeRF (DgRFNetwork& networkIn, const std::string& nameIn,
                                  int resIn)
         { return new DgZ7RF (networkIn, nameIn, resIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return 7; }

      virtual long long int dist (const DgZ7Coord& add1,
                        const DgZ7Coord& add2) const
                       { return 0; }

      virtual std::string add2str (const DgZ7Coord& add) const
                       { return std::string(add); }

      virtual std::string add2str (const DgZ7Coord& add, char delimiter)
                                                                         const
                       { return std::string(add); }

      virtual const char* str2add (DgZ7Coord* add, const char* str,
                                   char delimiter) const;

      virtual std::string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZ7Coord& undefAddress (void) const
                            { return DgZ7Coord::undefDgZ7Coord; }

   protected:

      DgZ7RF (DgRFNetwork& networkIn, const std::string& nameIn, int resIn);

      int res_;
      const DgZ7StringRF* z7strRF_;
      const DgZ7StringtoZ7Converter* z7strToZ7_;
      const DgZ7ToZ7StringConverter* z7toZ7str_;

   friend class DgZ7StringtoZ7Converter;
};

////////////////////////////////////////////////////////////////////////////////
#endif
