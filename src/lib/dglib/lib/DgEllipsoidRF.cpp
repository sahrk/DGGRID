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
// DgEllipsoidRF.cpp: DgEllipsoidRF class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgPolygon.h>

const DgGeoCoord DgGeoCoord::undefGeoCoord(LDBL_MAX, LDBL_MAX);
const long double DgGeoCoord::tolerance = 0.0000000005L;

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::gcDist (const DgGeoCoord& g1, const DgGeoCoord& g2,
                    bool rads)
/*
   return great circle distance in radians between g1 and g2.

   CRC Math, 1991, pp. 129-130.

   Kevin Sahr, 3/10/99
*/
{
   // use spherical triangle with g1 at A, g2 at B, and north pole at C

   long double bigC = fabsl(g2.lon() - g1.lon());

   if (bigC > M_PI) // assume we want the complement
   {
      // note that in this case they can't both be negative

      long double lon1 = g1.lon();
      if (lon1 < 0.0L) lon1 += 2.0L * M_PI;
      long double lon2 = g2.lon();
      if (lon2 < 0.0L) lon2 += 2.0L * M_PI;

      bigC = fabsl(lon2 - lon1);
   }

   long double b = M_PI_2 - g1.lat();
   long double a = M_PI_2 - g2.lat();

   // use law of cosines to find c

   long double cosc = cosl(a) * cosl(b) + sinl(a) * sinl(b) * cosl(bigC);
   if (cosc > 1.0L) cosc = 1.0L;
   if (cosc < -1.0L) cosc = -1.0L;

   long double retVal = acosl(cosc);

   if (!rads) retVal *= M_180_PI;

   return retVal;

} // long double DgGeoCoord::gcDist

////////////////////////////////////////////////////////////////////////////////
DgGeoCoord
DgGeoCoord::gcIntersect (const DgGeoCoord& g11, const DgGeoCoord& g12,
          const DgGeoCoord& g21, const DgGeoCoord& /* g22 */)
/*
   Return point of intersection of the two great circle arc segments g11-g12
   and g21-g22.

   Works by calling Lian Song's routine GCintersect.
*/
{
   GeoCoord sv11, sv12, sv21;
   sv11.lon = g11.lon(); sv11.lat = g11.lat();
   sv12.lon = g12.lon(); sv12.lat = g12.lat();
   sv21.lon = g21.lon(); sv21.lat = g21.lat();
   //sv22.lon = g22.lon(); sv22.lat = g22.lat();

   GeoCoord ans = GCintersect(sv11, sv12, sv21, sv11, 1);

   return DgGeoCoord(ans.lon, ans.lat);

} // DgGeoCoord DgGeoCoord::gcIntersect

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::geoTriArea (const DgGeoCoord& g1, const DgGeoCoord& g2,
                        const DgGeoCoord& g3)
/*
   return area in radians.

   CRC Math, 1991, pp. 129-130.

   Kevin Sahr, 3/10/99
*/
{
   // determine the edges

   long double a = DgGeoCoord::gcDist(g2, g3);
   long double b = DgGeoCoord::gcDist(g1, g3);
   long double c = DgGeoCoord::gcDist(g1, g2);

   // determine the angles using half-angle formulas

   long double s = (a + b + c) / 2.0L;

   long double sinsa = sinl(s - a);
   long double sinsb = sinl(s - b);
   long double sinsc = sinl(s - c);

   long double k = sqrtl(sinsa * sinsb * sinsc / sinl(s));

   long double bigA = 2.0L * atanl(k / sinsa);
   long double bigB = 2.0L * atanl(k / sinsb);
   long double bigC = 2.0L * atanl(k / sinsc);

   long double E = bigA + bigB + bigC - M_PI;

/*
   std::cout << "geoTriArea: " << g1 << " " << g2 << " " << g3 << std::endl
        << " a: " << a
        << " b: " << b
        << " c: " << c << std::endl
        << " A: " << bigA
        << " B: " << bigB
        << " C: " << bigC << std::endl
        << " E: " << E << " "
        << E / (4.0L * M_PI) << std::endl;
*/

   return E;

} // long double DgGeoCoord::geoTriArea

