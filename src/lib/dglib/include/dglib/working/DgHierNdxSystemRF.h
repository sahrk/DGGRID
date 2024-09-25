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

class DgHierNdxRFInt;
class DgHierNdxRFS;

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

   struct DgSystemSet {
      const DgHierNdxRFInt* intRF_;
      const DgHierNdxRFStr* strRF_;
   };

   public:

      const DgNdxHierRFS<B, DB>& ndxHierRFS (void) 
                { return ndxHierRFS_; }

      bool outModeInt (void) { return ndxHierRFS().outModeInt(); }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

      const DgHierNdxRFInt* intRF (void) { return curRes_.intRF_; }
      const DgHierNdxRFStr* strRF (void) { return curRes_.strRF_; }

      const DgHierNdxRFInt* pIntRF (void) { return pRes_.intRF_; }
      const DgHierNdxRFStr* pStrRF (void) { return pRes_.strRF_; }
      
      const DgHierNdxRFInt* chIntRF (void) { return chRes_.intRF_; }
      const DgHierNdxRFStr* chStrRF (void) { return chRes_.strRF_; }

   protected:

      DgHierNdxSystemRF<B, DB> (
            const DgNdxHierRFS<B, DB>& ndxHierRFSIn, int resIn, 
            const string& nameIn = "HierNdxSysRF");

      int setSystemSet (DgSystemSet& set, int res);
      virtual void initialize (void);

      const DgNdxHierRFS<B, DB>& ndxHierRFS_;
      int res_;

      DgSystemSet pRes_;
      DgSystemSet curRes_;
      DgSystemSet chRes_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
