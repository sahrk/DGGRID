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
// DgHierNdxCoord.h: DgHierNdxCoord, DgHierNdxIntCoord and DgHierNdxStringCoord
//    class definitions. These are kept apart from the RF headers so that
//    DgHierNdx can be complete before DgHierNdxSystemRFBase uses it.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXCOORD_H
#define DGHIERNDXCOORD_H

#include <cstdint>
#include <iostream>
#include <string>

#define HIERNDX_INT_TYPE uint64_t

////////////////////////////////////////////////////////////////////////////////
template <class T> class DgHierNdxCoord  {

   public:

      //static const DgHierNdxCoord<T> undefDgHierNdxCoord<T>;

      DgHierNdxCoord (void) : value_ (0) { }

      DgHierNdxCoord (T valIn) : value_ (valIn) { }

      DgHierNdxCoord (const DgHierNdxCoord<T>& coord)
              { value_ = coord.value(); }

      void setValue (T value) { value_ = value; }

      T value (void) const { return value_; }

      operator std::string (void) const { return valString(); }

      bool operator== (const DgHierNdxCoord<T>& c) const
          { return value() == c.value(); }

      bool operator!= (const DgHierNdxCoord<T>& c) const
          { return !(*this == c); }

      DgHierNdxCoord<T>& operator= (const DgHierNdxCoord<T>& add)
          {
             if (add != *this) setValue(add.value());

             return *this;
          }

      // sub-classes must define
      virtual std::string valString (void) const = 0;

   private:

      T value_;
};

////////////////////////////////////////////////////////////////////////////////
template <class T> inline std::ostream&
operator<< (std::ostream& stream, const DgHierNdxCoord<T>& coord)
{ return stream << std::string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIntCoord : public DgHierNdxCoord<HIERNDX_INT_TYPE> {

   public:

      DgHierNdxIntCoord (void);

      DgHierNdxIntCoord (HIERNDX_INT_TYPE val)
         : DgHierNdxCoord<HIERNDX_INT_TYPE>(val) { }

      // define abstract method from above
      // output as hexadecimal string
      virtual std::string valString (void) const;
};

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxStringCoord : public DgHierNdxCoord<std::string> {

   public:

      DgHierNdxStringCoord (void);

      DgHierNdxStringCoord (std::string val)
          : DgHierNdxCoord<std::string>(val) { }

      // abstract method from above
      virtual std::string valString (void) const {
         return value();
      }
};

////////////////////////////////////////////////////////////////////////////////
#endif
