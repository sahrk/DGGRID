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
// DgHierNdxSystem.h: DgHierNdxSystem header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTEM_H
#define DGHIERNDXSYSTEM_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>

class DgHierNdxRFInt;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystem {

   typedef struct {
      const DgIDGG* dgg_;
      const DgHierNdxRFInt* intRF_;
   } DgSystemSet;

   public:

      const DgIDGGS& dggs (void) { return dggs_; }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

      const DgIDGG*         dgg   (void) { return curRes_.dgg_; }
      const DgHierNdxRFInt* intRF (void) { return curRes_.intRF_; }

      const DgIDGG*         pDgg   (void) { return pRes_.dgg_; }
      const DgHierNdxRFInt* pIntRF (void) { return pRes_.intRF_; }

      const DgIDGG*         chDgg   (void) { return chRes_.dgg_; }
      const DgHierNdxRFInt* chIntRF (void) { return chRes_.intRF_; }

   protected:

      DgHierNdxSystem (const DgIDGGS& dggsIn, int resIn, const string& nameIn);

      int setSystemSet (DgSystemSet& set, int res);

      const DgIDGGS& dggs_;
      int res_;
      int aperture_;

      DgSystemSet pRes_;
      DgSystemSet curRes_;
      DgSystemSet chRes_;

      //const DgHierNdxRFInt* intRF_;

      //const DgIDGG& dgg_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
