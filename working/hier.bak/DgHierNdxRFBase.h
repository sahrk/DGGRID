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
// DgHierNdxRFBase.h: DgHierNdxRFBase header file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHIERNDXRFBASE_H
#define DGHIERNDXRFBASE_H

#include <climits>
#include <iostream>

#include <dglib/DgDiscTopoRF.h>
#include <dglib/DgIDGGS.h>
#include <dglib/DgIDGG.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
class DgHierNdxCoordBase  {

   public:

      DgHierNdxCoordBase (void) { }

      virtual ~DgHierNdxCoordBase (void);

      operator string (void) const { return valString(); }

      bool operator!= (const DgHierNdxCoordBase& c) const
          { return !(*this == c); }

      // to be defined by sub-classes
      // we give them dummy definitions here so the class isn't abstract
      virtual string valString (void) const { return ""; }
      virtual bool operator== (const DgHierNdxCoordBase& c) const { return false; }
      virtual DgHierNdxCoordBase& operator= (const DgHierNdxCoordBase& add)
                { return *this; }
      virtual const DgHierNdxCoordBase& undefCoord (void) const { return *this; }
};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgHierNdxCoordBase& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxRFBase : public DgDiscTopoRF<DgHierNdxCoordBase, DgQ2DICoord, long long int> {

   public:

      virtual ~DgHierNdxRFBase (void);

      const DgIDGGS& dggs (void) { return dggs_; }
      const DgIDGG& dgg (void) { return dgg_; }

      int res      (void) const { return res_; }
      int aperture (void) const { return aperture_; }

      virtual long long int dist (const DgHierNdxCoordBase& add1,
                        const DgHierNdxCoordBase& add2) const
                       { return 0; }

      virtual string add2str (const DgHierNdxCoordBase& add) const
                       { return string(add); }

      virtual string add2str (const DgHierNdxCoordBase& add, char delimiter)
                                                                         const
                       { return string(add); }

      virtual string dist2str (const long long int& dist) const
                       { return dgg::util::to_string(dist); }

      virtual long double dist2dbl (const long long int& dist) const
                       { return dist; }

      virtual unsigned long long int dist2int (const long long int& dist) const
                       { return dist; }

      // methods for sub-classes to define
      virtual const char* str2add (DgHierNdxCoordBase* add, const char* str,
                                   char delimiter) const = 0;

      // from DgDiscRF
      //virtual const DgHierNdxCoordBase& undefAddress (void) const = 0;

      // these are given dummy definitions for now, but should be moved down
      virtual DgHierNdxCoordBase quantify (const DgQ2DICoord& point) const
                { return undefAddress(); }
      virtual DgQ2DICoord invQuantify (const DgHierNdxCoordBase& add) const
                { return DgQ2DICoord::undefDgQ2DICoord; }

      // these should use the associated dgg
      virtual void setAddNeighbors (const DgHierNdxCoordBase& add, DgLocVector& vec) const { }
      virtual void setAddVertices  (const DgHierNdxCoordBase& add, DgPolygon& vec) const { }

   protected:

      DgHierNdxRFBase (const DgIDGGS& dggsIn, int resIn, const string& nameIn)
         : DgDiscTopoRF<DgHierNdxCoordBase, DgQ2DICoord, long long int>(dggsIn.network(), 
                       dggsIn.idgg(resIn), nameIn, dggsIn.gridTopo(), dggsIn.gridMetric()),
           dggs_ (dggsIn), dgg_ (dggsIn.idgg(resIn)), res_ (resIn), aperture_ (dggsIn.aperture()) { }

      const DgIDGGS& dggs_;
      const DgIDGG& dgg_;
      int res_;
      int aperture_;

};

////////////////////////////////////////////////////////////////////////////////
#endif