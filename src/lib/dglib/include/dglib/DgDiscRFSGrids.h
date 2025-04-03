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
// DgDiscRFSGrids.h: DgDiscRFSGrids class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDISCRFSGRIDS_H
#define DGDISCRFSGRIDS_H

#include <vector>

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
//#include <dglib/DgDiscRF.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgRF.h>

//class DgDiscTopoRFS;

////////////////////////////////////////////////////////////////////////////////
template<class A> class DgResAdd {

   public:

      DgResAdd (void) : res_ (0) { }

      DgResAdd (const DgResAdd& add) { *this = add; }

      DgResAdd (const A& add, int res = 0)
         : address_ (add), res_ (res) { }

      const A& address (void) const { return address_; }

      void setAddress (const A& add) { address_ = add; }

      int res (void) const { return res_; }

      void setRes (int res) { res_ = res; }

      operator string (void) const
                { return  string("[") + dgg::util::to_string((int) res()) + ", " +
                          string(address()) + string("]"); }

      DgResAdd<A>& operator= (const DgResAdd<A>& add)
                {
                   if (add != *this)
                   {
                      setRes(add.res());
                      setAddress(add.address());
                   }

                   return *this;
                }

      bool operator== (const DgResAdd<A>& add) const
             { return res() == add.res() && address() == add.address(); }

      bool operator!= (const DgResAdd<A>& add) const
             { return !operator==(add); }

/*
      // remind users of the pure virtual functions remaining from above

       virtual const DgResAdd<A>& undefAddress (void) const = 0;
*/

   private:

      A address_{};
      int res_ = -1;
};

////////////////////////////////////////////////////////////////////////////////
template<class A> ostream& operator<< (ostream& stream, const DgResAdd<A>& add)
{
   stream << string(add);
   return stream;

} // ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
/** @class DgDiscRFSGrids
 *  @brief A multi-resolution array of pointers to DgDiscRF's
 *
 *   Has template parameters:
 *
 *   DRF - class of each resolution in the grids_ array; should have A addresses
 *   A - the DiscRF type to be wrapped in DgResAdd
 *   B - background RF type for DgResAdd<A>
 *   DB - distance type of background RF
 *   BG -  background RF for each resolution grid 
 *   
 *   each resolution in the grids_ array is a DRF<A, BG, DB>*
 */

