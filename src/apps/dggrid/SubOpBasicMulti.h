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
// SubOpBasicMulti.h: basic executable sub operation with potentially multiple
//       DGG placements
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPBASICMULTI_H
#define SUBOPBASICMULTI_H

#include <dglib/DgCell.h>
#include <dglib/DgUtil.h>

#include "SubOpBasic.h"

class DgDataList;
struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
struct SubOpBasicMulti : public SubOpBasic {

   SubOpBasicMulti (OpBasic& _op, bool _activate = true);

   // redefine from DgApSubOperation to loop over multiple grids
   virtual int execute (bool force = false);

   // methods to support lightweight text input/output
   // default loops over the data fields in the list
   virtual string dataToOutStr (DgDataList* data);
   //virtual DgCell* inStrToPointCell (const string& inStr) const;
   virtual DgLocationData* inStrToPointLoc (const string& inStr) const;

};

////////////////////////////////////////////////////////////////////////////////

#endif
