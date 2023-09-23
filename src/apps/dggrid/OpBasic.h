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
// OpBasic.h: this file defines a base class operation that has access to all
//            of the major suboperations used by current DGGRID operations
//
////////////////////////////////////////////////////////////////////////////////

#ifndef OPBASIC_H
#define OPBASIC_H

#include <dglib/DgUtil.h>

#include <dgaplib/DgApOperationPList.h>

#include "SubOpMain.h"
#include "SubOpDGG.h"
#include "SubOpIn.h"
#include "SubOpOut.h"

struct SubOpBasic;
struct SubOpBasicMulti;

////////////////////////////////////////////////////////////////////////////////
struct OpBasic : public DgApOperationPList {

   OpBasic (const string& inFileName);

   virtual int initialize (bool force = false);
   virtual int cleanup    (bool force = false);

   // sub operation objects; these get added to the list of sub ops
   // (maintained by DgApOperation) by the DgApSubOperation constructor
   // sub-classes should deactivate unused sub operations by setting the
   // activate flag to false in the constructor
   SubOpMain mainOp;
   SubOpDGG dggOp;
   SubOpIn inOp;
   SubOpOut outOp;
   SubOpBasicMulti* primarySubOp;
};

////////////////////////////////////////////////////////////////////////////////

#endif
