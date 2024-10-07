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

#ifndef DGHIERNDXRFSBASE_H
#define DGHIERNDXRFSBASE_H

#include <vector>
#include <dglib/DgDiscRFS.h>

class DgHierNdx;

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxSystemRFSBase : 
              public DgDiscRFS<DgHierNdx, ResAdd<DgQ2DICoord>, long long int> {

   public:

      const DgIDGGBase& dggBase (int res) const
             { return static_cast<const DgIDGGBase&>(operator[](res)); }

      const DgIDGGSBase& dggs (void) { return dggs_; }

      virtual operator string (void) const
      {
         string s = "*** DgHierNdxSystemRFSBase";
         return s;
      }

      // externally an Int or Str form?
      bool outModeInt (void) { return outModeInt_; }
      void setOutModeInt (bool outModeIntIn = true) { outModeInt_ = outModeIntIn; }

      // indexing parent
      // only the DgLocation version performs checking on the input

      virtual void setNdxParent (int res, const DgLocation& loc, DgLocation& parent) const;

      virtual void setNdxParent (const DgResAdd<DgHierNdx>& add, const DgRFBase& rf,
                               DgLocation& parent) const
           {
             setNdxParent(add, parent);
             rf.convert(&parent);
           }

      virtual void setNdxParent (const DgResAdd<DgHierNdx>& add, DgLocation& parent) const
           {
             //add.clearAddress();
             convert(&parent);
             if (add.res() > 0 && add.res() < nRes())
	         setAddNdxParent(add, parent);
           }

      virtual DgLocation* makeNdxParent (int res, const DgLocation& loc) const
           {
             DgLocation* parent = new DgLocation(*this);
             setNdxParent(res, loc, *parent);

             return parent;
           }

      virtual DgLocation* makeNdxParent (const DgResAdd<DgHierNdx>& add) const
           {
             DgLocation* parent = new DgLocation(*this);
             setNdxParent(add, *parent);
             return parent;
           }

      // indexing children
      // only the DgLocation version performs checking on the input

      virtual void setNdxChildren (int res,
                    const DgLocation& loc, DgLocVector& chld) const;

      virtual void setNdxChildren (const DgResAdd<DgHierNdx>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& chld) const
           {
              setNdxChildren(add, chld);
              rf.convert(chld);
           }

      virtual void setNdxChildren (const DgResAdd<DgHierNdx>& add,
                                        DgLocVector& chld) const
           {
             chld.clearAddress();
             convert(chld);
             if (add.res() >= 0 && add.res() < (nRes() - 1)) {
                setAddNdxChildren(add, chld);
             }
           }

      virtual DgLocVector* makeNdxChildren (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* chld = new DgLocVector(*this);
             setNdxChildren(res, loc, *chld);

             return chld;
           }

      virtual DgLocVector* makeNdxChildren (const DgResAdd<DgHierNdx>& add) const
           {
             DgLocVector* chld = new DgLocVector(*this);
             setNdxChildren(add, *chld);

             return chld;
           }

   protected:

     DgHierNdxSystemRFSBase (const DgIDGGSBase& dggsIn, bool outModeIntIn = true,
            const string& nameIn = "HierNdxSystemRFS")
        : DiscRFS<DgHierNdx, ResAdd<DgQ2DICoord>, long long int> 
                    (dggsIn.network(), dggsIn, dggsIn.nRes(), nameIn),
          dggs_ (dggsIn), outModeInt_ (outModeIntIn)
     { 
        grids_ = new vector<const DgHierNdxSystemRFBase*>(nRes_, nullptr);
     }
                  
     // override the default definitions from DgDiscRFS (which quantify at
     // the highest resolution)
     virtual DgResAdd<DgHierNdx> quantify (const ResAdd<DgQ2DICoord>& point) const;
     virtual ResAdd<DgQ2DICoord> invQuantify (const DgResAdd<DgHierNdx>& add) const;

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
