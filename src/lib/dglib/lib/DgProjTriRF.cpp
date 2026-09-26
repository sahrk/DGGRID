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
// DgProjTriRF.cpp: DgProjTriRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include <dglib/DgProjTriRF.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgSphIcosa::DgSphIcosa (const DgGeoCoord& vert0, long double azimuthDegs)
{
   sphIcosa_.pt.lon = vert0.lon();
   sphIcosa_.pt.lat = vert0.lat();
   sphIcosa_.azimuth = azimuthDegs * M_PI_180;

   ico12verts();

} // DgSphIcosa::DgSphIcosa

////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& str, const DgSphIcosa& dgsi)
{
   const SphIcosa& si = dgsi.sphIcosa_;

   DgGeoCoord tmp(si.pt);
   str << "vert0: " << tmp << std::endl;
   str << "az0: " << si.azimuth * M_180_PI << std::endl;

   str << "vertices:\n";
   str << "{\n";
   for (int i = 0; i < 12; i++)
   {
      DgGeoCoord tmp(si.icoverts[i]);
      str << "  " << i << ": " << tmp << std::endl;
   }
   str << "}\n";

   str << "triangles:\n";
   str << "{\n";
   for (int i = 0; i < 20; i++)
   {
      str << "  " << i;
      for (int j = 0; j < 3; j++)
      {
         DgGeoCoord tmp(si.icotri[i][j]);
         str << " " << tmp;
      }
      str << std::endl;
   }
   str << "}\n";

   return str;

} // std::ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
GeoCoord coordtrans(const GeoCoord& newNPold, const GeoCoord& ptold,
                    long double lon0)
/* return the new coordinates of any point in orginal coordinate system.
   Define a point (newNPold) in orginal coordinate system as the North
   Pole in new coordinate system, and the great circle connect the original
   and new North Pole as the lon0 longitude in new coordinate system, given
   any point in orginal coordinate system, this function return the new
   coordinates.

   With D = ptold.lon - newNPold.lon and theta the distance from the new pole:

      sin(theta) sin(lam) = cos(lat) sin(D)
      sin(theta) cos(lam) = sin(lat) cos(latNP) - cos(lat) sin(latNP) cos(D)
      cos(theta)          = sin(lat) sin(latNP) + cos(lat) cos(latNP) cos(D)

   and the new longitude is lon0 - lam. Both angles are computed with atan2l,
   which is accurate everywhere; the previous acosl forms lost half the
   digits (sqrt(epsilon)) when lam or theta was near 0 or pi, e.g. for the
   icosahedron vertices on the lon0 and lon0 + 180 meridians. The sign rule
   of the acosl form (lon0 - |lam| for D in [0, pi), lon0 + |lam| otherwise)
   is the sign of sin(D), which atan2l applies directly. */

 {
  GeoCoord ptnew;

  const long double sinLat = sinl(ptold.lat);
  const long double cosLat = cosl(ptold.lat);
  const long double sinLatNP = sinl(newNPold.lat);
  const long double cosLatNP = cosl(newNPold.lat);
  const long double D = ptold.lon - newNPold.lon;
  const long double cosD = cosl(D);

  const long double sn = cosLat * sinl(D);                 // sin(theta) sin(lam)
  const long double cs = sinLat * cosLatNP - cosLat * sinLatNP * cosD;
                                                           // sin(theta) cos(lam)
  const long double ct = sinLat * sinLatNP + cosLat * cosLatNP * cosD;
                                                           // cos(theta)
  const long double st = hypotl(sn, cs);                   // sin(theta)

  // new latitude = pi/2 - theta
  ptnew.lat = atan2l(ct, st);

  // at (or within PRECISION*100000 of) either pole of the new system the
  // longitude is set to 0, as before
  if (st < PRECISION*100000)
      ptnew.lon=0.;
  else
   {
    ptnew.lon = lon0 - atan2l(sn, cs);
    if (ptnew.lon>M_PI_L) ptnew.lon -= M_2PI;
    if (ptnew.lon<-M_PI_L) ptnew.lon += M_2PI;
   }
  return ptnew;
 }

