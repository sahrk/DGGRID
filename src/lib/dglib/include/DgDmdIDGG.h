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
// DgDmdIDGG.h: DgDmdIDGG class definitions
//
// Kevin Sahr, 8/11/20
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGDMDIDGG_H 
#define DGDMDIDGG_H

#include "DgIDGGBase.h"

class DgIDGGS4D;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  Icosahedral DGG class.
//
class DgDmdIDGG : public DgIDGGBase {

   public:

      DgDmdIDGG (const DgIDGGS4D& dggs, unsigned int aperture = 4,
              int res = 0, const string& name = "DmdIDGG", 
              bool isD4 = true, unsigned int precision = DEFAULT_PRECISION);

      DgDmdIDGG (const DgDmdIDGG& grd);

      virtual ~DgDmdIDGG (void);

      const DgIDGGS4D& dmdDggs (void) const;

      long double scaleFac (void) const { return scaleFac_; }

      bool isD4 (void) const { return isD4_; }
 
   protected:

      void initialize (void);

   private:

      long double scaleFac_;
      bool isD4_;
};

////////////////////////////////////////////////////////////////////////////////
#endif