////////////////////////////////////////////////////////////////////////////////
long double
DgGeoCoord::geoPolyArea (const DgPolygon& poly, const DgGeoCoord center)
//
// returns area of spherical polygon in radians;
//
// assumes DgAddressBase for poly is DgGeoCoord
//
// Assumes poly is a "reasonably" convex polygon (i.e. a cell bouncary) and
// that center is an interior point.
//
{
   long double totArea = 0.0L;

   const DgGeoSphRF* geoRF = dynamic_cast<const DgGeoSphRF*>(&poly.rf());
   if (geoRF == 0)
      report("DgGeoCoord::geoPolyArea() non-geo polygon", DgBase::Fatal);

   // do each sub-triangle
   for (int i = 0; i < poly.size(); i++)
   {
      const DgGeoCoord& v1 = *geoRF->getAddress(poly[i]);
      const DgGeoCoord& v2 = *geoRF->getAddress(poly[(i + 1) % poly.size()]);

      totArea += DgGeoCoord::geoTriArea(center, v1, v2);
   }

   return totArea;

} // long double DgGeoCoord::geoPolyArea

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char*
DgEllipsoidRF::str2add (DgGeoCoord* add, const char* str, char delimiter) const
{
   if (!add) add = new DgGeoCoord();

   DgDVec2D vec;
   const char* tmp = vec.fromString(str, delimiter);

   *add = DgGeoCoord(vec.x(), vec.y(), false);

   return tmp;

} // const char* DgEllipsoidRF::str2add

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*
   sphTrig.C: vector and spherical trig utilities

   Lian Song and Kevin Sahr, July 24, 1994.

   Kevin Sahr, merged & optimized, July 20, 2001
*/
/******************************************************************************/

#include <dglib/DgBase.h>

#include <cmath>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************/
void sphTriInit (SphTri* tri)
/*
   Initialize tri with UNDEFVAL values.
*/
{
   int i;

   for (i = 0; i < 3; i++)
   {
      tri->verts[i].lon  = tri->verts[i].lat  = UNDEFVAL;
      tri->edges[i] = tri->angles[i] = UNDEFVAL;
   }
   tri->code = UNDEFINT;
   tri->area = tri->perimeter = UNDEFVAL;
   tri->compactness = UNDEFVAL;

} /* void sphTriInit */

/******************************************************************************/
void planeTriInit (PlaneTri* tri)
/*
   Initialize tri with UNDEFVAL values.
*/
{
   int i;

   for (i = 0; i < 3; i++)
      tri->points[i].x  = tri->points[i].y = UNDEFVAL;
   tri->code = tri->direction =  UNDEFINT;
   tri->cenpoint.x = tri->cenpoint.y = UNDEFVAL;

} /* void planeTriInit */

/******************************************************************************/
void printInt (long long int val)
/*
   Print val or 'UNDEFVAL' to stdout as appropriate.
*/
{
   if (val == UNDEFVAL)
      dgcout << "UNDEFVAL";
   else
      dgcout << val;

} /* void printInt */

/******************************************************************************/
void printVal (long double val)
/*
   Print val or 'UNDEFVAL' to stdout as appropriate.
*/
{
   if (val == UNDEFVAL)
      dgcout << "UNDEFVAL";
   else
      dgcout << val;

} /* void printVal */

/******************************************************************************/
void printGeoCoord (const GeoCoord& p)
/*
   Print p to stdout.
*/
{
   dgcout << "(" << p.lon * M_180_PI << ", " << p.lat * M_180_PI << ")";

} /* void printGeoCoord */

/******************************************************************************/
void printVec2D (const Vec2D& p)
/*
   Print p to stdout.
*/
{
   dgcout << "(" << p.x << ", " << p.y << ")";

} /* void printVec2D */

/******************************************************************************/
void printSphTri (const SphTri& tri)
/*
   Print tri to stdout.
*/
{
   int i;

   dgcout << "{\n  code: ";
   printInt(tri.code);
   dgcout << "\n  vertices: ";
   for (i = 0; i < 3; i++)
   {
      dgcout << " ";
      printGeoCoord(tri.verts[i]);
   }
   dgcout << "\n";

   dgcout << "  A: "; printVal(tri.edges[0]);
   dgcout << "  B: "; printVal(tri.edges[1]);
   dgcout << "  C: "; printVal(tri.edges[2]);
   dgcout << "\n";

   dgcout << "  a: "; printVal(tri.angles[0]* M_180_PI);
   dgcout << "  b: "; printVal(tri.angles[1]* M_180_PI);
   dgcout << "  c: "; printVal(tri.angles[2]* M_180_PI);
   dgcout << "\n";

   dgcout << "  area: "; printVal(tri.area);
   dgcout << "  perimeter: "; printVal(tri.perimeter);
   dgcout << "  compactness: "; printVal(tri.compactness);
   dgcout << "\n";

   dgcout << "}\n";

} /* void printSphTri */

/******************************************************************************/
void printPlaneTri (const PlaneTri& tri)
/*
   Print tri to stdout.
*/
{
   int i;

   dgcout << "{\n  code: "; printInt(tri.code);
   dgcout << "\n  vertices: ";
   for (i = 0; i < 3; i++)
   {
      dgcout << " ";
      printVec2D(tri.points[i]);
   }
   dgcout << "\n";
   printVec2D(tri.cenpoint);
   dgcout << "\n";

   dgcout << "}\n";

} /* void PlaneTri */

