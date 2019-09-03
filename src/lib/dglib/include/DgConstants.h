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
#define M_EPSILON   0.00000000000050000000000000000000000000000L
#define M_ZERO      0.00000000000000000000000000000000000000000L
#define M_ONE       1.00000000000000000000000000000000000000000L
#define M_HALF      0.50000000000000000000000000000000000000000L
#define M_2PI       6.28318530717958647692528676655900576839433L
#define M_SQRT3     1.7320508075688772935274463415058723669428L
#define M_1_SQRT3   0.5773502691896257645091487805019574556476L
#define M_SQRT3_2   0.8660254037844386467637231707529361834714L
#define M_SQRT5     2.2360679774997896964091736687312762354406L
#define M_SQRT7     2.6457513110645905905016157536392604257102L
#define M_1_SQRT7   0.3779644730092272272145165362341800608157L
#define M_SQRT7_2   1.3228756555322952952508078768196302128551L
#define M_SQRT8     2.8284271247461900976033774484193961571393L
#define M_SQRT10    3.16227766016837933199889354443271853371955L
#define M_SQRT15    3.8729833462074168851792653997823996108329L
#define M_PHI       1.618033988749894848204586834365638117720309L
#define M_SIN60     M_SQRT3_2
#define M_COS60     0.50000000000000000000000000000000000000000L
#define M_SIN30     M_COS60
#define M_COS30     M_SIN60
#define M_ATAN2     1.1071487177940905030170654601785370400700476L
#define M_ATAN2_2   0.5535743588970452515085327300892685200350238L
// atan(1/2)
#define M_ATAN_HALF 0.46364760900080611621425623146121440202853705L
#define M_PI_180    0.0174532925199432957692369076848861271111L
#define M_180_PI    57.29577951308232087679815481410517033240547L

// asin(sqrt(3.0 / 28.0))
#define M_AP7_ROT_RADS 0.333473172251832115336090755351601070065900389L
// asin(sqrt(3.0 / 28.0))*180.0/pi
#define M_AP7_ROT_DEGS 19.106605350869094394517474740130082234976075229L

// some spherical earth datum radii
#define WGS84_MEAN_RADIUS_KM      6371.0087714L
#define WGS84_EQUATOR_RADIUS_KM   6378.137L
#define WGS84_AUTHALIC_RADIUS_KM  6371.007180918475L
#define DEFAULT_RADIUS_KM         WGS84_AUTHALIC_RADIUS_KM

// misc

#define DEFAULT_PRECISION  7

#endif
