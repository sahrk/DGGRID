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
// DgGeodesic.h: DgGeodesic class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGGEODESIC_H
#define DGGEODESIC_H

#include <vector>

////////////////////////////////////////////////////////////////////////////////
//
// Geodesics on an oblate ellipsoid of revolution (a sphere included). Angles
// are radians, lengths are meters, latitudes are geodetic and azimuths are
// measured clockwise from north.
//
// A geodesic is described on the auxiliary sphere (Bessel 1825, Helmert 1880;
// notation of Karney, J. Geodesy 87, 2013): a point of reduced latitude beta
// lies at arc sigma from the point where the geodesic crosses the equator
// northward with azimuth alpha0, and
//
//    sin(beta) = cos(alpha0) sin(sigma)
//    ds/dsigma = b sqrt(1 + k^2 sin^2(sigma)),   k^2 = e'^2 cos^2(alpha0)
//    dlon/dsigma = sin(alpha0) (1 / cos^2(beta)
//                     - f (2 - f) / (1 + (1 - f) sqrt(1 + k^2 sin^2(sigma))))
//
// The distance and longitude integrals are evaluated by Gauss-Legendre
// quadrature rather than by a truncated series, so the results are accurate
// to rounding for any flattening.
//
// The inverse problem is solved for the azimuth at the first point by a
// bracketed Newton iteration. With the points arranged so that
// beta1 <= 0 and |beta2| <= |beta1|, the longitude difference reached at the
// parallel of point 2 matches the given one at exactly one azimuth in
// [0, pi], that of the shortest geodesic, and is smaller below it and larger
// above it. The bracket therefore always holds the solution, and the
// iteration converges for every pair of points, including nearly antipodal
// ones.
//
class DgGeodesic {

   public:

      // a is the equatorial radius and b the polar radius, a >= b > 0
      DgGeodesic (long double a, long double b);

      // false if the axes do not describe an oblate ellipsoid or a sphere
      bool valid (void) const { return valid_; }

      // A geodesic through a point (point 1)
      struct Line {
         long double sinAlpha0, cosAlpha0; // azimuth at the equator crossing
         long double k2;                   // e'^2 cos^2(alpha0)
         long double sinSigma1, cosSigma1; // arc from that crossing to point 1
         long double sigma1;
      };

      // The arc of a geodesic from point 1 to point 2
      struct Arc {
         Line line;
         long double sigma12;                    // arc on the auxiliary sphere
         long double s12;                        // length in meters
         long double sinAzimuth1, cosAzimuth1;   // forward azimuth at point 1
         long double sinAzimuth2, cosAzimuth2;   // forward azimuth at point 2
         int iterations;   // steps the inverse solve took (0: closed form)
      };

      // The geodesic through latitude lat1 with the given azimuth
      Line line (long double lat1, long double sinAzimuth1,
                 long double cosAzimuth1) const;

      // Shortest geodesic from point 1 to point 2. At a pole the azimuth is
      // relative to the meridian of the longitude given for that point.
      // Returns false for coordinates that are not finite.
      bool inverse (long double lat1, long double lon1,
                    long double lat2, long double lon2, Arc& arc) const;

      // Point 2 reached by travelling s12 (which may be negative) from
      // point 1 along azimuth1; azimuth2 is the forward azimuth there.
      // lon2 is in [-pi, pi].
      void direct (long double lat1, long double lon1, long double azimuth1,
                   long double s12, long double& lat2, long double& lon2,
                   long double& azimuth2) const;

      // Area in m^2 of the polygon whose edges are the shortest geodesics
      // between consecutive vertices (the last joins the first). The result
      // is not negative and does not depend on the direction of the ring.
      //
      // The area is the line integral of (q0 - q(lat)) dlon around the ring,
      // q being the authalic latitude function, evaluated along each
      // geodesic by quadrature. q0 is chosen so that the integrand is of the
      // size of the polygon, which keeps full relative accuracy for
      // polygons of any size, including ones that contain a pole.
      //
      // centerLat and centerLon give a point in or near the polygon; it only
      // selects q0. The polygon must be smaller than a hemisphere and its
      // boundary must stay away from at least one pole. Returns false if it
      // does not, if there are fewer than 3 vertices or if a coordinate is
      // not finite.
      bool polygonArea (const std::vector<long double>& lat,
                        const std::vector<long double>& lon,
                        long double centerLat, long double centerLon,
                        long double& areaM2) const;

      long double a (void) const { return a_; }
      long double b (void) const { return b_; }
      long double f (void) const { return f_; }

      // surface area of the whole ellipsoid in m^2
      long double totalArea (void) const;

   private:

      // values of the inverse problem's longitude function for one azimuth
      struct Lambda {
         long double lam12, dlam12;     // longitude difference and derivative
         long double sinAlpha1, cosAlpha1, sinAlpha2, cosAlpha2;
         long double sinAlpha0, cosAlpha0, k2;
         long double sinSigma1, cosSigma1, sigma1, sigma12;
      };

      long double reduced (long double lat, long double& sinBeta,
                           long double& cosBeta) const;

      void integrals (long double k2, long double sigma, long double dSigma,
                      long double* E, long double* D3, long double* J) const;

      void panelSums (long double k2, long double sigma, long double dSigma,
                      long double& E, long double& D3, long double& J) const;

      void lambda12 (long double sinBeta1, long double cosBeta1,
                     long double sinBeta2, long double cosBeta2,
                     long double sinAlpha1, long double cosAlpha1,
                     Lambda& out) const;

      long double q (long double sinLat) const;

      long double a_, b_, f_, f1_, e2_, e_, ep2_;
      bool valid_;
};

#endif
