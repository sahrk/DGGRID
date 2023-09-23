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
// SubOpMain.h: this file defines the main parameters for any DGGRID operation
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPMAIN_H
#define SUBOPMAIN_H

#include "SubOpBasic.h"

struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
struct SubOpMain : public SubOpBasic {

   public:

      SubOpMain (OpBasic& op, bool activate = true);

      // the parameters
      string operation;  // operation for dggrid to perform
      int precision;     // number of output digits to right of decimal point
      int verbosity;     // debugging info verbosity
      bool megaVerbose;
      bool pauseOnStart;
      bool pauseBeforeExit;
      bool useMother;         // use Mother RNG?
      unsigned long int updateFreq; // how often to output updates

      // DgApSubOperation virtual methods that use the pList
      virtual int initializeOp (void);
      virtual int setupOp (void);
};

////////////////////////////////////////////////////////////////////////////////

#endif
