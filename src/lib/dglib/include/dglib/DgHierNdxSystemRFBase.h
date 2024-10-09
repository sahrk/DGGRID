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
// DgHierNdxSystemRFBase.h: DgHierNdxSystemRFBase header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTEMRFBASE_H
#define DGHIERNDXSYSTEMRFBASE_H

#include <climits>
#include <iostream>

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>
#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>
//#include <dglib/DgHierNdxIntRF.h>
//#include <dglib/DgHierNdxStringRF.h>

//class DgHierNdxSystemRFSBase;
//class DgHierNdxRFS;
class DgHierNdx;
class DgHierNdxIntRF;
class DgHierNdxStringCoord;
class DgHierNdxIntCoord;
class DgHierNdxStringRF;
class DgHierNdxSystemRF;
class DgHierNdxSystemRFSBase;

using namespace std;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxStringToIntConverter :
        public DgConverter<DgHierNdxStringCoord, long long int, DgHierNdxIntCoord,
                            long long int>
{
   public:

      virtual DgHierNdxIntCoord convertTypedAddress
                                (const DgHierNdxStringCoord& addIn) const;

   private:

      DgHierNdxStringToIntConverter (const DgHierNdxStringRF& from, 
                                     const DgHierNdxIntRF& to);

      const DgHierNdxSystemRF& sys;

   friend class Dg2WayIntToStringConverter;
};

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxIntToStringConverter :
        public DgConverter<DgHierNdxIntCoord, long long int, DgHierNdxStringCoord, 
                            long long int>
{
   public:

      virtual DgHierNdxStringCoord convertTypedAddress
                                (const DgHierNdxIntCoord& addIn) const;

   private:

      DgHierNdxIntToStringConverter (const DgHierNdxIntRF& from, 
            const DgHierNdxStringRF& to);

      const DgHierNdxSystemRF& sys;

   friend class Dg2WayIntToStringConverter;
};

////////////////////////////////////////////////////////////////////////////////
class DgHierNdx2WayIntToStringConverter : public Dg2WayConverter {

   public:

     DgHierNdx2WayIntToStringConverter (const DgHierNdxSystemRF& sys);

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
class DgHierNdx {
   DgHierNdxIntCoord intNdx_;
   DgHierNdxStringCoord strNdx_;
};
*/

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystemRFBase : DgDiscRF<DgHierNdx, DgQ2DICoord, long long int> {

   struct DgSystemSet {
      const DgIDGG*         dgg_;
      const DgHierNdxIntRF* intRF_;
      const DgHierNdxStringRF* strRF_;
   };

   public:

      const DgHierNdxSystemRFSBase& hierNdxRFS (void) { return hierNdxRFS_; }

      const DgIDGGS& dggs (void) { return dggs_; }

      bool outModeInt (void);

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }
      
      const DgIDGG*            dgg   (void) { return curRes_.dgg_; }
      const DgHierNdxIntRF*    intRF (void) { return curRes_.intRF_; }
      const DgHierNdxStringRF* strRF (void) { return curRes_.strRF_; }

      const DgIDGG*            pDgg   (void) { return pRes_.dgg_; }
      const DgHierNdxIntRF*    pIntRF (void) { return pRes_.intRF_; }
      const DgHierNdxStringRF* pStrRF (void) { return pRes_.strRF_; }
      
      const DgIDGG*            chDgg   (void) { return chRes_.dgg_; }
      const DgHierNdxIntRF*    chIntRF (void) { return chRes_.intRF_; }
      const DgHierNdxStringRF* chStrRF (void) { return chRes_.strRF_; }

      void setIntFromStringCoord (DgHierNdx& hn) const
               { hn.intNdx_ = toIntCoord(hn.strNdx_); }

      void setStringFromIntCoord (DgHierNdx& hn) const
               { hn.strNdx_ = toStringCoord(hn.intNdx_); }

      // provide default methods that quantify via the string representation
      virtual DgHierNdx quantify (const DgQ2DICoord& point) const;
      virtual DgQ2DICoord invQuantify (const DgHierNdx& add) const;

      // abstract methods for sub-classes
      virtual DgHierNdxIntCoord toIntCoord (const DgHierNdxStringCoord& c) = 0;
      virtual DgHierNdxStringCoord toStringCoord (const DgHierNdxIntCoord& c) = 0;

   protected:

      DgHierNdxSystemRFBase (const DgHierNdxSystemRFSBase& hierNdxRFSIn, int resIn,
            const string& nameIn = "HierNdxSysRFBase");

      int setSystemSet (DgSystemSet& set, int res);
      virtual void initialize (void);

      const DgHierNdxSystemRFSBase& hierNdxRFS_;
      const DgIDGGS& dggs_;
      int res_;
      int aperture_;

      DgSystemSet pRes_;
      DgSystemSet curRes_;
      DgSystemSet chRes_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
