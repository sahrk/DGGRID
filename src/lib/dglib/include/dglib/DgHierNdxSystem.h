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
typedef struct {
      const DgIDGG* dgg_;
      const DgHierNdxRFInt* intRF_;
} DgSystemSet;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystem {

   public:

      const DgHierNdxRFInt& intRF (void) { return *intRF; }

      const DgIDGGS& dggs (void) { return dggs_; }
      const DgIDGG& dgg (void) { return dgg_; }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

   protected:

      DgHierNdxSystem (const DgIDGGS& dggsIn, int resIn, const string& nameIn)
           dggs_ (dggsIn), res_ (resIn), aperture_ (dggsIn.aperture());
           pRes_ {nullptr, nullptr, nullptr},
           curRes_ {nullptr, nullptr, nullptr},
           chRes_ {nullptr, nullptr, nullptr},

//dgg_ (dggsIn.idgg(resIn)), 

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
