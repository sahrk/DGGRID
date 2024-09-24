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
// DgNdxHierRFS.h: DgNdxHierRFS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGNDXHIERRFS_H
#define DGNDXHIERRFS_H

#include <vector>
#include <dglib/DgDiscRFS.h>

class DgHierNdx;

////////////////////////////////////////////////////////////////////////////////
template<class B, class DB> class DgNdxHierRFS : 
                                          public DgDiscRFS<DgHierNdx, B, DB> {

   public:

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

     DgNdxHierRFS <B, DB>(void)
        : grids_ (new vector<const DgHierNdxSystemRF<B, DB>*>(nRes_, nullptr))
     { }

     // new pure virtual functions
     virtual void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const = 0;

};

////////////////////////////////////////////////////////////////////////////////

// JFW: is this really what we mean?
#include "../lib/DgNdxHierRFS.hpp"

#endif
