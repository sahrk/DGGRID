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
class DgHierNdxSystemRFBase;
class DgHierNdxSystemRFSBase;
template<class TINT, class TSTR> class DgHierNdxSystemRFS;

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

      const DgHierNdxSystemRFBase& sys;

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

      const DgHierNdxSystemRFBase& sys;

   friend class Dg2WayIntToStringConverter;
};

////////////////////////////////////////////////////////////////////////////////
class DgHierNdx2WayIntToStringConverter : public Dg2WayConverter {

   public:

     DgHierNdx2WayIntToStringConverter (const DgHierNdxSystemRFBase& sys);

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
class DgHierNdxSystemRFBase
        : public DgDiscRF<DgHierNdx, DgResAdd<DgQ2DICoord>, long long int> {

   struct DgSystemSet {
      const DgIDGG*         dgg_;
      const DgHierNdxIntRF* intRF_;
      const DgHierNdxStringRF* strRF_;
   };

   public:

      const DgHierNdxSystemRFSBase& hierNdxRFS (void) const
            { return hierNdxRFS_; }

      const DgIDGGS& dggs (void) const { return dggs_; }

      bool outModeInt (void);

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }
      
      const DgIDGG*            dgg   (void) const { return curRes_.dgg_; }
      const DgHierNdxIntRF*    intRF (void) const { return curRes_.intRF_; }
      const DgHierNdxStringRF* strRF (void) const { return curRes_.strRF_; }

      const DgIDGG*            pDgg   (void) const { return pRes_.dgg_; }
      const DgHierNdxIntRF*    pIntRF (void) const { return pRes_.intRF_; }
      const DgHierNdxStringRF* pStrRF (void) const { return pRes_.strRF_; }
      
      const DgIDGG*            chDgg   (void) const { return chRes_.dgg_; }
      const DgHierNdxIntRF*    chIntRF (void) const { return chRes_.intRF_; }
      const DgHierNdxStringRF* chStrRF (void) const { return chRes_.strRF_; }

      void setIntFromStringCoord (DgHierNdx& hn) const;
      void setStringFromIntCoord (DgHierNdx& hn) const;

      // provide default methods that quantify via the string representation
      virtual DgHierNdx quantify (const DgResAdd<DgQ2DICoord>& point) const;
      virtual DgResAdd<DgQ2DICoord> invQuantify (const DgHierNdx& add) const;
    
      // abstract methods
      virtual string add2str (const DgHierNdx& add) const;
       
      virtual string add2str (const DgHierNdx& add, char delimiter) const;
                          
      virtual const DgHierNdx& undefAddress (void) const;

      virtual const char* str2add (DgHierNdx* add, const char* str, char delimiter) const;
                                    
      // abstract methods for sub-classes; given dummy definitions here
      virtual DgHierNdxIntCoord toIntCoord (const DgHierNdxStringCoord& c) const;
      virtual DgHierNdxStringCoord toStringCoord (const DgHierNdxIntCoord& c) const;

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

      template<class TINT, class TSTR> friend class DgHierNdxSystemRFS;
};

////////////////////////////////////////////////////////////////////////////////
#endif