////////////////////////////////////////////////////////////////////////////////
int
DgSphIcosa::whichIcosaTri (const GeoCoord& pt)
/*
   Return the index of the icosahedron triangle in which pt occurs.
*/
{
   // start by assuming the first face is the minimum
   int minFace = 0;
   long double minDist = DgGeoCoord::gcDist(sphIcosa().triCen[0].pt, pt);

   // compare against the rest of the faces
   for (int i = 1; i < 20; i++)
   {
      const DgGeoCoord& triCen = sphIcosa().triCen[i].pt;
      long double dist = DgGeoCoord::gcDist(triCen, pt);
      if (dist < minDist) {
         minDist = dist;
         minFace = i;
      }
   }

   return minFace;

} /* int DgSphIcosa::whichIcosaTri */

////////////////////////////////////////////////////////////////////////////////
void
DgSphIcosa::ico12verts (void)
/*
   Fill in the icosahedron particulars given one point and one edge's azimuth.
*/
{
   GeoCoord newnpold, vertsnew[12];
   int i, j;
   Vec3D ptri[3];

   static int verts[20][3] = {

	   {  0,  1,  2 },   /*  0 */
	   {  0,  2,  3 },   /*  1 */
	   {  0,  3,  4 },   /*  2 */
           {  0,  4,  5 },   /*  3 */
	   {  0,  5,  1 },   /*  4 */
	   {  6,  2,  1 },   /*  5 */
           {  7,  3,  2 },   /*  6 */
           {  8,  4,  3 },   /*  7 */
           {  9,  5,  4 },   /*  8 */
           { 10,  1,  5 },   /*  9 */
           {  2,  6,  7 },   /* 10 */
           {  3,  7,  8 },   /* 11 */
           {  4,  8,  9 },   /* 12 */
           {  5,  9, 10 },   /* 13 */
           {  1, 10,  6 },   /* 14 */
           { 11,  7,  6 },   /* 15 */
           { 11,  8,  7 },   /* 16 */
           { 11,  9,  8 },   /* 17 */
           { 11, 10,  9 },   /* 18 */
           { 11,  6, 10 }    /* 19 */
   };

   newnpold.lat = sphIcosa().pt.lat;
   newnpold.lon = 0.0;
   for (i = 1; i <= 5; i++)
   {
     // the upper vertex ring is at latitude atan(1/2) (= 90deg - atan(2))
     vertsnew[i].lat = M_ATAN_HALF;
     vertsnew[i].lon = -sphIcosa().azimuth + 72.0L * (i - 1) * M_PI_180;
     if (vertsnew[i].lon > M_PI_L-PRECISION) vertsnew[i].lon -= M_2PI;
     if (vertsnew[i].lon < -(M_PI_L+PRECISION)) vertsnew[i].lon += M_2PI;
     vertsnew[i+5].lat = -M_ATAN_HALF;
     vertsnew[i+5].lon =
              -sphIcosa().azimuth + (36.0L + 72.0L * (i - 1)) * M_PI_180;
     if (vertsnew[i + 5].lon > M_PI_L - PRECISION)
     {
        vertsnew[i + 5].lon -= M_2PI;
     }
     if (vertsnew[i+5].lon < -(M_PI_L+PRECISION)) vertsnew[i+5].lon += M_2PI;
   }
   vertsnew[11].lat = -M_PI_2_L;
   vertsnew[11].lon = M_ZERO;
   sphIcosa().icoverts[0].lat = sphIcosa().pt.lat;
   sphIcosa().icoverts[0].lon = sphIcosa().pt.lon;
/***** hardwire for bug test ******/

/*
vertsnew[0].lat = M_PI_2_L;
vertsnew[0].lon = M_ZERO;
for (i = 0; i < 12; i++)
{
   sphIcosa().icoverts[i].lat = vertsnew[i].lat;
   sphIcosa().icoverts[i].lon = vertsnew[i].lon;
}
*/

/**********************************/
   for (i = 1; i < 12; i++)
   {
      sphIcosa().icoverts[i] =
                   coordtrans(newnpold, vertsnew[i], sphIcosa().pt.lon);
   }

   /* calculate the triangle-specific values */

   for (i = 0; i < 20; i++)
   {
      /* set the vertices */

      for (j = 0; j < 3; j++)
      {
         sphIcosa().icotri[i][j] = sphIcosa().icoverts[verts[i][j]];
      }

      /* pre-calculate the center point values */

      sphIcosa().triCen[i].pt = sphTricenpoint(sphIcosa().icotri[i]);
      sphIcosa().triCen[i].sinLat = sinl(sphIcosa().triCen[i].pt.lat);
      sphIcosa().triCen[i].sinLon = sinl(sphIcosa().triCen[i].pt.lon);
      sphIcosa().triCen[i].cosLat = cosl(sphIcosa().triCen[i].pt.lat);
      sphIcosa().triCen[i].cosLon = cosl(sphIcosa().triCen[i].pt.lon);

      /* pre-calculate the dazh's */

      const GeoCoord* t = sphIcosa().icotri[i];
      const PreCompGeo& c = sphIcosa().triCen[i];
      sphIcosa().dazh[i] = atan2l(cosl(t[0].lat) * sinl(t[0].lon - c.pt.lon),
                  c.cosLat * sinl(t[0].lat) - c.sinLat * cosl(t[0].lat) *
                  cosl(t[0].lon - c.pt.lon));

/*
      sphIcosa().dazh[i] = atan2l(cosl(sphIcosa().icotri[i][0].lat) *
                  sinl(sphIcosa().icotri[i][0].lon - sphIcosa().triCen[i].pt.lon),
                  sphIcosa().triCen[i].cosLat * sinl(sphIcosa().icotri[i][0].lat) -
                  sphIcosa().triCen[i].sinLat * cosl(sphIcosa().icotri[i][0].lat) *
                  cosl(sphIcosa().icotri[i][0].lon - sphIcosa().triCen[i].pt.lon));
*/

      /* pre-calculate the pt-in-triangle constants */

      for (j = 0; j < 3; j++) ptri[j] = llxyz(sphIcosa().icotri[i][j]);

      sphIcosa().ptin[i].p0x0 = ptri[1].y * ptri[2].z - ptri[2].y * ptri[1].z;
      sphIcosa().ptin[i].p0y0 = ptri[1].x * ptri[2].z - ptri[2].x * ptri[1].z;
      sphIcosa().ptin[i].p0z0 = ptri[1].x * ptri[2].y - ptri[2].x * ptri[1].y;

      sphIcosa().ptin[i].t00 = ptri[0].x * sphIcosa().ptin[i].p0x0 -
                              ptri[0].y * sphIcosa().ptin[i].p0y0 +
                              ptri[0].z * sphIcosa().ptin[i].p0z0;

      sphIcosa().ptin[i].p0x1 = ptri[0].y * ptri[2].z - ptri[2].y * ptri[0].z;
      sphIcosa().ptin[i].p0y1 = ptri[0].x * ptri[2].z - ptri[2].x * ptri[0].z;
      sphIcosa().ptin[i].p0z1 = ptri[0].x * ptri[2].y - ptri[2].x * ptri[0].y;

      sphIcosa().ptin[i].t01 = ptri[1].x * sphIcosa().ptin[i].p0x1 -
                              ptri[1].y * sphIcosa().ptin[i].p0y1 +
                              ptri[1].z * sphIcosa().ptin[i].p0z1;

      sphIcosa().ptin[i].p0x2 = ptri[0].y * ptri[1].z - ptri[1].y * ptri[0].z;
      sphIcosa().ptin[i].p0y2 = ptri[0].x * ptri[1].z - ptri[1].x * ptri[0].z;
      sphIcosa().ptin[i].p0z2 = ptri[0].x * ptri[1].y - ptri[1].x * ptri[0].y;

      sphIcosa().ptin[i].t02 = ptri[2].x * sphIcosa().ptin[i].p0x2 -
                              ptri[2].y * sphIcosa().ptin[i].p0y2 +
                              ptri[2].z * sphIcosa().ptin[i].p0z2;

   }

/*
   for (int i = 0; i < 20; i++)
   {
      printf("%d\n", i);
      for (int j = 0; j < 3; j++) printGeoCoord(sphIcosa().icotri[i][j]);
      printGeoCoord(sphIcosa().triCen[i].pt);
      printf("\n");
   }
*/

} /* void ico12verts */

////////////////////////////////////////////////////////////////////////////////
const char*
DgProjTriRF::str2add (DgProjTriCoord* add, const char* str, char delimiter)
                                                                          const
{
   if (!add) add = new DgProjTriCoord();

   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);

   // get the triNum
   char* tok = strtok(tmpStr, delimStr);
   int t = -1;
   if (!tok || sscanf(tok, "%d", &t) != 1)
   {
      ::report("DgProjTriRF::fromString() invalid value in string " +
               ((tok) ? std::string(tok) : std::string("\"\"")), DgBase::Fatal);
   }

   const char* tmp = &(str[strlen(tok) + 1]);
   DgDVec2D vec;
   tmp = vec.fromString(tmp, delimiter);

   *add = DgProjTriCoord(t, vec);

   return tmp;

} // const char* DgProjTriRF::str2add