/******************************************************************************/
long double maxval(long double val1, long double val2)
/*
  return the maxmum of two variables
*/
 {
  long double maxx;
  if (val1>val2) maxx=val1;
  else maxx=val2;
  return maxx;
 } /* long double maxval */

/******************************************************************************/
long double minval(long double val1, long double val2)
/*
  return the minmum of two variables
*/
 {
  long double minn;
  if (val1<val2) minn=val1;
  else minn=val2;
  return minn;
 } /* long double minval */

/******************************************************************************/
GeoCoord sphTricenpoint(GeoCoord sp[3])
/*
   Calculate and return the center point of a sphere triangle
   Input and output unit is degree.
*/
 {
  DgDVec3D cp = DgDVec3D::fromGeo(sp[0]) + DgDVec3D::fromGeo(sp[1]) +
                DgDVec3D::fromGeo(sp[2]);
  cp /= 3.0L;
  return cp.normalized().toGeo();
 } /* GeoCoord sphTricenpoint */

/******************************************************************************/
long double chorddist(const GeoCoord& ll1, const GeoCoord& ll2)
/*
   Calculate the chord distance between two points on the sphere
   Input unit is degree. Output unit regard the earth's radius as one unit.
*/
 {
  return DgDVec3D::fromGeo(ll1).distance(DgDVec3D::fromGeo(ll2));

 } /* long double chorddist */

/******************************************************************************/
long double spheredist(const GeoCoord& ll1, const GeoCoord& ll2)
/*
   Calculate the sphere distance between two points on the sphere
   Input and Output unit is in radius.
*/
 {
  long double sd;
  sd=cosl(M_PI_2-ll1.lat)*cosl(M_PI_2-ll2.lat)+
     sinl(M_PI_2-ll1.lat)*sinl(M_PI_2-ll2.lat)*cosl(ll1.lon-ll2.lon);
  if (sd>1) sd=1;
  if (sd<-1) sd=-1;
  return acosl(sd);
 }/* long double spheredist */

/******************************************************************************/
GeoCoord GCmidpoint(const GeoCoord& pp1, const GeoCoord& pp2)
{
  DgDVec3D mpt = DgDVec3D::fromGeo(pp1) + DgDVec3D::fromGeo(pp2);
  mpt /= 2.0L;
  return mpt.normalized().toGeo();
}

/******************************************************************************/
long double Azimuth(const GeoCoord& pt1, const GeoCoord& pt2)
/*
  return the azimuth of pt2 relatived to pt1
  output azimuth unit is radius
*/
 {
  long double az;

  az=atan2l(cosl(pt2.lat)*sinl(pt2.lon-pt1.lon), cosl(pt1.lat)*sinl(pt2.lat)-
     sinl(pt1.lat)*cosl(pt2.lat)*cosl(pt2.lon-pt1.lon));
  return az;
 } /* long double Azimuth */

/******************************************************************************/
GeoCoord GCintersect(const GeoCoord& sv11, const GeoCoord& sv12,
                     const GeoCoord& sv21, const GeoCoord& sv22, int sign)
