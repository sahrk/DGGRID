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
// DgZ7StringRF.h: DgZ7StringRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZ7STRINGRF_H
#define DGZ7STRINGRF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgQ2DICoord;
class DgIDGGBase;
class DgZ7Coord;
class DgZ7StringCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgQ2DItoZ7StringConverter :
        public DgConverter<DgQ2DICoord, long long int, DgZ7StringCoord, long long int>
{
   public:

      DgQ2DItoZ7StringConverter (const DgRF<DgQ2DICoord, long long int>& from,
                                   const DgRF<DgZ7StringCoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgZ7StringCoord convertTypedAddress
                                (const DgQ2DICoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class DgZ7StringToQ2DIConverter :
        public DgConverter<DgZ7StringCoord, long long int, DgQ2DICoord, long long int>
{
   public:

      DgZ7StringToQ2DIConverter (const DgRF<DgZ7StringCoord, long long int>& from,
                                   const DgRF<DgQ2DICoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgQ2DICoord convertTypedAddress
                                (const DgZ7StringCoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int res_;
      int numClassI_;
      unsigned long long int unitScaleClassIres_;
};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZ7StringConverter : public Dg2WayConverter {

   public:

      Dg2WayZ7StringConverter (const DgRF<DgQ2DICoord, long long int>& fromFrame,
              const DgRF<DgZ7StringCoord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgQ2DItoZ7StringConverter(fromFrame, toFrame)),
              *(new DgZ7StringToQ2DIConverter(toFrame, fromFrame))) {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and digit-interleaved radix string
//
class DgZ7StringCoord  {

   public:

      static const DgZ7StringCoord undefDgZ7StringCoord;

      DgZ7StringCoord (void) { }

      DgZ7StringCoord (const string& valStrIn)
         : valString_ (valStrIn) { }

      DgZ7StringCoord (const DgZ7StringCoord& coord)
              { valString_ = coord.valString(); }

      void setValString (const string strIn) { valString_ = strIn; }

      const string& valString (void) const { return valString_; }

      operator string (void) const { return valString(); }

      bool operator== (const DgZ7StringCoord& c) const
          { return valString() == c.valString(); }

      bool operator!= (const DgZ7StringCoord& c) const
          { return !(*this == c); }

      DgZ7StringCoord& operator= (const DgZ7StringCoord& add)
          {
             if (add != *this) setValString(add.valString());

             return *this;
          }

   private:

      string valString_;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZ7StringCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZ7StringRF : public DgRF<DgZ7StringCoord, long long int> {

   public:

      static DgZ7StringRF* makeRF (DgRFNetwork& networkIn,
               const string& nameIn, int resIn)
         { return new DgZ7StringRF (networkIn, nameIn, resIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return 7; }

      virtual long long int dist (const DgZ7StringCoord& add1,
                        const DgZ7StringCoord& add2) const
                       { return 0; }

      virtual string add2str (const DgZ7StringCoord& add) const
                       { return string(add); }

      virtual string add2str (const DgZ7StringCoord& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual const char* str2add (DgZ7StringCoord* add, const char* str,
                                   char delimiter) const;

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZ7StringCoord& undefAddress (void) const
                       { return DgZ7StringCoord::undefDgZ7StringCoord; }

   protected:

      DgZ7StringRF (DgRFNetwork& networkIn, const string& nameIn,
                         int resIn)
         : DgRF<DgZ7StringCoord, long long int>(networkIn, nameIn),
           res_ (resIn) { }

      int res_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
