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
// DgHierNdxIDGG.h: DgHierNdxIDGG header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXIDGG_H
#define DGHIERNDXIDGG_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>

class DgHierNdxRFInt;
class DgNdxHierIDGGSBase;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIDGG : public DgHierNdxSystemRF<DgQ2DICoord, long long int> {

   public:

      const DgNdxHierIDGGSBase& ndxHierDggs (void) { return ndxHierDggs_; }
      const DgIDGGS&            dggs        (void) { return dggs_; }

      int res      (void) const { return res_; }

      // get current, parent, and child dgg's
      const DgIDGG* dgg   (void) { return curResDgg_; }
      const DgIDGG* pDgg  (void) { return pResDgg_; }
      const DgIDGG* chDgg (void) { return chResDgg_; }

   protected:

      DgHierNdxIDGG (const DgNdxHierIDGGSBase& ndxHierDggsIn, int resIn, 
               bool outModeIntIn = true, const string& nameIn = "HierNdxIDGG");

      int setDgg (const DgIDGG** dgg, int res);

      const DgNdxHierIDGGSBase& ndxHierDggs_;
      const DgIDGGS& dggs_;
      int aperture_;

      const DgIDGG* curResDgg_;
      const DgIDGG* pResDgg_;
      const DgIDGG* chResDgg_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