/*
  return the intersect point of two great circle
  sign=1: two great circle segment with ends of sv11 and sv12,
          sv21 and sv22 respectively
  sign=0: two whole great circle with one pass sv11 and sv12, one pass
          sv21 and sv22, return the intersect point on North Hemisphere
*/
 {
  GeoCoord pt;
  long double a,b,maxlon,minlon;

/* calculate the intersect point of two great circle */
  const DgDVec3D nn1 = DgDVec3D::fromGeo(sv11).cross(DgDVec3D::fromGeo(sv12));
  const DgDVec3D nn2 = DgDVec3D::fromGeo(sv21).cross(DgDVec3D::fromGeo(sv22));
  long double ppx = 0.0L, ppy = 0.0L, ppz = 0.0L;
  if ((nn2.z()*nn1.y()-nn1.z()*nn2.y())!= 0.0L) {
    b=(nn1.x()*nn2.y()-nn2.x()*nn1.y())/(nn2.z()*nn1.y()-nn1.z()*nn2.y());
    a=(nn2.x()*nn1.z()-nn1.x()*nn2.z())/(nn1.y()*nn2.z()-nn2.y()*nn1.z());
    ppx=1/sqrtl(a*a+b*b+1);
    ppy=a*ppx;
    ppz=b*ppx;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) &&
         ((nn1.x()*nn2.y()-nn2.x()*nn1.y())==0.0L) && ((nn1.x()*nn2.z()-nn2.x()*nn1.z())==0.0L)) {
    report("Error in GCintersect: the two great circle planes are parallel.\n",
           DgBase::Fatal);
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn1.z()!=0.0L)) {
    ppx=0.0L;
    ppy=1.0L/sqrtl(1+nn1.y()*nn1.y()/nn1.z()/nn1.z());
    ppz=-nn1.y()/nn1.z()*ppy;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn2.z()!=0.0L)) {
    ppx=0.0L;
    ppy=1.0L/sqrtl(1.0L+nn2.y()*nn2.y()/nn2.z()/nn2.z());
    ppz=-nn2.y()/nn2.z()*ppy;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn1.y()!=0.0L)) {
    ppx=0.0L;
    ppz=1/sqrtl(1+nn1.z()*nn1.z()/nn1.y()/nn1.y());
    ppy=-nn1.z()/nn1.y()*ppz;
   } else if (((nn2.z()*nn1.y()-nn1.z()*nn2.y())==0.0L) && (nn2.y()!=0.0L)) {
    ppx=0.0L;
    ppz=1.0L/sqrtl(1.0L+nn2.z()*nn2.z()/nn2.y()/nn2.y());
    ppy=-nn2.z()/nn2.y()*ppz;
   }
  DgDVec3D pp(ppx, ppy, ppz);

  if (sign==0) {
    if (pp.z()<0.0L) pp *= -1.0L;
    pt=pp.toGeo();
    return pt;
  } else {
   /* judge if the point is on the two great circle segment */

  pt=pp.toGeo();
  maxlon=maxval(sv11.lon,sv12.lon);
  minlon=minval(sv11.lon,sv12.lon);
  if ((pt.lon<=maxlon) && (pt.lon>=minlon))
   return pt;
  else {
    pt=(pp * -1.0L).toGeo();
    if ((pt.lon<=maxlon) && (pt.lat>=minlon))
     return pt;
    else {
      dgcerr << "Error of GCintersect: the point is not on great circle segment.\n";
      pt.lat=UNDEFVAL; pt.lon=UNDEFVAL;
      return pt;
    }
  }
  }
} /* GeoCoord GCintersect */

/******************************************************************************/
long double GCptlat(long double lon, const GeoCoord& sv1, const GeoCoord& sv2)
/*
  return latitude of the point on great circle segment with known longtitude
*/
 {
  long double lat,a,b,c;
  const DgDVec3D p1 = DgDVec3D::fromGeo(sv1);
  const DgDVec3D p2 = DgDVec3D::fromGeo(sv2);

  a=(p1.y()*p2.z()-p1.z()*p2.y())*cosl(lon);
  b=(p1.x()*p2.z()-p1.z()*p2.x())*sinl(lon);
  c=(p1.x()*p2.y()-p1.y()*p2.x());
  if (c!=0.0L) lat=atanl((b-a)/c);
  else {
     lat = UNDEFVAL;
     dgcerr << "Error in GCptlat: the two end points are at one longitude.\n";
  }
  return(lat);
 } /* long double GCptlat */

/******************************************************************************/
int ptinsphtri(const GeoCoord& pt, GeoCoord sTri[3])
/*
  decide if a point is in a spherical triangle
  return 1 if it is in and return 0 if not
*/
{
   // pt is on the same side as each vertex of the plane through the other
   // two vertices (and the centre): compare the signs of the triple products
   DgDVec3D ptri[3];
   for (int i = 0; i < 3; i++) ptri[i] = DgDVec3D::fromGeo(sTri[i]);
   const DgDVec3D pp = DgDVec3D::fromGeo(pt);

   for (int k = 0; k < 3; k++)
   {
      const DgDVec3D n = ptri[(k + 1) % 3].cross(ptri[(k + 2) % 3]);
      if (pp.dot(n) * ptri[k].dot(n) < 0.0L) return 0;
   }

   return 1;

}  /* ptinsphtri */

/******************************************************************************/
GeoCoord GCdaz(const GeoCoord& pt, long double distance, long double az)
/*
  compute and return the point whose azimuth (az) and distance
  (distance) relative to a known point (pt) are given.
  all angles are in radians.

  The point is p cos(distance) + (n cos(az) + e sin(az)) sin(distance), with
  p the unit vector of pt and n, e the unit north and east tangents at p.
  These stay defined at a pole (north is then along the meridian pt.lon), so
  paths along a meridian, over a pole or from a pole need no special case.
*/
 {
  const long double sinLat = sinl(pt.lat), cosLat = cosl(pt.lat);
  const long double sinLon = sinl(pt.lon), cosLon = cosl(pt.lon);

  const DgDVec3D p(cosLat * cosLon, cosLat * sinLon, sinLat);
  const DgDVec3D n(-sinLat * cosLon, -sinLat * sinLon, cosLat);
  const DgDVec3D e(-sinLon, cosLon, 0.0L);

  const DgDVec3D q = p * cosl(distance) +
                     (n * cosl(az) + e * sinl(az)) * sinl(distance);

  return q.toGeo();
 } /* GeoCoord GCdaz */

/******************************************************************************/