template<template <class, class, class> class DRF, class A, class B, class DB, class BG> class DgDiscRFSGrids {

   public:

      virtual ~DgDiscRFSGrids (void) { delete grids_; }
/*
      virtual string add2str (const DgResAdd<A>& add) const
                                              { return string(add); }

      virtual string add2str (const DgResAdd<A>& add, char delimiter) const
                  { return dgg::util::to_string(add.res()) + delimiter +
                        grids()[add.res()]->add2str(add.address(), delimiter); }

      virtual const char* str2add (DgResAdd<A>* add, const char* str,
                                   char delimiter) const;
 */

      DgDiscRFSGrids<DRF, A, B, DB, BG>& operator=
                      (const DgDiscRFSGrids<DRF, A, B, DB, BG>& rf)
      // shallow copy with possible memory leak; don't use if avoidable
          {
             if (*this != rf)
             {
                //DRF<DgResAdd<A>, B, DB>::operator=(rf);

                nRes_ = rf.nRes();
                delete grids_; // the DgRFNetwork will delete the grids themselves

                grids_ = new vector<const DRF<A, BG, DB>*>(rf.nRes());
                for (int i = 0; i < nRes(); i++)
                {
                   // KLUDGE: don't know real type of each grid so can't
                   // easily create them here; opt for shallow copy

                   (*grids_)[i] = rf.grids()[i];
                }
            }

            return *this;
         }

      const vector<const DRF<A, BG, DB>*>& grids (void) const { return *grids_; }

      int nRes (void) const { return nRes_; }

      // no bounds checking

      const DRF<A, BG, DB>& operator[] (int res) const
                           { return *((*grids_)[res]); }

      virtual operator string (void) const
      {
         string s = "*** DgDiscRFSGrids\nnRes: " + dgg::util::to_string(nRes()) + "\n";
         for (int i = 0; i < nRes(); i++)
            s += " >>> " + dgg::util::to_string(i) + ": " +
                   string(*(*grids_)[i]) + "\n";

         return s;
      }

   protected:

      DgDiscRFSGrids (const DgRF<B, DB>& backFrameIn, int nResIn)
         : backFrameLocal_ (backFrameIn), grids_ (nullptr), nRes_ (nResIn)
        {
          this->grids_ = new vector<const DRF<A, BG, DB>*>(nRes(), nullptr);
          if (nRes() < 0) {
             report("DgDiscRFSGrids<DRF, A, B, DB, BG>::DgDiscRF() nRes < 0",
                    DgBase::Fatal);
          }
        }

      DgDiscRFSGrids (const DgDiscRFSGrids<DRF, A, B, DB, BG>& rf) // uses dubious operator=
        //: DgDiscRF<DgResAdd<A>, B, DB> (rf)
        { *this = rf; }

      vector<const DRF<A, BG, DB>*>& gridsMutable (void) const { return *grids_; }

      // hokey temporary notion of distance
      virtual long long int distRFS (const DgResAdd<A>& add1,
                      const DgResAdd<A>& add2) const
            { return abs(add2.res() - add1.res()); }
    /*
      virtual DgResAdd<A> quantifyRFS (const B& point) const
            {
               // quantify using max res grid
               int maxRes = nRes() - 1;
               DgLocation* loc = backFrameLocal_.makeLocation(point);
               const DRF<A, B, DB>& grid = *grids()[maxRes];
               grid.convert(loc);
               DgResAdd<A> add(*grid.getAddress(*loc), maxRes);

               delete loc;

               return add;
             }

      virtual B invQuantifyRFS (const DgResAdd<A>& add) const
             {
               const DRF<A, B, DB>& grid = *grids()[add.res()];
               DgLocation* loc = grid.makeLocation(add.address());
               backFrameLocal_.convert(loc);
               B newAdd(*(backFrameLocal_.getAddress(*loc)));
               delete loc;
               return newAdd;
             }
     */

      // state data
      //using ConstDRFPtr = const typename DRF<A, B, DB>*;
      //std::vector<ConstDRFPtr>* grids_;
      const DgRF<B, DB>& backFrameLocal_;
      vector<const DRF<A, BG, DB>*>* grids_;
      int nRes_;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
template <template <class, class, class> class DRF, class A, class B, class DB, class BG> class DgResAddConverter :
    public DgConverter<DgResAdd<A>, long long int, A, long long int> {

   public:

      DgResAddConverter (const DgDiscRFSGrids<DRF, A, B, DB, BG>& fromFrame,
                         const DgDiscRF<A, BG, DB>& toFrame, int resIn)
         : DgConverter<DgResAdd<A>, long long int, A,
                 long long int> (fromFrame, toFrame),
           res_ (resIn), discRFS_ (fromFrame), discRF_ (toFrame)
           {
	      // JFW: second clause will never be > (int vs long):
              if (res() < 0 ||
                  static_cast<unsigned long>(res()) >= discRFS().grids().size())
              {
                 report("DgResAddConverter<A, B, DB, BG>::DgResAddConverter() "
                        "invalid resolution", DgBase::Fatal);
              }

              if (*(discRFS().grids()[res()]) != discRF())
              {
                 report("DgDgResAddConverter<A, B, DB, BG>::DgResAddConverter() "
                        "grid mismatch", DgBase::Fatal);
              }
           }

      DgResAddConverter (const DgResAddConverter& con)
         : DgConverter<DgResAdd<A>, long long int, A, long long int> (con) { }

      int res (void) const { return res_; }

      const DgDiscRFSGrids<DRF, A, B, DB, BG>& discRFS (void) const { return discRFS_; }

      const DRF<A, B, DB>& discRF (void) const { return discRF_; }

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
      const DgDiscRFSGrids<DRF, A, B, DB, BG>& discRFS_;
      const DRF<A, BG, DB>& discRF_;

};

////////////////////////////////////////////////////////////////////////////////
template <template <class, class, class> class DRF, class A, class B, class DB, class BG> class DgAddResConverter :
    public DgConverter<A, long long int, DgResAdd<A>, long long int> {

   public:

      DgAddResConverter (const DgDiscRF<A, BG, DB>& fromFrame,
                         const DgDiscRFSGrids<DRF, A, B, DB, BG>& toFrame, int resIn)
         : DgConverter<A, long long int, DgResAdd<A>,
                       long long int> (fromFrame, toFrame),
           res_ (resIn), discRFS_ (toFrame), discRF_ (fromFrame)
           {
              // JFW: Note that int res() > long size() can never occur:
              if (res() < 0 ||
                static_cast<unsigned long>(res()) >= discRFS().grids().size())
              {
                 report("DgDgAddResConverter<A, B, DB, BG>::DgAddResConverter() "
                        "invalid resolution", DgBase::Fatal);
              }

              if (*(discRFS().grids()[res()]) != discRF())
              {
                 report("DgAddResConverter<A, B, DB, BG>::DgAddResConverter() "
                        "grid mismatch", DgBase::Fatal);
              }
           }

      DgAddResConverter (const DgAddResConverter& con)
         : DgConverter<A, long long int, DgResAdd<A>, long long int> (con) { }

      int res (void) const { return res_; }

      const DgDiscRFSGrids<DRF, A, B, DB, BG>& discRFS (void) const { return discRFS_; }

      const DRF<A, BG, DB>& discRF (void) const { return discRF_; }

      virtual DgResAdd<A> convertTypedAddress (const A& add) const
        {
           return DgResAdd<A>(add, res());
        }

   protected:

      int res_;
      const DgDiscRFSGrids<DRF, A, B, DB, BG>& discRFS_;
      const DRF<A, BG, DB>& discRF_;

};


// JFW: is this really what we mean?
//#include "../lib/DgDiscRFSGrids.hpp"

#endif
