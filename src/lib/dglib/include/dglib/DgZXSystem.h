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

#include <dglib/DgHierNdxIDGGS.h>

class DgZXRF;
class DgZXStringRF;

////////////////////////////////////////////////////////////////////////////////
class DgZXSystem : public DgHierNdxIDGGS<DgZXRF, DgZXStringRF> {

   public:

   protected:

     DgZXSystem (const DgIDGGSBase& dggsIn, bool outModeIntIn = true, 
            const string& nameIn = "ZXSystem")
        : DgHierNdxIDGGS(dggsIn, outModeIntIn, nameIn)
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
