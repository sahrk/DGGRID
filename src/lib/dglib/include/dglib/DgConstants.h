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
// DgConstants.h: DgConstants class definitions
//
//   Constants were calculated to arbitrary precision using Wcalc.
//
// Version 7.0 - Kevin Sahr, 12/14/14
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGCONSTANTS_H
#define DGCONSTANTS_H

/* precise 12 digits after dec pt */
constexpr long double M_EPSILON =   0.00000000000050000000000000000000000000000L;
constexpr long double M_ZERO =      0.00000000000000000000000000000000000000000L;
constexpr long double M_ONE =       1.00000000000000000000000000000000000000000L;
constexpr long double M_HALF =      0.50000000000000000000000000000000000000000L;
// pi, pi/2 (long double versions of the double-precision M_PI, M_PI_2 macros)
constexpr long double M_PI_L =      3.14159265358979323846264338327950288419716940L;
constexpr long double M_PI_2_L =    1.57079632679489661923132169163975144209858470L;
constexpr long double M_2PI =       6.28318530717958647692528676655900576839433L;
constexpr long double M_SQRT3 =     1.7320508075688772935274463415058723669428L;
constexpr long double M_1_SQRT3 =   0.5773502691896257645091487805019574556476L;
constexpr long double M_SQRT3_2 =   0.8660254037844386467637231707529361834714L;
constexpr long double M_SQRT5 =     2.2360679774997896964091736687312762354406L;
constexpr long double M_SQRT7 =     2.6457513110645905905016157536392604257102L;
constexpr long double M_1_SQRT7 =   0.3779644730092272272145165362341800608157L;
constexpr long double M_SQRT7_2 =   1.3228756555322952952508078768196302128551L;
constexpr long double M_SQRT8 =     2.8284271247461900976033774484193961571393L;
constexpr long double M_SQRT10 =    3.16227766016837933199889354443271853371955L;
constexpr long double M_SQRT15 =    3.8729833462074168851792653997823996108329L;
constexpr long double M_PHI =       1.618033988749894848204586834365638117720309L;
constexpr long double M_SIN60 =     M_SQRT3_2;
constexpr long double M_COS60 =     0.50000000000000000000000000000000000000000L;
constexpr long double M_SIN30 =     M_COS60;
constexpr long double M_COS30 =     M_SIN60;
constexpr long double M_ATAN2 =     1.1071487177940905030170654601785370400700476L;
constexpr long double M_ATAN2_2 =   0.5535743588970452515085327300892685200350238L;
// atan(1/2)
constexpr long double M_ATAN_HALF = 0.46364760900080611621425623146121440202853705L;
constexpr long double M_PI_180 =    0.0174532925199432957692369076848861271111L;
constexpr long double M_180_PI =    57.29577951308232087679815481410517033240547L;

// asin(sqrt(3.0 / 28.0))
constexpr long double M_AP7_ROT_RADS = 0.333473172251832115336090755351601070065900389L;
// asin(sqrt(3.0 / 28.0))*180.0/pi
constexpr long double M_AP7_ROT_DEGS = 19.106605350869094394517474740130082234976075229L;

// icosahedron geometry
//
// With the icosahedron vertices at the cyclic permutations of (0, +-1, +-phi),
// phi = (1 + sqrt(5))/2, the edge (0,1,phi)-(0,-1,phi) has its midpoint on the
// z-axis, so the north pole lies on an edge midpoint and the equator is a
// mirror plane of the icosahedron. Vertex (0,1,phi) then has latitude
// atan(phi/1) = atan(phi). This is DGGRID's default vert0 latitude.
// atan(phi) (radians)
constexpr long double M_ATAN_PHI =
                     1.01722196789785136772278896155048292206356087699L;
// atan(phi) * 180/pi (degrees)
constexpr long double M_ICOSA_VERT0_LAT_DEG =
                     58.2825255885389946757860968602266473356021071498L;

// ISEA (Snyder 1992) constants
//
// g: the spherical distance from a face centre to a face vertex,
// g = atan(3 - sqrt(5)) = atan(2/phi^2) (radians; 37.3773681406496956...deg)
constexpr long double M_ISEA_G =
                     0.652358139784368185995390631643822574365307919963L;
// scale constants: the planar face area equals the spherical face area
// 4 pi R^2 / 20, which gives the planar circumradius r_c = R * k with
// k = sqrt(4 pi / (15 sqrt(3))). Snyder's rho(vertex) = R' tan g = r_c
// then gives R'/R = k / tan g = k / (3 - sqrt(5)).
// R'/R = sqrt(4 pi / (15 sqrt(3))) / (3 - sqrt(5))
constexpr long double M_ISEA_R1 =
                     0.910383281509503568223473953373552451373533737718L;
// half the planar edge = (sqrt(3)/2) * sqrt(4 pi / (15 sqrt(3)))
constexpr long double M_ISEA_ORIGIN_X_OFF =
                     0.602295502927627353639379448917445846643753581805L;
// the planar inradius = (1/2) * sqrt(4 pi / (15 sqrt(3)))
constexpr long double M_ISEA_ORIGIN_Y_OFF =
                     0.347735470746966685358527448284139240437236623696L;

// some spherical earth datum radii
//
// Derived from the WGS84 defining parameters a = 6378137 m and
// 1/f = 298.257223563 (b = a (1 - f), e^2 = f (2 - f)).
//
// mean radius of the semi-axes R1 = (2a + b)/3; NGA publishes it rounded to
// 0.1 mm as 6371008.7714 m.
constexpr long double WGS84_MEAN_RADIUS_KM =
                     6371.00877141505983252132219987788505226605710370L;
constexpr long double WGS84_EQUATOR_RADIUS_KM =   6378.137L;
// authalic radius (the sphere with the ellipsoid's surface area) a sqrt(q_p/2),
// with q_p = 1 + ((1 - e^2)/(2e)) ln((1 + e)/(1 - e))
constexpr long double WGS84_AUTHALIC_RADIUS_KM =
                     6371.00718091847389797633784573196106269051841063L;
constexpr long double DEFAULT_RADIUS_KM =         WGS84_AUTHALIC_RADIUS_KM;

// misc

constexpr int DEFAULT_PRECISION = 7;

#endif
