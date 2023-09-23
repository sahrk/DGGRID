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
// SubOpBinPts.h: this file defines operations with point input
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SUBOPBINPTS_H
#define SUBOPBINPTS_H

#include <dglib/DgUtil.h>

#include "SubOpBasicMulti.h"

struct OpBasic;
struct Val;

////////////////////////////////////////////////////////////////////////////////
struct SubOpBinPts : public SubOpBasicMulti {

   SubOpBinPts (OpBasic& _op, bool _activate = true);

   // virtual methods
   virtual int initializeOp (void);
   virtual int setupOp (void);
   virtual int cleanupOp (void);
   virtual int executeOp (void);

   // the parameters
   bool wholeEarth;
   bool outputAllCells;   // or only occupied ones?
   string inValFieldName; // name of input field to take the average of
   string valFmtStr;      // how format values for string output
   bool useValInput;      // input has a value field

   bool outputCount;      // output count of points-in-cell?
   string outputCountFldName;

   bool outputTotal;      // output total of all values
   string outputTotalFldName;

   bool outputMean;       // output a mean field
   string outputMeanFldName;

   bool outputPresVec;    // output presence/absence bit vector
   string outputPresVecFldName;

   bool outputNumClasses;    // output number of true's in the presence/absence vector
   string outputNumClassesFldName;

   bool outputMeanDefault;
   bool outputPresVecDefault;

   protected:

      // helper methods
      virtual DgLocationData* inStrToPointLoc (const string& inStr) const;
      virtual double getVal (DgLocationData& loc) const;
      void initVal (Val& val, bool allocPresVec) const;
      int presVecToString (const bool* presVec, int allClasses,
                  string& vecStr) const;

      void binPtsGlobal (void);
      void binPtsPartial (void);
      void outputCell(unsigned long int sNum, const Val& val) const;
      void outputCell(const DgLocation& loc, const Val& val) const;
};

////////////////////////////////////////////////////////////////////////////////

#endif
