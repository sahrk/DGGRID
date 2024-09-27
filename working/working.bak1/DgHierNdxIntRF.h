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

#define INT_NDX_TYPE uint64_t

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIntRF : public DgHierNdxRF<INT_NDX_TYPE> {

   public:

      static DgHierNdxIntRF* makeRF (const DgIDGGS& dggsIn, int resIn,
                                     const string& nameIn = "IntNdx")
         { return new DgHierNdxIntRF(dggsIn, resIn, nameIn); }

      // these assume input/output strings are in hexadecimal
      virtual string add2str (const INT_NDX_TYPE& add) const;
      virtual const char* str2add (INT_NDX_TYPE* c, const char* str, char delimiter) const;

      // these need to be defined by specializations
      // they have dummy definitions from the superclass
      //virtual INT_NDX_TYPE quantify (const DgQ2DICoord& point) const
      //virtual DgQ2DICoord invQuantify (const INT_NDX_TYPE& add) const

   protected:

      DgHierNdxIntRF (const DgIDGGS& dggsIn, int resIn, const string& nameIn)
         : DgHierNdxRF<INT_NDX_TYPE>(dggsIn, resIn, nameIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
