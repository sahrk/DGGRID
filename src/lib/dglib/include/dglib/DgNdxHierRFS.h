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

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> class DgNdxHierRFS {

   public:
    
      const DgDiscRFS<A, B, DB>& rfs (void) const { return rfs_; }

      // indexing parent
      // only the DgLocation version performs checking on the input

      virtual void setNdxParent (int res, const DgLocation& loc, DgLocation& parent) const;

      virtual void setNdxParent (const DgResAdd<A>& add, const DgRFBase& rf,
                               DgLocation& parent) const
           {
             setNdxParent(add, parent);
             rf.convert(&parent);
           }

      virtual void setNdxParent (const DgResAdd<A>& add, DgLocation& parent) const
           {
             //add.clearAddress();
             rfs().convert(&parent);
             if (add.res() > 0 && add.res() < rfs().nRes())
	         setAddNdxParent(add, parent);
           }

      virtual DgLocation* makeNdxParent (int res, const DgLocation& loc) const
           {
             DgLocation* parent = new DgLocation(rfs());
             setNdxParent(res, loc, *parent);

             return parent;
           }

      virtual DgLocation* makeNdxParent (const DgResAdd<A>& add) const
           {
             DgLocation* parent = new DgLocation(rfs());
             setNdxParent(add, *parent);
             return parent;
           }

      // indexing children
      // only the DgLocation version performs checking on the input

      virtual void setNdxChildren (int res,
                    const DgLocation& loc, DgLocVector& chld) const;

      virtual void setNdxChildren (const DgResAdd<A>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& chld) const
           {
              setNdxChildren(add, chld);
              rf.convert(chld);
           }

      virtual void setNdxChildren (const DgResAdd<A>& add,
                                        DgLocVector& chld) const
           {
             chld.clearAddress();
             rfs().convert(chld);
             if (add.res() >= 0 && add.res() < (rfs().nRes() - 1)) {
                setAddNdxChildren(add, chld);
             }
           }

      virtual DgLocVector* makeNdxChildren (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* chld = new DgLocVector(rfs());
             setNdxChildren(res, loc, *chld);

             return chld;
           }

      virtual DgLocVector* makeNdxChildren (const DgResAdd<A>& add) const
           {
             DgLocVector* chld = new DgLocVector(rfs());
             setNdxChildren(add, *chld);

             return chld;
           }

      virtual operator string (void) const
      {
/*
         string s = "*** DgNdxHierRFS " + DgRFBase::name() +
               "\nap: " + dgg::util::to_string(aperture()) +
               "\nnRes: " + dgg::util::to_string(nRes()) +
               "\nisCongruent: " + dgg::util::to_string(isCongruent()) +
               "\nisAligned: " + dgg::util::to_string(isAligned()) + "\n";
         for (int i = 0; i < nRes(); i++)
            s += " >>> " + dgg::util::to_string(i) + ": " +
                   string(*(*grids_)[i]) + "\n";

*/
         string s = "*** DgNdxHierRFS";
         return s;
      }

   protected:

     DgNdxHierRFS (const DgDiscRFS<A, B, DB>& rfsIn)
        : rfs_ (rfsIn)
     { }

     // new pure virtual functions
     virtual void setAddNdxParent (const DgResAdd<A>& add,
                                   DgLocation& parent) const = 0;
     virtual void setAddNdxChildren (const DgResAdd<A>& add,
                                     DgLocVector& children) const = 0;

   private:

     // state data
     const DgDiscRFS<A, B, DB>& rfs_;
};

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> ostream& operator<< (ostream& stream,
          const DgNdxHierRFS<A, B, DB>& g)
{
   stream << string(g) << endl;

   return stream;
}

// JFW: is this really what we mean?
#include "../lib/DgNdxHierRFS.hpp"

#endif
