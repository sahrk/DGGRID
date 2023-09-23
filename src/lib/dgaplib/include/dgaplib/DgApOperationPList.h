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
// DgApOperationPList.h: DgApOperationPList class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGAPOPERATIONPLIST_H
#define DGAPOPERATIONPLIST_H

#include <dgaplib/DgApOperation.h>
#include <dgaplib/DgApParamList.h>

////////////////////////////////////////////////////////////////////////////////
struct DgApOperationPList : public DgApOperation {

   DgApOperationPList (const string& _inFileName)
      : DgApOperation (), inFileName (_inFileName) { }

   virtual int initialize (bool force = false) {

      // setup the plist
      int result = initializeAll(force);

      // read the parameter values into the plist
      pList.loadParams(inFileName);

      // load the parameter values into the sub operations
      int result2 = setupAll(force);

      return result || result2;
   }

   virtual int cleanup (bool force = false) {
      int result = DgApOperation::cleanup(force);
      pList.clearList();

      return result;
   }

   string inFileName;
   DgApParamList pList;
};

////////////////////////////////////////////////////////////////////////////////

/*
inline ostream& operator<< (ostream& output, const DgApOperationPList& f)
   { return output << f.mp.operation; }
*/

////////////////////////////////////////////////////////////////////////////////

#endif
