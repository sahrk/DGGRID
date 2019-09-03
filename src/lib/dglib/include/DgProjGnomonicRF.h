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
// DgProjGnomonicRF.h: DgProjGnomonicRF class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGPROJGNOMONICRF_H
#define DGPROJGNOMONICRF_H

#include <iostream>

#include "DgConstants.h"
#include "DgGeoProjRF.h"
#include "DgDVec2D.h"

using namespace std;

class DgEllipsoidRF;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgProjGnomonicRF : public DgGeoProjRF {

   public:

      DgProjGnomonicRF (DgRFNetwork& networkIn, 
                      const string& nameIn = "GnomonicRF",
                      const DgGeoCoord& proj0In = DgGeoCoord(M_ZERO, M_ZERO),
                      long double localUpAzimuthIn = 0.0L);

      long double localUpAzimuth (void) const { return localUpAzimuth_; }

      // pure virtual methods

      virtual DgDVec2D   projForward (const DgGeoCoord& addIn,
                                      const DgEllipsoidRF& e) const;

      virtual DgGeoCoord projInverse (const DgDVec2D& addIn,
                                      const DgEllipsoidRF& e) const;

   protected:

      long double sinph0_;
      long double cosph0_;
      int mode_;

      long double localUpAzimuth_;

}; // class DgProjGnomonicRF

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
