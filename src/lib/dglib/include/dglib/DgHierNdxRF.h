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
// DgHierNdxRF.h: DgHierNdxRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXRF_H
#define DGHIERNDXRF_H

#include <climits>
#include <iostream>

#include <dglib/DgHierNdxRFBase.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
template <class T> class DgHierNdxCoord : public DgHierNdxCoordBase {

   public:

      // this should be initialized in the instantiated class
      static const DgHierNdxCoord<T> undefCoordTyped;

/*
      DgHierNdxCoord<T> (void)
         { *this = undefCoordTyped; }

      DgHierNdxCoord<T> (const DgHierNdxCoord<T>& coord)
         { *this = coord; }
*/
      DgHierNdxCoord (T valIn) : value_ (valIn) { }

      DgHierNdxCoord<T> (const DgHierNdxCoord<T>& coord = undefCoordTyped)
         { *this = coord; }

      void setValue (T value) { value_ = value; }

      T value (void) const { return value_; }

      virtual bool operator== (const DgHierNdxCoordBase& c) const
          { 
             const DgHierNdxCoord<T>* cTyped = dynamic_cast<const DgHierNdxCoord<T>*>(&c);
             // false if cTyped == NULL
             return cTyped && value() == cTyped->value(); 
          }

      virtual DgHierNdxCoordBase& operator= (const DgHierNdxCoordBase& c)
          {
             const DgHierNdxCoord<T>* cTyped = dynamic_cast<const DgHierNdxCoord<T>*>(&c);
             if (cTyped && *cTyped != *this) setValue(cTyped->value());

             return *this;
          }

      virtual const DgHierNdxCoordBase& undefCoord (void) const
          { return undefCoordTyped; }

      // sub-classes need to define this method
      // it has a dummy definition from the Base class
      //virtual string valString (void) const;

   private:

      T value_;
};

////////////////////////////////////////////////////////////////////////////////
template <class T> class DgHierNdxRF : public DgHierNdxRFBase {

   public:

      static DgHierNdxRF<T>* makeRF (const DgIDGGBase& dggIn, const string& nameIn,
                                  int resIn, int apertureIn)
         { return new DgHierNdxRF<T>(dggIn, nameIn, resIn, apertureIn); }

      virtual const char* str2add (DgHierNdxCoordBase* c, const char* str,
                                   char delimiter) const {
             DgHierNdxCoord<T>* cTyped = dynamic_cast<DgHierNdxCoord<T>*>(&c);
             if (!cTyped) {
                *c = c->undefCoord();
                return str;
             } else {
                return str2addTyped(cTyped, str, delimiter); 
             }
         }

      const DgHierNdxCoord<T>& undefCoordTyped (void) const
                            { return DgHierNdxCoord<T>::undefCoordTyped; }

      virtual const DgHierNdxCoordBase& undefCoord (void) const
                            { return undefCoordTyped(); }

      // method to be defined by sub-classes
      // given a dummy implementation here so the class isn't abstract
      virtual const char* str2addTyped (DgHierNdxCoord<T>* add, const char* str,
                                   char delimiter) const { return str; }

   protected:

      DgHierNdxRF<T> (const DgIDGGBase& dggIn, const string& nameIn,
                    int resIn, int apertureIn)
         : DgHierNdxRFBase(dggIn, nameIn, resIn, apertureIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
