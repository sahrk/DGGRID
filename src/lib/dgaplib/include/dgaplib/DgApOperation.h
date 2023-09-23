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
// DgApOperation.h: DgApOperation class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGAPOPERATION_H
#define DGAPOPERATION_H

#include <string>
#include <vector>

#include <dgaplib/DgApSubOperation.h>

////////////////////////////////////////////////////////////////////////////////
struct DgApOperation {

   DgApOperation (void) { }

   void addSubOp (DgApSubOperation& op)
      { subops.push_back(&op); }

   // note no separate setup() method for DgApOperations
   // because they have no data members
   virtual int initialize (bool force = false) {
      int result1 = initializeAll(force);
      int result2 = setupAll(force);
      return result1 || result2;
   }

   // default execute executes the subops
   virtual int execute (bool force = false) {
      return executeAll(force);
   }

   // default cleanup cleans the subops
   virtual int cleanup (bool force = false) {
      return cleanupAll(force);
   }

   // the methods below call corresponding
   // methods on the sub operations

   // usually called before setupAll()
   int initializeAll (bool force = false) {
      int result = 0;
      for (auto op: subops) {
         int thisResult = op->initialize(force);
         if (thisResult) result = thisResult;
      }
      return result;
   }

   // usually called before execute()
   int setupAll (bool force = false) {
      int result = 0;
      for (auto op: subops) {
         int thisResult = op->setup(force);
         if (thisResult) result = thisResult;
      }
      return result;
   }

   // execute subops function
   int executeAll (bool force = false) {
      int result = 0;
      for (auto op: subops) {
         int thisResult = op->execute(force);
         if (thisResult) result = thisResult;
      }
      return result;
   }

   // usually called after execute()
   int cleanupAll (bool force = false) {
      int result = 0;
      for (auto op: subops) {
         int thisResult = op->cleanup(force);
         if (thisResult) result = thisResult;
      }
      return result;
   }

   // the sub-operations
   vector<DgApSubOperation*> subops;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
