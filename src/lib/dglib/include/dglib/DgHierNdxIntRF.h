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
// DgHierNdxIntRF.h: DgHierNdxIntRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXINTRF_H
#define DGHIERNDXINTRF_H

#include <climits>
#include <iostream>

#include <dglib/DgHierNdxRF.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
class DgHierNdxIntCoord : public DgHierNdxCoord<uint64_t> {

   public:

      DgHierNdxIntCoord (void)
         { *this = undefCoordTyped; }

      // method for sub-classes to define
      virtual string valString (void) const = 0;
};

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIntRF : public DgHierNdxRF<uint64_t> {

   public:

      static DgHierNdxIntRF* makeRF (const DgIDGGBase& dggIn, const string& nameIn,
                                  int resIn, int apertureIn)
         { return new DgHierNdxRF<uint64_t>(dggIn, nameIn, resIn, apertureIn); }

      // method to be defined by sub-classes
      virtual const char* str2addTyped (DgHierNdxCoord<T>* add, const char* str,
                                   char delimiter) const = 0;

   protected:

      DgHierNdxIntRF (const DgIDGGBase& dggIn, const string& nameIn,
                    int resIn, int apertureIn)
         : DgHierNdxRF<uint64_t>(dggIn, nameIn, resIn, apertureIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
