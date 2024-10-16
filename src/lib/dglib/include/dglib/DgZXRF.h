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

#include <dglib/DgHierNdxIntRF.h>

class DgZXSystem;

////////////////////////////////////////////////////////////////////////////////
class DgZXRF : public DgHierNdxIntRF {

   public:

      // these need to be defined by specializations
      virtual const char* str2add (DgHierNdxIntCoord* c, const char* str, 
                   char delimiter) const;

      // these have dummy definitions from the superclass
      virtual DgHierNdxIntCoord quantify (const DgQ2DICoord& point) const;
      virtual DgQ2DICoord invQuantify (const DgHierNdxIntCoord& add) const;

   protected:

    DgZXRF (const DgHierNdxSystemRFBase& sysIn, int resIn, const std::string& nameIn);

    friend DgZXSystem;
    template<class TINT, class TSTR> friend class DgHierNdxSystemRF;
};

////////////////////////////////////////////////////////////////////////////////
#endif
