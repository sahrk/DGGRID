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

#ifndef DGWGS84RF_H
#define DGWGS84RF_H

#include <dglib/DgConstants.h>
#include <dglib/DgEllipsoidRF.h>

// WGS 84 geographic coordinates: longitude and geodetic latitude in radians.
// The grid is generated on the authalic sphere. Area and resolution
// statistics stay on that sphere. Distance measured in this frame is the
// WGS 84 geodesic between geodetic coordinates. Cell area measured in this
// frame is the geodesic area of the cell.
class DgWGS84RF : public DgEllipsoidRF {
   public:
      static const DgWGS84RF* makeRF(DgRFNetwork& network,
                                     const std::string& name = "WGS84")
         { return new DgWGS84RF(network, name); }

      static constexpr long double semiMajorAxisMeters() { return 6378137.0L; }
      static constexpr long double inverseFlattening() { return 298.257223563L; }
      static constexpr long double semiMinorAxisMeters()
         { return semiMajorAxisMeters() *
                  (1.0L - 1.0L / inverseFlattening()); }

      // Projection-sphere radius for grid geometry. This is the full-precision
      // WGS 84 authalic radius. DgAuthalic::authalicRadiusKM() evaluates the
      // same quantity from the defining axes for conversion checks.
      static constexpr long double canonicalAuthalicRadiusKM()
         { return WGS84_AUTHALIC_RADIUS_KM; }

      // Geodesic fan on WGS 84. center and boundary are converted into this
      // frame first, which changes authalic latitude to geodetic latitude
      // when they come from the WGS 84 authalic sphere.
      virtual long double cellAreaKM (const DgLocation& center,
                                       const DgPolygon& boundary) const override;

   private:
      DgWGS84RF(DgRFNetwork& network, const std::string& name)
         : DgEllipsoidRF(network, name, semiMajorAxisMeters(),
                         semiMinorAxisMeters()) { }

      long double triangleAreaM2 (const DgGeoCoord& p, const DgGeoCoord& q,
                                  const DgGeoCoord& r, int depth) const;
};

#endif
