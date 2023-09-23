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
// DgApSubOpPList.h: DgApSubOpPList class definitions
//
// A DgApSubOperation that uses a DgApParamList to get it's parameter values
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGAPSUBOPPLIST_H
#define DGAPSUBOPPLIST_H

#include <dgaplib/DgApOperationPList.h>
#include <dgaplib/DgApSubOperation.h>
#include <dgaplib/DgApParamList.h>

////////////////////////////////////////////////////////////////////////////////
struct DgApSubOpPList : public DgApSubOperation {

   DgApSubOpPList (DgApOperationPList& _opPList, bool _active = true)
      : DgApSubOperation (_opPList, _active), opPList (_opPList) {  }

   DgApOperationPList& opPList;
   DgApParamList& pList (void) { return opPList.pList; }
};

////////////////////////////////////////////////////////////////////////////////

/*
inline ostream& operator<< (ostream& output, const DgApSubOpPList& f)
   { return output << f.operation(); }
*/

////////////////////////////////////////////////////////////////////////////////

#endif
