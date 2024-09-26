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
// DgZXRFS.h: DgZXRFS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGZXRFS_H
#define DGZXRFS_H

#include <dglib/DgNdxHierIDGGS.h>

class DgZXRF;
class DgZXStringRF;

////////////////////////////////////////////////////////////////////////////////
class DgZXRFS : public DgNdxHierIDGGS<DgZXRF, DgZXStringRF> {

   public:

   protected:

     DgZXRFS (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
            const string& nameIn = "ZXRFS")
        : DgNdxHierIDGGS(dggsIn, outModeIntIn, nameIn)
     { 
     }

     // override virtual functions with dummy definitions from above
     // quant/invQuant?

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgQ2DICoord>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgQ2DICoord>& add,
                                     DgLocVector& children) const = 0;
};

#endif
