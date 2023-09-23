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
// OpBasic.cpp: OpBasic class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgConstants.h>
#include <dglib/DgConverterBase.h>

#include "OpBasic.h"
#include "SubOpBasicMulti.h"
#include "SubOpBinPts.h"
#include "SubOpGen.h"
#include "SubOpStats.h"
#include "SubOpTransform.h"

using namespace dgg::addtype;

////////////////////////////////////////////////////////////////////////////////
// by default only main and DGG operations are active
OpBasic::OpBasic (const string& inFileName)
      : DgApOperationPList (inFileName),
        mainOp (*this, true), dggOp (*this, true), inOp (*this, true),
        outOp (*this, true), primarySubOp (nullptr)
{
}

////////////////////////////////////////////////////////////////////////////////
int
OpBasic::initialize (bool force)
{
   // first get the operation name
   mainOp.initializeOp();
   pList.loadParams(inFileName, false);
   mainOp.setupOp();
   string opName = mainOp.operation;
   mainOp.reset();
   pList.clearList();

   //cout << "operation: " << opName << endl;

   if (opName == "GENERATE_GRID")
      primarySubOp = new SubOpGen(*this);
   else if (opName == "GENERATE_GRID_FROM_POINTS")
      primarySubOp = new SubOpBinPts(*this);
   else if (opName == "OUTPUT_STATS")
      primarySubOp = new SubOpStats(*this);
   else if (opName == "BIN_POINT_VALS")
      primarySubOp = new SubOpBinPts(*this);
   else if (opName == "BIN_POINT_PRESENCE")
      primarySubOp = new SubOpBinPts(*this);
   else if (opName == "TRANSFORM_POINTS")
      primarySubOp = new SubOpTransform(*this);
   else
      report(string("invalid operation ") + mainOp.operation, DgBase::Fatal);

   // now reset the plist using the correct primary sub operation
   return DgApOperationPList::initialize();

} // int OpBasic::initialize

////////////////////////////////////////////////////////////////////////////////
int
OpBasic::cleanup (bool force)
{
   int result = DgApOperationPList::cleanup(force);
   // cleanup should have called cleanupOp on the primarySubOp
   delete primarySubOp;

   return result;
}

////////////////////////////////////////////////////////////////////////////////
