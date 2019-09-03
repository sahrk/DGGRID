/*******************************************************************************
    Copyright (C) 2018 Kevin Sahr

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
// DgLabelLoc.h: DgLabelLoc class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGLABELLOC_H
#define DGLABELLOC_H

#include <string>
#include <iostream>

using namespace std;

#include "DgRFBase.h"

////////////////////////////////////////////////////////////////////////////////
class DgLabelLoc {

   public:

      DgLabel (DgLocBase& locIn, string labelIn = "")
         : location_ (locIn), label_ (labelIn) { }

      const string& label (void) const { return label_; }
      const DgLocBase& location (void) const { return location_; }

      void setLabel (const string& labelIn) { label_ = labelIn; }
      void setLocation (DgLocBase& locIn) { label_ = &locIn; }


};

////////////////////////////////////////////////////////////////////////////////
inline ostream& operator<< (ostream& stream, const DgLabel& label)
            { return stream << label.label(); }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgLabelLocBase : public DgLocBase, public DgLabel {

   public:

      

};

////////////////////////////////////////////////////////////////////////////////
#endif
