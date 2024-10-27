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
// DgDiscRFS.h: DgDiscRFS class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDISCRFS_H
#define DGDISCRFS_H

#include <vector>

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
#include <dglib/DgDiscRF.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgRF.h>
#include <dglib/DgDiscRFSGrids.h>

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> class DgDiscRFS
         : public DgDiscRF<DgResAdd<A>, B, DB>, 
           public DgDiscRFSGrids<DgDiscRF, A, B, DB> {

   public:

      DgDiscRFS (DgRFNetwork& network, const DgRF<B, DB>& backFrame,
                 int nResIn, const string& name = "DiscS")
        : DgDiscRF<DgResAdd<A>, B, DB> (network, backFrame, name), 
          DgDiscRFSGrids<DgDiscRF, A, B, DB> (backFrame, nResIn)
        {
        }

      DgDiscRFS (const DgDiscRFS<A, B, DB>& rf) // uses dubious operator=
        : DgDiscRF<DgResAdd<A>, B, DB> (rf)
        { *this = rf; }

      //vector<const DgDiscRF<A, B, DB>*>& gridsMutable (void) const { return *grids_; }

      virtual DgResAdd<A> quantify (const B& point) const
            {
               // quantify using max res grid

               int maxRes = nRes() - 1;
               DgLocation* loc = this->backFrame().makeLocation(point);
               const DgDiscRF<A, B, DB>& grid = *grids()[maxRes];
               grid.convert(loc);
               DgResAdd<A> add(*grid.getAddress(*loc), maxRes);

               delete loc;

               return add;
             }

      virtual B invQuantify (const DgResAdd<A>& add) const
             {
               const DgDiscRF<A, B, DB>& grid = *grids()[add.res()];
               DgLocation* loc = grid.makeLocation(add.address());
               this->backFrame().convert(loc);
               B newAdd(*(this->backFrame().getAddress(*loc)));
               delete loc;
               return newAdd;
             }

      // state data

      const DgRF<B, DB>* backFrame_;
      vector<const DgDiscRF<A, B, DB>*>* grids_;
      int nRes_;

};

////////////////////////////////////////////////////////////////////////////////
template<class A, class B, class DB> ostream& operator<< (ostream& stream,
          const DgDiscRFS<A, B, DB>& g)
{
   stream << string(g) << endl;

   return stream;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template <class A, class B, class DB> class DgResAddConverter :
    public DgConverter<DgResAdd<A>, long long int, A, long long int> {

   public:

      DgResAddConverter (const DgDiscRFS<A, B, DB>& fromFrame,
                         const DgDiscRF<A, B, DB>& toFrame, int resIn)
         : DgConverter<DgResAdd<A>, long long int, A,
                 long long int> (fromFrame, toFrame),
           res_ (resIn), discRFS_ (fromFrame), discRF_ (toFrame)
           {
	      // JFW: second clause will never be > (int vs long):
              if (res() < 0 ||
                  static_cast<unsigned long>(res()) >= discRFS().grids().size())
              {
                 report("DgResAddConverter<A, B, DB>::DgResAddConverter() "
                        "invalid resolution", DgBase::Fatal);
              }

              if (*(discRFS().grids()[res()]) != discRF())
              {
                 report("DgDgResAddConverter<A, B, DB>::DgResAddConverter() "
                        "grid mismatch", DgBase::Fatal);
              }
           }

      DgResAddConverter (const DgResAddConverter& con)
         : DgConverter<DgResAdd<A>, long long int, A, long long int> (con) { }

      int res (void) const { return res_; }

      const DgDiscRFS<A, B, DB>& discRFS (void) const { return discRFS_; }

      const DgDiscRF<A, B, DB>& discRF (void) const { return discRF_; }

      virtual A convertTypedAddress (const DgResAdd<A>& add) const
        {
           if (add.res() == res()) return add.address();

           DgLocation* tmpLoc =
             discRFS().grids()[add.res()]->makeLocation(add.address());
           discRF().convert(tmpLoc);
           A newAdd = *(discRF().getAddress(*tmpLoc));
           delete tmpLoc;
           return newAdd;
        }

   protected:

      int res_;
      const DgDiscRFS<A, B, DB>& discRFS_;
      const DgDiscRF<A, B, DB>& discRF_;

};

////////////////////////////////////////////////////////////////////////////////
template <class A, class B, class DB> class DgAddResConverter :
    public DgConverter<A, long long int, DgResAdd<A>, long long int> {

   public:

      DgAddResConverter (const DgDiscRF<A, B, DB>& fromFrame,
                         const DgDiscRFS<A, B, DB>& toFrame, int resIn)
         : DgConverter<A, long long int, DgResAdd<A>,
                       long long int> (fromFrame, toFrame),
           res_ (resIn), discRFS_ (toFrame), discRF_ (fromFrame)
           {
              // JFW: Note that int res() > long size() can never occur:
              if (res() < 0 ||
                static_cast<unsigned long>(res()) >= discRFS().grids().size())
              {
                 report("DgDgAddResConverter<A, B, DB>::DgAddResConverter() "
                        "invalid resolution", DgBase::Fatal);
              }

              if (*(discRFS().grids()[res()]) != discRF())
              {
                 report("DgAddResConverter<A, B, DB>::DgAddResConverter() "
                        "grid mismatch", DgBase::Fatal);
              }
           }

      DgAddResConverter (const DgAddResConverter& con)
         : DgConverter<A, long long int, DgResAdd<A>, long long int> (con) { }

      int res (void) const { return res_; }

      const DgDiscRFS<A, B, DB>& discRFS (void) const { return discRFS_; }

      const DgDiscRF<A, B, DB>& discRF (void) const { return discRF_; }

      virtual DgResAdd<A> convertTypedAddress (const A& add) const
        {
           return DgResAdd<A>(add, res());
        }

   protected:

      int res_;
      const DgDiscRFS<A, B, DB>& discRFS_;
      const DgDiscRF<A, B, DB>& discRF_;

};

////////////////////////////////////////////////////////////////////////////////
template <class A, class B, class DB> class Dg2WayResAddConverter
                                               : public Dg2WayConverter {

   public:

      Dg2WayResAddConverter (const DgDiscRFS<A, B, DB>& fromFrame,
                             const DgDiscRF<A, B, DB>& toFrame, int res)
         : Dg2WayConverter
              (*(new DgResAddConverter<A, B, DB>(fromFrame, toFrame, res)),
               *(new DgAddResConverter<A, B, DB>(toFrame, fromFrame, res)))
           { }

};

// JFW: is this really what we mean?
#include "../lib/DgDiscRFS.hpp"

#endif
