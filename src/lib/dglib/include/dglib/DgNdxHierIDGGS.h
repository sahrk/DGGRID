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
// DgNdxHierIDGGS.h: DgNdxHierIDGGS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGNDXHIERIDGGS_H
#define DGNDXHIERIDGGS_H

#include <vector>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgNdxHierRFS.h>

////////////////////////////////////////////////////////////////////////////////
class DgNdxHierIDGGS : 
         public DgNdxHierRFS<DgQ2DICoord, DgGeoCoord, long double> {

   public:
    
      const DgIDGGSBase& idggs (void) { return idggs_; }

      virtual operator string (void) const
      {
         string s = "*** DgNdxHierIDGGS";
         return s;
      }

   protected:

     DgNdxHierIDGGS (const DgIDGGSBase& idggsIn)
        : DgNdxHierRFS(idggsIn), idggs_ (idggsIn)
     { }

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                     DgLocVector& children) const = 0;

   private:
     // state data
     const DgIDGGSBase& idggs_;
};

#endif
