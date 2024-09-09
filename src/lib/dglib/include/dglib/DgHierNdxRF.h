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
// DgHierNdxRF.h: DgHierNdxRF header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXRF_H
#define DGHIERNDXRF_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>
#include <dglib/DgHierNdxSystem.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
template <class T> class DgHierNdxRF : 
                       public DgDiscRF<T, DgQ2DICoord, long long int> 
{
   public:

      // this should be initialized in the instantiated class
      static const T undefCoord;

      // sub-classes should create a factory method
/*
      static DgHierNdxRF<T>* makeRF (const DgIDGGS& dggsIn, int resIn, 
                                     const string& nameIn)
         { 
           sys_ = new DgHierNdxSystem(dggsIn, resIn, nameIn + "Sys");
           return new DgHierNdxRF<T>(dggsIn, resIn, nameIn); 
         }

      static DgHierNdxRF<T>* makeRF (const DgHierNdxSystem& sysIn, const string& nameIn)
         { return new DgHierNdxRF<T>(sysIn, nameIn); }
*/

      const DgHierNdxSystem& system (void) { return *sys_; }

      const DgIDGGS& dggs (void) { return sys_->dggs(); }
      const DgIDGG& dgg (void) { return sys_->dgg(); }

      int res      (void) const { return sys_->res(); }
      int aperture (void) const { return sys_->aperture(); }

      // indexes don't typically use delimiters
      virtual string add2str (const T& add, char delimiter) const
                       { return add2str(add); }

      virtual const T& undefAddress (void) const { return undefCoord; }

      // these need to be defined by specializations
      // given dummy definitions for now so the class isn't abstract
      virtual T quantify (const DgQ2DICoord& point) const
                { return undefAddress(); }
      virtual DgQ2DICoord invQuantify (const T& add) const
                { return DgQ2DICoord::undefDgQ2DICoord; }
      virtual string add2str (const T& add) const { return dgg::util::to_string(add); }
      virtual const char* str2add (T* c, const char* str, char delimiter) const 
                      { return str; }

      // these should use the associated dgg
      virtual long long int dist (const T& add1, const T& add2) const
                       { return 0; }
      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }
      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }
      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }
      virtual void setAddNeighbors (const T& add, DgLocVector& vec) const { }
      virtual void setAddVertices  (const T& add, DgPolygon& vec) const { }

   protected:

      DgHierNdxRF<T> (const DgHierNdxSystem& sysIn, const string& nameIn)
         : DgDiscRF<T, DgQ2DICoord, long long int>(sysIn.dggs().network(), 
                       sysIn.dggs().idgg(resIn), nameIn, sysIn.dggs().gridTopo(), 
                       sysIn.dggs().gridMetric()),
           sys_ (&sysIn) { }

      void initSystem (const DgIDGGS& dggsIn, int resIn, const string& nameIn)
         { 
           sys_ = new DgHierNdxSystem(dggsIn, resIn, nameIn + "Sys");
         }

      const DgHierNdxSystem* sys_;
      bool ownSysMemory;
};           
                
// the actual value should be defined by the specializations
//template<typename T> const T DgHierNdxRF<T>::undefCoord;
                
////////////////////////////////////////////////////////////////////////////////
#endif   
