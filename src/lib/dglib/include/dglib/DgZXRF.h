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
// DgZXRF.h: DgZXRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZXRF_H
#define DGZXRF_H

//#include <climits>
//#include <iostream>

#include <dglib/DgHierNdxIntRF.h>

//using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgZXRF : public DgHierNdxIntRF {

   public:

      static DgZXRF* makeRF (const DgIDGGS& dggsIn, int resIn, 
                             const std::string& nameIn) 
         { return new DgZXRF (dggsIn, resIn, nameIn); }

      // these need to be defined by specializations
      // they have dummy definitions from the superclass
      virtual DgHierNdxIntCoord quantify (const DgQ2DICoord& point) const;
      virtual DgQ2DICoord invQuantify (const DgHierNdxIntCoord& add) const;

   protected:

      DgZXRF (const DgIDGGS& dggsIn, int resIn, const std::string& nameIn)
         : DgHierNdxIntRF(dggsIn, resIn, nameIn) { }

};

////////////////////////////////////////////////////////////////////////////////
#endif