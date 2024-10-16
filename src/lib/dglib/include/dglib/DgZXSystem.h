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
// DgZXSystem.h: DgZXSystem class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZXSYSTEM_H
#define DGZXSYSTEM_H

#include <dglib/DgHierNdxSystemRFS.h>
#include <dglib/DgZXRF.h>
#include <dglib/DgZXStringRF.h>

////////////////////////////////////////////////////////////////////////////////
class DgZXSystem : public DgHierNdxSystemRFS<DgZXRF, DgZXStringRF> {

   public:

      static DgZXSystem* makeSystem (const DgIDGGS& dggsIn, bool outModeIntIn = true,
                             const std::string& nameIn = "ZXSystem")
      {
          return new DgZXSystem (dggsIn, outModeIntIn, nameIn);
      }
 
   protected:

     DgZXSystem (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
                 const string& nameIn = "ZXSystem");
 
     // default methods quantize via string representation; redefine to
     // get different behavior
     //virtual DgHierNdx quantify (const DgQ2DICoord& point) const;
     //virtual DgQ2DICoord invQuantify (const DgHierNdx& add) const;

     // abstract methods from above

     virtual DgHierNdxIntCoord toIntCoord (const DgHierNdxStringCoord& c) const;
     virtual DgHierNdxStringCoord toStringCoord (const DgHierNdxIntCoord& c) const;
     virtual void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const;
     virtual void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const;
};

#endif
