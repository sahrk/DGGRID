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
// Grid geometry and distance statistics continue to use the projection sphere.
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

      // Preserve the historical projection-sphere radius exactly. The
      // axes-derived value differs by about one nanometer; use that derived
      // value for mathematical checks, not for changing legacy grid geometry.
      static constexpr long double canonicalAuthalicRadiusKM()
         { return WGS84_AUTHALIC_RADIUS_KM; }

      // No ellipsoidal geodesic is supplied here. Fail if distance is requested
      // rather than returning DgEllipsoidRF's placeholder value.
      virtual long double dist(const DgGeoCoord&, const DgGeoCoord&) const;

   private:
      DgWGS84RF(DgRFNetwork& network, const std::string& name)
         : DgEllipsoidRF(network, name, semiMajorAxisMeters(),
                         semiMinorAxisMeters()) { }
};

#endif
