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
// DgZXStringRF.h: DgZXStringRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZXSTRINGRF_H
#define DGZXSTRINGRF_H

#include <climits>
#include <iostream>

#include <dglib/DgHierNdxStringRF.h>

////////////////////////////////////////////////////////////////////////////////
class DgZXStringRF : public DgHierNdxStringRF {

   public:

      static DgZXStringRF* makeRF (const DgIDGGS& dggsIn, int resIn, 
                             const std::string& nameIn) 
         { return new DgZXStringRF (dggsIn, resIn, nameIn); }

      // these need to be defined by specializations
      virtual const char* str2add (DgHierNdxStringCoord* add, const char* str,
                                   char delimiter) const;

      // these have dummy definitions from the superclass
      virtual DgHierNdxStringCoord quantify (const DgQ2DICoord& point) const;
      virtual DgQ2DICoord invQuantify (const DgHierNdxStringCoord& add) const;

   protected:

      DgZXStringRF (const DgIDGGS& dggsIn, int resIn, const std::string& nameIn)
         : DgHierNdxStringRF(dggsIn, resIn, nameIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif
