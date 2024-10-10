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
// DgDiscTopoRFS.h: DgDiscRFS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDISCTOPORFS_H
#define DGDISCTOPORFS_H

#include <vector>

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
//#include <dglib/DgDiscTopoRF.h>
#include <dglib/DgDiscRFS.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgRF.h>

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> class DgDiscTopoRFS
                               : public DgDiscRFS<A, B, DB> {

   public:

      DgDiscTopoRFS<A, B, DB>& operator= (const DgDiscTopoRFS<A, B, DB>& rf)
      // shallow copy with possible memory leak; don't use if avoidable
          {
             if (*this != rf)
             {
                DgDiscRFS<A, B, DB>::operator=(rf);
                aperture_ = rf.aperture();
                isCongruent_ = rf.isCongruent();
                isAligned_ = rf.isAligned();

                this->grids_ = new vector<const DgDiscRF<A, B, DB>*>(rf.nRes(), nullptr);
                for (int i = 0; i < rf.nRes(); i++)
                {
                   // KLUDGE: don't know real type of each grid so can't
                   // easily create them here; opt for shallow copy
                   (*(this->grids_))[i] = rf.grids()[i];
                }
            }

            return *this;
         }

      unsigned int aperture (void) const { return aperture_; }

      bool isCongruent (void) const { return isCongruent_; }
      bool isAligned   (void) const { return isAligned_; }

      // no bounds checking
      const DgDiscTopoRF<A, B, DB>& topoRF (int res) const
        { return *(static_cast<const DgDiscTopoRF<A, B, DB>*>(this->grids()[res])); }

      // parents

      // only the DgLocation version performs checking on the input

      virtual void setParents (int res,
                    const DgLocation& loc, DgLocVector& vec) const;

      virtual void setParents (const DgResAdd<A>& add, const DgRFBase& rf,
                               DgLocVector& vec) const
           {
             setParents(add, vec);
             rf.convert(vec);
           }

      virtual void setParents (const DgResAdd<A>& add, DgLocVector& vec) const
           {
             vec.clearAddress();
             this->convert(vec);
             if (add.res() > 0 && add.res() < this->nRes())
	            setAddParents(add, vec);
           }

      virtual DgLocVector* makeParents (int res, const DgLocation& loc) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setParents(res, loc, *vec);

             return vec;
           }

      virtual DgLocVector* makeParents (const DgResAdd<A>& add) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setParents(add, *vec);
             return vec;
           }

      // interior children

      // only the DgLocation version performs checking on the input

      virtual void setInteriorChildren (int res,
                    const DgLocation& loc, DgLocVector& vec) const;

      virtual void setInteriorChildren (const DgResAdd<A>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& vec) const
           {
              setInteriorChildren(add, vec);
              rf.convert(vec);
           }

      virtual void setInteriorChildren (const DgResAdd<A>& add,
                                        DgLocVector& vec) const
           {
             vec.clearAddress();
             this->convert(vec);
             if (add.res() >= 0 && add.res() < (this->nRes() - 1))
             {
                setAddInteriorChildren(add, vec);
             }
           }

      virtual DgLocVector* makeInteriorChildren (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setInteriorChildren(res, loc, *vec);

             return vec;
           }

      virtual DgLocVector* makeInteriorChildren (const DgResAdd<A>& add) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setInteriorChildren(add, *vec);

             return vec;
           }

      // boundary children

      // only the DgLocation version performs checking on the input

      virtual void setBoundaryChildren (int res,
                    const DgLocation& loc, DgLocVector& vec) const;

      virtual void setBoundaryChildren (const DgResAdd<A>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& vec) const
           {
              setBoundaryChildren(add, vec);
              rf.convert(vec);
           }

      virtual void setBoundaryChildren (const DgResAdd<A>& add,
                                        DgLocVector& vec) const
           {
             vec.clearAddress();
             this->convert(vec);
             if (add.res() >= 0 && add.res() < (this->nRes() - 1))
             {
                setAddBoundaryChildren(add, vec);
             }
           }

      virtual DgLocVector* makeBoundaryChildren (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setBoundaryChildren(res, loc, *vec);

             return vec;
           }

      virtual DgLocVector* makeBoundaryChildren (const DgResAdd<A>& add) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setBoundaryChildren(add, *vec);

             return vec;
           }

      // second order boundary children (hex aperture 7 only)

      // only the DgLocation version performs checking on the input

      virtual void setBoundary2Children (int res,
                    const DgLocation& loc, DgLocVector& vec) const;

      virtual void setBoundary2Children (const DgResAdd<A>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& vec) const
           {
              setBoundary2Children(add, vec);
              rf.convert(vec);
           }

      virtual void setBoundary2Children (const DgResAdd<A>& add,
                                        DgLocVector& vec) const
           {
             vec.clearAddress();
             this->convert(vec);
             if (add.res() >= 0 && add.res() < (this->nRes() - 1)) {
                setAddBoundary2Children(add, vec);
             }
           }

      virtual DgLocVector* makeBoundary2Children (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setBoundary2Children(res, loc, *vec);

             return vec;
           }

      virtual DgLocVector* makeBoundary2Children (const DgResAdd<A>& add) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setBoundary2Children(add, *vec);

             return vec;
           }

      // all children (interior and boundary)

      // only the DgLocation version performs checking on the input

      virtual void setAllChildren (int res,
                    const DgLocation& loc, DgLocVector& vec) const;

      virtual void setAllChildren (const DgResAdd<A>& add,
                                        const DgRFBase& rf,
                                        DgLocVector& vec) const
           {
              setAllChildren(add, vec);
              rf.convert(vec);
           }

      virtual void setAllChildren (const DgResAdd<A>& add,
                                        DgLocVector& vec) const
           {
             vec.clearAddress();
             this->convert(vec);
             if (add.res() >= 0 && add.res() < (this->nRes() - 1)) {
                setAddAllChildren(add, vec);
             }
           }

      virtual DgLocVector* makeAllChildren (int res,
                                                 const DgLocation& loc) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setAllChildren(res, loc, *vec);

             return vec;
           }

      virtual DgLocVector* makeAllChildren (const DgResAdd<A>& add) const
           {
             DgLocVector* vec = new DgLocVector(*this);
             setAllChildren(add, *vec);

             return vec;
           }

   protected:

      DgDiscTopoRFS (DgRFNetwork& network, const DgRF<B, DB>& backFrame,
                 int nRes, unsigned int aperture,
                 dgg::topo::DgGridTopology gridTopo = dgg::topo::Hexagon,
                 dgg::topo::DgGridMetric gridMetric = dgg::topo::D6,
                 bool isCongruent = true, bool isAligned = false,
                 const string& name = "DiscS")
        : DgDiscRFS<A, B, DB> (network, backFrame, nRes, name),
          aperture_ (aperture), isCongruent_ (isCongruent), isAligned_ (isAligned)
        {
          this->grids_ = new vector<const DgDiscRF<A, B, DB>*>(nRes, nullptr);
          if (!this->isAligned() && !this->isCongruent()) {
             report("DgDiscRFS::DgDiscRFS() grid system must be either "
                    "congruent, aligned, or both", DgBase::Fatal);
          }
        }

      DgDiscTopoRFS (const DgDiscTopoRFS<A, B, DB>& rf) // uses dubious operator=
        : DgDiscRFS<A, B, DB> (rf)
        { *this = rf; }


      virtual void setAddVertices (const DgResAdd<A>& add,
                                   DgPolygon& vec) const
                    { this->grids()[add.res()]->backFrame().convert(vec);
                      topoRF(add.res())->setVertices(add.address(), vec);
                      this->backFrame().convert(vec);
		    }

      virtual void setAddNeighbors (const DgResAdd<A>& add,
                                    DgLocVector& vec) const;

      // second order boundary children; aperture 7 hex only
      virtual void setAddBoundary2Children (const DgResAdd<A>&, DgLocVector&) const { }

      // new pure virtual functions

      virtual void setAddParents (const DgResAdd<A>& add,
                                  DgLocVector& vec) const = 0;

      virtual void setAddInteriorChildren (const DgResAdd<A>& add,
                                           DgLocVector& vec) const = 0;

      virtual void setAddBoundaryChildren (const DgResAdd<A>& add,
                                           DgLocVector& vec) const = 0;

      virtual void setAddAllChildren (const DgResAdd<A>& add,
                                      DgLocVector& vec) const = 0;

      // state data
      int aperture_;
      bool isCongruent_;
      bool isAligned_;

};

// JFW: is this really what we mean?
#include "../lib/DgDiscTopoRFS.hpp"

#endif
