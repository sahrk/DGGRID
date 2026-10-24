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
#include <dglib/DgHierNdx.h>
#include <dglib/DgAddressType.h>
#include <dglib/DgIDGGutil.h>

using namespace dgg::addtype;

class DgHierNdxSystemRFBase;
class DgIDGGSBase;
class DgIDGGBase;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystemRFSBase :
              public DgDiscRFS<DgHierNdx, DgQ2DICoord, long long int> {

   public:

      static DgHierNdxSystemRFSBase* makeSystem (const DgIDGGSBase& dggsIn,
               DgHierNdxSysType sysType = Z3, DgHierNdxFormType form = Int64,
               const std::string& nameIn = "Z3");

      static const DgResAdd<DgHierNdx> undefCoord;

      const DgHierNdxSystemRFBase& sysRF (int res) const;

      const DgIDGGSBase& dggs (void) const { return dggs_; }
      const DgIDGGBase& dgg (int res) const;

      // DGGS resolutions [firstNdxRes(), lastNdxRes()] carry indexes. Other
      // resolutions still exist in grids() (so grids()[r] stays aligned with
      // the DGGS) but have no index; converting to or from them is fatal.
      // Most systems index every resolution (0 to nRes() - 1); H3 starts at
      // DGGS resolution 2 (H3 resolution 0, its base cells).
      int  firstNdxRes (void) const { return firstNdxRes_; }
      int  lastNdxRes  (void) const { return lastNdxRes_; }
      bool isNdxRes    (int res) const
                 { return res >= firstNdxRes_ && res <= lastNdxRes_; }

      // the system's own resolution number for a DGGS resolution (H3: res - 2)
      int  sysRes      (int dggsRes) const { return dggsRes - firstNdxRes_; }

      // DGGS resolution of the indexing parent/child of a cell at res, or -1.
      // A sparse system (e.g. Superfund: 1,2,4,...,16,17) would replace these
      // two and the stored range with a resolution map; they are the only
      // places that assume consecutive indexed resolutions.
      int  ndxParentRes (int res) const;
      int  ndxChildRes  (int res) const;

      // Resolution 0 has a base grid aperture, but no incoming step.
      unsigned int apertureAtResolution (int res) const;
      unsigned int parentStepAperture (int res) const;
      unsigned int childStepAperture (int res) const;

      virtual operator std::string (void) const
      {
         std::string s = "*** DgHierNdxSystemRFSBase";
         return s;
      }

      // externally an Int or Str form?
      bool extModeInt (void) const { return extModeInt_; }
      void setOutModeInt (bool extModeIntIn = true)
               { extModeInt_ = extModeIntIn; }

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

      // convenience methods
      virtual void setNdxChildren (const DgResAdd<DgQ2DICoord>& add, DgLocVector& chld) const;
      virtual void setNdxParent (const DgResAdd<DgQ2DICoord>& add, DgLocation& parent) const;

   protected:

     // lastNdxRes of -1 means nRes() - 1; it is clamped to nRes() - 1. These are
     // constructor parameters (not virtuals) since they are needed while the
     // per-resolution RFs are built, before the concrete system exists.
     DgHierNdxSystemRFSBase (const DgIDGGSBase& dggsIn, bool extModeIntIn = true,
            const std::string& nameIn = "HierNdxSystemRFS",
            int firstNdxResIn = 0, int lastNdxResIn = -1);

     // from above
     virtual const DgResAdd<DgHierNdx>& undefAddress (void) const;
/*
     // override the default definitions from DgDiscRFS (which quantify at
     // the highest resolution)
     virtual DgResAdd<DgHierNdx> quantify (const DgResAdd<DgQ2DICoord>& point) const;
     virtual DgResAdd<DgQ2DICoord> invQuantify (const DgResAdd<DgHierNdx>& add) const;
 */

     // pure virtual functions to be defined by particular systems
     virtual DgHierNdxIntCoord toIntCoord (const DgHierNdxStringCoord& c,
                                           int gridRes) const = 0;
     virtual DgHierNdxStringCoord toStringCoord (const DgHierNdxIntCoord& c,
                                                 int gridRes) const = 0;

     virtual void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const = 0;

     // state data
     const DgIDGGSBase& dggs_;
     bool extModeInt_;
     int firstNdxRes_;
     int lastNdxRes_;

     friend class DgHierNdxSystemRFBase;
};

////////////////////////////////////////////////////////////////////////////////

#endif
