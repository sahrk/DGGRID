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
// DgHierNdxSystemRFSBase.h: DgHierNdxSystemRFSBase class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXSYSTEMRFSBASE_H
#define DGHIERNDXSYSTEMRFSBASE_H

#include <vector>
#include <dglib/DgDiscRFS.h>

class DgHierNdxSystemRFBase;
class DgHierNdx;
class DgQ2DICoord;
class DgIDGGSBase;
class DgIDGGBase;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystemRFSBase : 
              public DgDiscRFS<DgHierNdx, DgQ2DICoord, long long int> {

   public:

      static const DgResAdd<DgHierNdx> undefCoord;
                  
      const DgHierNdxSystemRFBase& sysRF (int res) const;
                  
      const DgIDGGSBase& dggs (void) const { return dggs_; }
      const DgIDGGBase& dgg (int res) const;
 
      virtual operator string (void) const
      {
         string s = "*** DgHierNdxSystemRFSBase";
         return s;
      }

      // externally an Int or Str form?
      bool outModeInt (void) const { return outModeInt_; }
      void setOutModeInt (bool outModeIntIn = true)
               { outModeInt_ = outModeIntIn; }

      // indexing parent
      // only the DgLocation version performs checking on the input

      virtual void setNdxParent (int res, const DgLocation& loc, DgLocation& parent) const;
      virtual void setNdxParent (const DgResAdd<DgHierNdx>& add, const DgRFBase& rf,
                               DgLocation& parent) const;
      virtual void setNdxParent (const DgResAdd<DgHierNdx>& add, DgLocation& parent) const;
      virtual DgLocation* makeNdxParent (int res, const DgLocation& loc) const;
      virtual DgLocation* makeNdxParent (const DgResAdd<DgHierNdx>& add) const;

      // indexing children
      // only the DgLocation version performs checking on the input
      virtual void setNdxChildren (int res, const DgLocation& loc, DgLocVector& chld) const;
      virtual void setNdxChildren (const DgResAdd<DgHierNdx>& add, const DgRFBase& rf, DgLocVector& chld) const;
      virtual void setNdxChildren (const DgResAdd<DgHierNdx>& add, DgLocVector& chld) const;
      virtual DgLocVector* makeNdxChildren (int res, const DgLocation& loc) const;
      virtual DgLocVector* makeNdxChildren (const DgResAdd<DgHierNdx>& add) const;

   protected:

     DgHierNdxSystemRFSBase (const DgIDGGSBase& dggsIn, bool outModeIntIn = true,
            const string& nameIn = "HierNdxSystemRFS");
                  
     // from above
     virtual const DgResAdd<DgHierNdx>& undefAddress (void) const;
/*
     // override the default definitions from DgDiscRFS (which quantify at
     // the highest resolution)
     virtual DgResAdd<DgHierNdx> quantify (const DgResAdd<DgQ2DICoord>& point) const;
     virtual DgResAdd<DgQ2DICoord> invQuantify (const DgResAdd<DgHierNdx>& add) const;
 */

     // pure virtual functions passed down from above
     virtual void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const = 0;

     // state data
     const DgIDGGSBase& dggs_;
     bool outModeInt_;

};

////////////////////////////////////////////////////////////////////////////////

#endif
