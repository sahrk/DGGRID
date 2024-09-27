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

#include <dglib/DgConverter.h>
#include <dglib/Dg2WayConverter.h>

class DgHierNdxCoordBase;
class DgZOrderStringCoord;
class DgIDGGBase;
class DgHierNdxCoordBase;

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
      virtual string valString (void) const = 0;
      virtual bool operator== (const DgHierNdxCoordBase& c) const = 0;
      virtual DgHierNdxCoordBase& operator= (const DgHierNdxCoordBase& add) = 0;
      virtual const DgHierNdxCoordBase& undefCoord (void) const = 0;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream&
operator<< (ostream& stream, const DgHierNdxCoordBase& coord)
{ return stream << string(coord); }

////////////////////////////////////////////////////////////////////////////////
class DgHierNdxRFBase : public DgDiscRF<DgHierNdxCoordBase, long long int> {

   //using DgDiscRF<DgQ2DICoord, DgGeoCoord, long double>::setVertices;

   public:

      static DgHierNdxRFBase* makeRF (const DgIDGGBase& dggIn, const string& nameIn,
                                      int resIn, int apertureIn)
         { return new DgHierNdxRFBase (dggIn, nameIn, resIn, apertureIn); }

      virtual ~DgHierNdxRFBase (void);

      const DgIDGGBase& dgg (void) { return dgg_; }

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
      virtual const DgHierNdxCoordBase& undefCoord (void) const = 0;

   protected:

      DgHierNdxRFBase (const DgIDGGBase& dggIn, const string& nameIn,
                       int resIn, int apertureIn)
         : DgDiscRF<DgHierNdxCoordBase, long long int>(dggIn.network(), nameIn),
           dgg_ (dggIn), res_ (resIn), aperture_ (apertureIn) { }

      const DgIDGGBase& dgg_;
      int res_;
      int aperture_;

};

////////////////////////////////////////////////////////////////////////////////
#endif
