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

#ifndef DGAUTHALICCONVERTER_H
#define DGAUTHALICCONVERTER_H

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgWGS84RF.h>

// WGS 84 geodetic latitude phi and authalic latitude beta, both in radians.
// Longitude is unaffected. The functions also serve geographic metafile
// values that must be converted before a frame graph is constructed.
class DgAuthalic {
   public:
      static long double geodeticToAuthalicLatitude(long double phi);
      static long double authalicToGeodeticLatitude(long double beta);
      static long double authalicRadiusKM();
};

class DgGeodeticToAuthalicConverter :
   public DgConverter<DgGeoCoord, long double, DgGeoCoord, long double> {
   public:
      DgGeodeticToAuthalicConverter(const DgWGS84RF& from,
                                   const DgGeoSphRF& to)
         : DgConverter<DgGeoCoord, long double, DgGeoCoord, long double>
              (from, to) { }

      virtual DgGeoCoord convertTypedAddress(const DgGeoCoord& address) const;
};

class DgAuthalicToGeodeticConverter :
   public DgConverter<DgGeoCoord, long double, DgGeoCoord, long double> {
   public:
      DgAuthalicToGeodeticConverter(const DgGeoSphRF& from,
                                   const DgWGS84RF& to)
         : DgConverter<DgGeoCoord, long double, DgGeoCoord, long double>
              (from, to) { }

      virtual DgGeoCoord convertTypedAddress(const DgGeoCoord& address) const;
};

// Construct after both frames exist, before any composed paths are cached.
class Dg2WayAuthalicConverter : public Dg2WayConverter {
   public:
      Dg2WayAuthalicConverter(const DgWGS84RF& ellipsoid,
                             const DgGeoSphRF& sphere);
};

#endif
