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
// SubOpTransform.cpp: SubOpTransform class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgCell.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgInputStream.h>
#include <dglib/DgOutputStream.h>
#include <dglib/DgDataField.h>
#include <dglib/DgInLocTextFile.h>

#include "OpBasic.h"
#include "SubOpTransform.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
SubOpTransform::SubOpTransform (OpBasic& _op, bool _activate)
   : SubOpBasicMulti (_op, _activate)
{
   // turn-on/off the available sub operations
   op.mainOp.active = true;
   op.dggOp.active = true;
   op.inOp.active = true;
   op.outOp.active = true;

   op.inOp.isPointInput = true;

} // SubOpTransform::SubOpTransform

////////////////////////////////////////////////////////////////////////////////
int
SubOpTransform::initializeOp (void) {

   vector<string*> choices;
   return 0;

} // int SubOpTransform::initializeOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpTransform::setupOp (void) {

   /////// fill state variables from the parameter list //////////
   string dummy;

   return 0;

} // int SubOpTransform::setupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpTransform::cleanupOp (void) {

   return 0;

} // int SubOpTransform::cleanupOp

////////////////////////////////////////////////////////////////////////////////
int
SubOpTransform::executeOp (void) {

   // Transform to GEO doesn't make sense (that would just be the cell point)
   if (op.outOp.outAddType == dgg::addtype::Geo) {
      ::report("SubOpTransform::executeOp() output address type must be non-GEO",
               DgBase::Fatal);
   }

   const DgIDGGBase& dgg = op.dggOp.dgg();
   dgcout << "Res " << dgg.outputRes() << " " << dgg.gridStats() << endl;
   dgcout << "\ntransforming values..." << endl;

   while (true) {

      DgLocationData* loc = op.inOp.getNextLoc();
      if (!loc) break; // reached EOF on last input file
#if DGDEBUG
   dgcout << "TRANSFORM BEFORE: " << *loc << endl;
#endif

      //op.outOp.pOutRF->convert(loc);
      dgg.convert(loc);

      op.outOp.outputCellAdd2D(*loc, nullptr, loc->dataList());

#if DGDEBUG
   dgcout << "TRANSFORM AFTER: " << *loc << endl;
#endif

      delete loc;
   }

   int numFiles = op.inOp.fileNum;
   dgcout << "\nprocessed " << numFiles << " input file"
          << ((numFiles > 1) ? "s." : ".") << endl;
   dgcout << "** transformation complete **" << endl;

   return 0;

} // int SubOpTransform::executeOp

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

