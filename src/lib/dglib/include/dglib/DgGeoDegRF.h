/*******************************************************************************
    Copyright (C) 2026 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef DGGEODEGRF_H
#define DGGEODEGRF_H

#include <dglib/DgContCartRF.h>
#include <dglib/DgEllipsoidRF.h>

// Longitude and latitude in degrees, attached to a geographic frame whose
// DgGeoCoord addresses are in radians. This adapter changes units only.
class DgGeoDegRF : public DgContCartRF {
   public:
      static const DgGeoDegRF* makeRF(const DgEllipsoidRF& geoRF,
                                      const std::string& name = "GeodeticDeg")
         { return new DgGeoDegRF(geoRF, name); }

      const DgEllipsoidRF& geoRF(void) const { return geoRF_; }

   protected:
      DgGeoDegRF(const DgEllipsoidRF& geoRF, const std::string& name);

   private:
      const DgEllipsoidRF& geoRF_;
};

#endif
