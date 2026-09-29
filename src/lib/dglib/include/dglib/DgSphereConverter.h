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

#ifndef DGSPHERECONVERTER_H
#define DGSPHERECONVERTER_H

#include <dglib/Dg2WayConverter.h>
#include <dglib/DgConverter.h>
#include <dglib/DgGeoSphRF.h>

// A point has the same longitude and latitude on spheres of any radius.
class DgSphereToSphereConverter :
   public DgConverter<DgGeoCoord, long double, DgGeoCoord, long double> {
   public:
      DgSphereToSphereConverter(const DgGeoSphRF& from, const DgGeoSphRF& to)
         : DgConverter<DgGeoCoord, long double, DgGeoCoord, long double>
              (from, to) { }

      virtual DgGeoCoord convertTypedAddress(const DgGeoCoord& address) const
         { return address; }
};

// Construct after both frames exist, before any composed paths are cached.
class Dg2WaySphereConverter : public Dg2WayConverter {
   public:
      Dg2WaySphereConverter(const DgGeoSphRF& sphere1, const DgGeoSphRF& sphere2)
         : Dg2WayConverter(*(new DgSphereToSphereConverter(sphere1, sphere2)),
                           *(new DgSphereToSphereConverter(sphere2, sphere1)))
         { }
};

#endif
