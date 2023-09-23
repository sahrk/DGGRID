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
// DgApSubOperation.cpp: DgApSubOperation class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <string>

#include <dglib/DgBase.h>
#include <dgaplib/DgApOperation.h>

////////////////////////////////////////////////////////////////////////////////
DgApSubOperation::DgApSubOperation (DgApOperation& _operation, bool _active)
   : active (_active)
{
   setOperation(_operation);
}

////////////////////////////////////////////////////////////////////////////////
DgApSubOperation::~DgApSubOperation (void)
{ }

////////////////////////////////////////////////////////////////////////////////
void
DgApSubOperation::setOperation(DgApOperation& _operation) {
   operation = &_operation;
   operation->addSubOp(*this);
}

////////////////////////////////////////////////////////////////////////////////
// initialize the parameter data
// could be called in constructor
int
DgApSubOperation::initialize (bool force) {

   if (!active) return 0;

   // don't do if already done
   if (state >= INITIALIZED && !force) return 1;

   // initialize my parameters
   int result = initializeOp();

   if (!result) {
      // update my state
      state = INITIALIZED;
      numExecutions = 0;
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// setup the parameter data
// called after initialize() and before execute()
int
DgApSubOperation::setup (bool force) {

   if (!active) return 0;

   // don't do if already done
   if (state >= SETUP && !force) return 1;

   // setup my parameters
   int result = setupOp();

   if (!result)
      state = SETUP;

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// called after setup()
// this calls executeOp as per initialize() and setup()
int
DgApSubOperation::execute (bool force) {

   if (!active) return 0;

   // don't do if already done
   if (state >= EXECUTED && !force) return 1;

   int result = executeOp();

   // update my state
   if (!result) {
     state = EXECUTED;
     numExecutions++;
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// called after execute()
int
DgApSubOperation::cleanup (bool force) {

   if (!active) return 0;

   // don't do if already done
   if (state >= CLEANED && !force) return 1;

   int result = cleanupOp();

   if (!result)
      state = CLEANED;

   return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
