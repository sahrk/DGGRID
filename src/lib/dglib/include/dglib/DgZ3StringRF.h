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
// DgZ3StringRF.h: DgZ3StringRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZ3STRINGRF_H
#define DGZ3STRINGRF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgQ2DICoord;
class DgIDGGBase;
class DgZ3Coord;
class DgZ3StringCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgQ2DItoZ3StringConverter :
        public DgConverter<DgQ2DICoord, long long int, DgZ3StringCoord, long long int>
{
   public:

      DgQ2DItoZ3StringConverter (const DgRF<DgQ2DICoord, long long int>& from,
                                   const DgRF<DgZ3StringCoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgZ3StringCoord convertTypedAddress
                                (const DgQ2DICoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class DgZ3StringToQ2DIConverter :
        public DgConverter<DgZ3StringCoord, long long int, DgQ2DICoord, long long int>
{
   public:

      DgZ3StringToQ2DIConverter (const DgRF<DgZ3StringCoord, long long int>& from,
                                   const DgRF<DgQ2DICoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgQ2DICoord convertTypedAddress
                                (const DgZ3StringCoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZ3StringConverter : public Dg2WayConverter {

   public:

      Dg2WayZ3StringConverter (const DgRF<DgQ2DICoord, long long int>& fromFrame,
              const DgRF<DgZ3StringCoord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgQ2DItoZ3StringConverter(fromFrame, toFrame)),
              *(new DgZ3StringToQ2DIConverter(toFrame, fromFrame))) {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and digit-interleaved radix string
//
class DgZ3StringCoord  {

   public:

      static const DgZ3StringCoord undefDgZ3StringCoord;

      DgZ3StringCoord (void) { }

      DgZ3StringCoord (const string& valStrIn)
         : valString_ (valStrIn) { }

      DgZ3StringCoord (const DgZ3StringCoord& coord)
              { valString_ = coord.valString(); }

      void setValString (const string strIn) { valString_ = strIn; }

      const string& valString (void) const { return valString_; }

      operator string (void) const { return valString(); }

      bool operator== (const DgZ3StringCoord& c) const
          { return valString() == c.valString(); }

      bool operator!= (const DgZ3StringCoord& c) const
          { return !(*this == c); }

      DgZ3StringCoord& operator= (const DgZ3StringCoord& add)
          {
             if (add != *this) setValString(add.valString());

             return *this;
          }

   private:

      string valString_;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZ3StringCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZ3StringRF : public DgRF<DgZ3StringCoord, long long int> {

   public:

      static DgZ3StringRF* makeRF (DgRFNetwork& networkIn,
               const string& nameIn, int resIn)
         { return new DgZ3StringRF (networkIn, nameIn, resIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return 3; }

      virtual long long int dist (const DgZ3StringCoord& add1,
                        const DgZ3StringCoord& add2) const
                       { return 0; }

      virtual string add2str (const DgZ3StringCoord& add) const
                       { return string(add); }

      virtual string add2str (const DgZ3StringCoord& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual const char* str2add (DgZ3StringCoord* add, const char* str,
                                   char delimiter) const;

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZ3StringCoord& undefAddress (void) const
                       { return DgZ3StringCoord::undefDgZ3StringCoord; }

   protected:

      DgZ3StringRF (DgRFNetwork& networkIn, const string& nameIn,
                         int resIn)
         : DgRF<DgZ3StringCoord, long long int>(networkIn, nameIn),
           res_ (resIn) { }

      int res_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
