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
// DgApSubOperation.h: DgApSubOperation class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGAPSUBOPERATION_H
#define DGAPSUBOPERATION_H

#include <string>
#include <dglib/DgBase.h>

struct DgApOperation;

////////////////////////////////////////////////////////////////////////////////
struct DgApSubOperation {

   enum DgApSubOpState { NEW, INITIALIZED, SETUP, EXECUTED, CLEANED };

   DgApSubOperation (DgApOperation& _operation, bool _active = true);

   virtual ~DgApSubOperation (void);

   void setOperation (DgApOperation& _operation);

   // these methods call the corresponding virtual helper
   // methods below

   // initialize the parameter data
   // this calls initializeOp
   int initialize (bool force = false);

   // setup the parameter data
   // called after initialize() and before execute()
   // this calls setUpOp
   int setup (bool force = false);

   // called after setup()
   // this calls executeOp
   virtual int execute (bool force = false);

   // called after execute()
   // this calls cleanupOp
   int cleanup (bool force = false);

   // rest to original state and numExecutions
   virtual int reset (void) {
      state = NEW;
      numExecutions = 0;
      return 0;
   }

   // instance variables
   DgApOperation* operation = NULL;
   DgApSubOpState state = NEW;
   int numExecutions = 0;
   bool active = true;

   // virtual methods with default implementations
   // to be defined by sub-classes
   protected:
      virtual int initializeOp (void) { return 0; }
      virtual int setupOp      (void) { return 0; }
      virtual int executeOp    (void) { return 0; }
      virtual int cleanupOp    (void) { return 0; }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
