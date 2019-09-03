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
// DgProjGnomonicRF.cpp: DgProjGnomonicRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "DgProjGnomonicRF.h"
#include "DgEllipsoidRF.h"
#include "DgGeoSphRF.h"

////////////////////////////////////////////////////////////////////////////////
DgProjGnomonicRF::DgProjGnomonicRF(DgRFNetwork& networkIn, const string& nameIn,
       const DgGeoCoord& proj0In, long double localUpAzimuthIn)
   : DgGeoProjRF (networkIn, nameIn, proj0In), localUpAzimuth_ (localUpAzimuthIn)
{ 
} // DgProjGnomonicRF::DgProjGnomonicRF

////////////////////////////////////////////////////////////////////////////////
DgDVec2D
DgProjGnomonicRF::projForward (const DgGeoCoord& addIn,
                               const DgEllipsoidRF& e) const
{
   DgDVec2D xy(DgDVec2D::undefDgDVec2D);

   // determine distance on the sphere from the projection center point
   double r = DgGeoCoord::gcDist(proj0(), addIn);

   if (r < M_EPSILON) {
       xy.setX(0.0L);
       xy.setY(0.0L);
       return xy;
   }

   // find CCW theta from local "up" azimuth
   double theta = localUpAzimuth() - DgGeoSphRF::azimuth(proj0(), addIn);
   if (theta < 0.0L) theta += M_2PI;
   if (theta >= M_2PI) theta -= M_2PI;

   // gnomonic projection scaling of r
   r = tan(r);
   // scale more for quad?

   // convert to cartesian coordinates
   xy.setX(r * cos(theta));
   xy.setY(r * sin(theta));

   return xy;

} // DgDVec2D DgProjGnomonicRF::projForward

////////////////////////////////////////////////////////////////////////////////
DgGeoCoord
DgProjGnomonicRF::projInverse (const DgDVec2D& addIn, 
                               const DgEllipsoidRF& e) const
{
   // calculate planar polar coordinates (r, theta)
   double r = addIn.magnitude();
   if (r < M_EPSILON) return proj0();

   double theta = atan2(addIn.y(), addIn.x());

   // perform inverse gnomonic scaling of r
   r = atan(r);

   // find theta as an azimuth
   theta = localUpAzimuth() - theta;
   if (theta < 0.0L) theta += M_2PI;
   if (theta >= M_2PI) theta -= M_2PI;

   // point at (r,theta) from the projection center
   DgGeoCoord geo = DgGeoSphRF::travelGC(proj0(), r, theta);

   return geo;

} // DgGeoCoord DgProjGnomonicRF::projInverse

////////////////////////////////////////////////////////////////////////////////
