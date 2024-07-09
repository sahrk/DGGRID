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

////////////////////////////////////////////////////////////////////////////////
template<class A> class DgNdxHierRFS {

   public:

      // indexing parent
      // only the DgLocation version performs checking on the input

      virtual void setNdxParent (const DgLocation& loc, DgLocation& parent) const;

      virtual void setNdxParent (const DgResAdd<A>& add, const DgRFBase& rf,
                               DgLocation& parent) const
           {
             setNdxParent(add, parent);
             rf.convert(parent);
           }

      virtual void setNdxParent (const DgResAdd<A>& add, DgLocation& parent) const
           {
             add.clearAddress();
             this->convert(parent);
             if (add.res() > 0 && add.res() < nRes())
	        setAddNdxParent(add, parent);
           }

      virtual DgLocation* makeNdxParent (int res, const DgLocation& loc) const
           {
             DgLocation* parent = new DgLocation(*this);
             setNdxParent(res, loc, *parent);

             return parent;
           }

      virtual DgLocation* makeNdxParent (const DgResAdd<A>& add) const
           {
             DgLocation* parent = new DgLocation(*this);
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
             this->convert(chld);
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

      virtual DgLocVector* makeNdxChildren (const DgResAdd<A>& add) const
           {
             DgLocVector* chld = new DgLocVector(*this);
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

      DgNdxHierRFS (DgRFNetwork& network, const DgRF<B, DB>& backFrame,
                 int nResIn, unsigned int aperture,
                 dgg::topo::DgGridTopology gridTopo = dgg::topo::Hexagon,
                 dgg::topo::DgGridMetric gridMetric = dgg::topo::D6,
                 bool isCongruent = true, bool isAligned = false,
                 const string& name = "DiscS")
        : DgDiscRF<DgResAdd<A>>
                      (network, backFrame, name, gridTopo, gridMetric),
          aperture_ (aperture), grids_ (new vector<const DgDiscRF<A>*>()),
          nRes_ (nResIn), isCongruent_ (isCongruent),
          isAligned_ (isAligned)
        {
          if (nRes() < 0)
          {
             report("DgNdxHierRFS<A>::DgDiscRF() nRes < 0",
                    DgBase::Fatal);
          }

          if (!this->isAligned() && !this->isCongruent())
          {
             report("DgNdxHierRFS::DgNdxHierRFS() grid system must be either "
                    "congruent, aligned, or both", DgBase::Fatal);
          }

          grids_->resize(nRes());
        }

      // new pure virtual functions

      virtual void setAddParent (const DgResAdd<A>& add,
                                  DgLocation& parent) const = 0;

      // state data

      const DgRF<B, DB>* backFrame_;

      unsigned int aperture_;

      vector<const DgDiscRF<A>*>* grids_;

      int nRes_;
      bool isCongruent_;
      bool isAligned_;

};

////////////////////////////////////////////////////////////////////////////////
template<class A> ostream& operator<< (ostream& stream,
          const DgNdxHierRFS<A>& g)
{
   stream << string(g) << endl;

   return stream;
}

// JFW: is this really what we mean?
#include "../lib/DgNdxHierRFS.hpp"

#endif
