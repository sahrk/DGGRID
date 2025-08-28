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
// DgLocList.h: DgLocList class definitions
//
////////////////////////////////////////////////////////////////////////////////
//
//  The DgLocList class defines a set of locations which can be treated as a
//  group for purposes of conversion, etc.
//
//  Note: add only loc's of the correct RF or use push_back. Other STL list
//    methods should be checked/corrected for RF-safeness.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGLOCLIST_H
#define DGLOCLIST_H

#include <dglib/DgLocBase.h>

#include <functional>
#include <list>

////////////////////////////////////////////////////////////////////////////////
class DgLocList : public DgLocBase, public std::list<DgLocBase*> {

   public:

      DgLocList (void) : isOwner_ (true) { }

      DgLocList (const DgRFBase& rfIn) : DgLocBase(rfIn), isOwner_ (true) { }

     ~DgLocList (void);

      void destroy (void);

      bool isOwner (void) const { return isOwner_; }

      void setIsOwner (bool isOwnerIn) { isOwner_ = isOwnerIn; }

      virtual void clearAddress (void);

      //virtual bool thin (float dMin = 0.0, bool clear = true);

      virtual int cardinality (void) const;

      DgLocList& operator= (const DgLocList& list); // does not make copy

      //bool operator== (const DgLocList& list) const;

      //bool operator!= (const DgLocList& list) const
      //     { return !operator==(list); }

      virtual std::string asString (void) const;
      virtual std::string asAddressString (void) const;

      virtual std::string asString (char delimiter) const;
      virtual std::string asAddressString (char delimiter) const;

      virtual const char* fromString (const char* str, char delimiter);

      void push_back (DgLocBase* loc);

   protected:

      virtual void convertTo (const DgRFBase& rf);

   private:

      bool isOwner_;

};

////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& stream, const DgLocList& list);

////////////////////////////////////////////////////////////////////////////////

#endif
