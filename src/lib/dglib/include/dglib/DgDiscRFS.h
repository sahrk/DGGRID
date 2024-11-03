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

#include <dglib/DgDiscRF.h>
#include <dglib/DgPolygon.h>
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
               
      // abstract functions from DgDiscRF
      // these functions are the same as in DgDiscRFS

      virtual string add2str (const DgResAdd<A>& add) const
                         { return string(add); }

      virtual string add2str (const DgResAdd<A>& add, char delimiter) const
                         { return dgg::util::to_string(add.res()) + delimiter +
                           this->grids()[add.res()]->add2str(add.address(), delimiter); }

      virtual const char* str2add (DgResAdd<A>* add, const char* str, char delimiter) const;
               
      //vector<const DgDiscRF<A, B, DB>*>& gridsMutable (void) const { return *grids_; }

      protected:
               
         virtual long long int dist (const DgResAdd<A>& add1,
                                        const DgResAdd<A>& add2) const
                              { return this->distRFS(add1, add2); }

         virtual DgResAdd<A> quantify (const B& point) const
            {
                // quantify using max res grid
                return this->quantifyRFS(point);
             }

         virtual B invQuantify (const DgResAdd<A>& add) const
             {
                 return this->invQuantifyRFS(add);
             }
/*
      // state data

      const DgRF<B, DB>* backFrame_;
      vector<const DgDiscRF<A, B, DB>*>* grids_;
      int nRes_;
*/

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// JFW: is this really what we mean?
#include "../lib/DgDiscRFS.hpp"

#endif
