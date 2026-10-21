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
// DgSeqNumRF.h: DgSeqNumRF class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGSEQNUMRF_H
#define DGSEQNUMRF_H

#include <climits>
#include <iostream>
#include <string>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgBoundedIDGG.h>

////////////////////////////////////////////////////////////////////////////////
// A cell's sequence number within its DGG. The undefined value is the largest
// unsigned value, since 0 is a valid sequence number in a zero-based DGG.
class DgSeqNumCoord {

   public:

      static const unsigned long long int undefValue;

      DgSeqNumCoord (void) : value_ (undefValue) { }

      DgSeqNumCoord (unsigned long long int valIn) : value_ (valIn) { }

      void setValue (unsigned long long int value) { value_ = value; }

      unsigned long long int value (void) const { return value_; }

      operator std::string (void) const { return valString(); }

      std::string valString (void) const { return std::to_string(value_); }

      bool operator== (const DgSeqNumCoord& c) const
          { return value() == c.value(); }

      bool operator!= (const DgSeqNumCoord& c) const
          { return !(*this == c); }

   private:

      unsigned long long int value_;
};

////////////////////////////////////////////////////////////////////////////////
inline std::ostream&
operator<< (std::ostream& stream, const DgSeqNumCoord& coord)
{ return stream << coord.valString(); }

////////////////////////////////////////////////////////////////////////////////
// The sequence numbers of the cells of one resolution of a DGG. It is backed
// by the DGG (Q2DI addresses), so any frame connected to the DGG converts to
// and from it.
class DgSeqNumRF : public DgDiscRF<DgSeqNumCoord, DgQ2DICoord, long long int>
{
   public:

      static const DgSeqNumCoord undefCoord;

      static const DgSeqNumRF* makeRF (const DgIDGGBase& dggIn,
                                       const std::string& nameIn)
                { return new DgSeqNumRF(dggIn, nameIn); }

      const DgIDGGBase& dgg (void) const { return dgg_; }

      virtual const DgSeqNumCoord& undefAddress (void) const
                       { return undefCoord; }

      virtual DgSeqNumCoord quantify (const DgQ2DICoord& point) const
                { return DgSeqNumCoord(dgg().bndRF().seqNumAddress(point)); }

      virtual DgQ2DICoord invQuantify (const DgSeqNumCoord& add) const
                { return dgg().bndRF().addFromSeqNum(add.value()); }

      // sequence numbers don't use delimiters
      virtual std::string add2str (const DgSeqNumCoord& add) const
                       { return add.valString(); }
      virtual std::string add2str (const DgSeqNumCoord& add, char) const
                       { return add.valString(); }

      virtual const char* str2add (DgSeqNumCoord* c, const char* str,
                                   char delimiter) const;

      virtual long long int dist (const DgSeqNumCoord&, const DgSeqNumCoord&) const
                       { return 0; }
      virtual void setAddNeighbors (const DgSeqNumCoord&, DgLocVector&) const { }
      virtual void setAddVertices  (const DgSeqNumCoord&, DgPolygon&) const { }

   protected:

      DgSeqNumRF (const DgIDGGBase& dggIn, const std::string& nameIn)
         : DgDiscRF<DgSeqNumCoord, DgQ2DICoord, long long int>(dggIn.network(),
                       dggIn, nameIn),
           dgg_ (dggIn) { }

      const DgIDGGBase& dgg_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
