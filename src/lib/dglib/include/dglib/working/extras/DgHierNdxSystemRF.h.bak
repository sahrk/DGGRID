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
// DgHierNdxSystemRF.h: DgHierNdxSystemRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTEMRF_H
#define DGHIERNDXSYSTEMRF_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>

class DgHierNdxRFInt;
class DgNdxHierIDGGSBase;

using namespace std;

// need to put in appropriate include
#ifndef HIERNDX_INT_TYPE
#define HIERNDX_INT_TYPE uint64_t
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgHierNdx {
// should these be ZXCoord/ZXStringCoord?
   HIERNDX_INT_TYPE intNdx_;
   string strNdx_;
};

// use Int as a standin until class is complete
typedef DgHierNdxRFStr DgHierNdxRFInt;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystemRF<B, DB> : DgDiscRF<DgHierNdx, B, DB> {

   typedef struct {
      const DgIDGG* dgg_;
      const DgHierNdxRFInt* intRF_;
      const DgHierNdxRFStr* strRF_;
   } DgSystemSet;

   public:

      const DgNdxHierIDGGSBase& ndxHierDggs (void) { return ndxHierDggs_; }
      const DgIDGGS&            dggs        (void) { return dggs_; }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

      const DgIDGG*         dgg   (void) { return curRes_.dgg_; }
      const DgHierNdxRFInt* intRF (void) { return curRes_.intRF_; }
      const DgHierNdxRFStr* strRF (void) { return curRes_.strRF_; }

      const DgIDGG*         pDgg   (void) { return pRes_.dgg_; }
      const DgHierNdxRFInt* pIntRF (void) { return pRes_.intRF_; }
      const DgHierNdxRFStr* pStrRF (void) { return pRes_.strRF_; }

      const DgIDGG*         chDgg   (void) { return chRes_.dgg_; }
      const DgHierNdxRFInt* chIntRF (void) { return chRes_.intRF_; }
      const DgHierNdxRFStr* chStrRF (void) { return chRes_.strRF_; }

   protected:

      DgHierNdxSystemRF (const DgNdxHierIDGGSBase& ndxHierDggsIn, int resIn, 
               bool outModeIntIn = true, const string& nameIn = "HierNdxSysRF");

      int setSystemSet (DgSystemSet& set, int res);

      const DgNdxHierIDGGSBase& ndxHierDggs_;
      const DgIDGGS& dggs_;
      int res_;
      int aperture_;

      DgSystemSet pRes_;
      DgSystemSet curRes_;
      DgSystemSet chRes_;

      // use int or str for output?
      bool outModeInt_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
