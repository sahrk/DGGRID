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
// DgZOrderStringRF.h: DgZOrderStringRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZORDERSTRINGRF_H
#define DGZORDERSTRINGRF_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgQ2DICoord;
class DgIDGGBase;
class DgZOrderCoord;
class DgZOrderStringCoord;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgQ2DItoZOrderStringConverter :
        public DgConverter<DgQ2DICoord, long long int, DgZOrderStringCoord, long long int>
{
   public:

      DgQ2DItoZOrderStringConverter (const DgRF<DgQ2DICoord, long long int>& from,
                                   const DgRF<DgZOrderStringCoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgZOrderStringCoord convertTypedAddress
                                (const DgQ2DICoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class DgZOrderStringToQ2DIConverter :
        public DgConverter<DgZOrderStringCoord, long long int, DgQ2DICoord, long long int>
{
   public:

      DgZOrderStringToQ2DIConverter (const DgRF<DgZOrderStringCoord, long long int>& from,
                                   const DgRF<DgQ2DICoord, long long int>& to);

      const DgIDGGBase& IDGG (void) const { return *pIDGG_; }

      virtual DgQ2DICoord convertTypedAddress
                                (const DgZOrderStringCoord& addIn) const;

   protected:

      const DgIDGGBase* pIDGG_;
      int effRes_;
      int effRadix_;

};

////////////////////////////////////////////////////////////////////////////////
class Dg2WayZOrderStringConverter : public Dg2WayConverter {

   public:

      Dg2WayZOrderStringConverter (const DgRF<DgQ2DICoord, long long int>& fromFrame,
              const DgRF<DgZOrderStringCoord, long long int>& toFrame)
         : Dg2WayConverter (*(new DgQ2DItoZOrderStringConverter(fromFrame, toFrame)),
              *(new DgZOrderStringToQ2DIConverter(toFrame, fromFrame))) {}
};

////////////////////////////////////////////////////////////////////////////////
//
//   Coordinate consisting of a string containing quad number, (aperture 3 hex
//        level indicator), and digit-interleaved radix string
//
class DgZOrderStringCoord  {

   public:

      static const DgZOrderStringCoord undefDgZOrderStringCoord;

      DgZOrderStringCoord (void) { }

      DgZOrderStringCoord (const string& valStrIn)
         : valString_ (valStrIn) { }

      DgZOrderStringCoord (const DgZOrderStringCoord& coord)
              { valString_ = coord.valString(); }

      void setValString (const string strIn) { valString_ = strIn; }

      const string& valString (void) const { return valString_; }

      operator string (void) const { return valString(); }

      bool operator== (const DgZOrderStringCoord& c) const
          { return valString() == c.valString(); }

      bool operator!= (const DgZOrderStringCoord& c) const
          { return !(*this == c); }

      DgZOrderStringCoord& operator= (const DgZOrderStringCoord& add)
          {
             if (add != *this) setValString(add.valString());

             return *this;
          }

   private:

      string valString_;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgZOrderStringCoord& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgZOrderStringRF : public DgRF<DgZOrderStringCoord, long long int> {

   public:

      static DgZOrderStringRF* makeRF (DgRFNetwork& networkIn,
               const string& nameIn, int resIn, int apertureIn)
         { return new DgZOrderStringRF (networkIn, nameIn, resIn, apertureIn); }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

      virtual long long int dist (const DgZOrderStringCoord& add1,
                        const DgZOrderStringCoord& add2) const
                       { return 0; }

      virtual string add2str (const DgZOrderStringCoord& add) const
                       { return string(add); }

      virtual string add2str (const DgZOrderStringCoord& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual const char* str2add (DgZOrderStringCoord* add, const char* str,
                                   char delimiter) const;

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      virtual const DgZOrderStringCoord& undefAddress (void) const
                       { return DgZOrderStringCoord::undefDgZOrderStringCoord; }

   protected:

      DgZOrderStringRF (DgRFNetwork& networkIn, const string& nameIn,
                         int resIn, int apertureIn)
         : DgRF<DgZOrderStringCoord, long long int>(networkIn, nameIn),
           res_ (resIn), aperture_ (apertureIn) { }

      int res_;
      int aperture_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
