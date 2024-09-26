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
// DgHierNdxIDGGSBase.h: DgHierNdxIDGGSBase class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXIDGGSBASE_H
#define DGHIERNDXIDGGSBASE_H

#include <vector>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgHierNdxRFS.h>

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIDGGSBase 
             : public DgHierNdxRFS<ResAdd<DgQ2DICoord>, long long int> {

   public:

      const DgIDGGBase& dggBase (int res) const
             { return static_cast<const DgIDGGBase&>(operator[](res)); }

      const DgIDGGSBase& dggs (void) { return dggs_; }

      virtual operator string (void) const
      {
         string s = "*** DgHierNdxIDGGSBase";
         return s;
      }

   protected:

     DgHierNdxIDGGSBase (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
            const string& nameIn = "HierNdxIDGGSBase")
        : DgHierNdxRFS(dggsIn.network(), dggsIn, dggsIn.nRes(), outModeIntIn, nameIn), 
          dggs_ (dggsIn)
     { }

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const = 0;

   private:

     // state data
     const DgIDGGSBase& dggs_;

};

#endif
