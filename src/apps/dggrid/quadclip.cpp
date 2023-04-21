/*******************************************************************************
    Copyright (C) 2021 Kevin Sahr

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
// quadclip.cpp: quadclip class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>
#include <map>

using namespace std;

#include "dggrid.h"
#include "gridgen.h"
#include "clipper.hpp"
#include <dglib/DgIVec2D.h>
#include <dglib/DgInAIGenFile.h>
#include <dglib/DgInShapefile.h>
#include <dglib/DgInGDALFile.h>
#include <dglib/DgOutShapefile.h>
#include <dglib/DgInShapefileAtt.h>
#include <dglib/DgOutLocFile.h>
#include <dglib/DgOutGdalFile.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgBoundedRF2D.h>
#include <dglib/DgParamList.h>
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgGeoProjConverter.h>
#include <dglib/DgRandom.h>
#include <dglib/DgCell.h>
#include <dglib/DgLocList.h>
#include <dglib/DgDmdD4Grid2D.h>
#include <dglib/DgDmdD4Grid2DS.h>
#include <dglib/DgTriGrid2D.h>
#include <dglib/DgOutRandPtsText.h>
#include <dglib/DgIDGGSBase.h>
#include "DgHexSF.h"

using namespace dgg::topo;

//////////////////////////////////////////////////////////////////////////////
ClipperLib::Paths*
intersectPolyWithQuad (const DgPolygon& v, const GridGenParam& dp,
                       DgQuadClipRegion& clipRegion)
//
// caller takes responsibility for the returned memory
//
{
   //// create a local copy and project to gnomonic
   DgPolygon polyVec(v);
   if (dp.megaVerbose) dgcout << "input polyVec(v): " << polyVec << endl;
   clipRegion.gnomProj().convert(polyVec);
   if (dp.megaVerbose) dgcout << " -> input: " << polyVec << endl;

   //// now create a clipper poly version
   ClipperLib::Paths clpPoly(1);

   for (int i = 0; i < polyVec.size(); i++) {
      const DgDVec2D& p0 = *clipRegion.gnomProj().getAddress(polyVec[i]);

      if (dp.megaVerbose) dgcout << "clipper: \n" << " i: " << i << " " << p0 << endl;

      clpPoly[0] <<
           ClipperLib::IntPoint(dp.clipperFactor * p0.x(), dp.clipperFactor * p0.y());
   }

   //// find the intersections

   ClipperLib::Clipper c;
   c.AddPaths(clpPoly, ClipperLib::ptSubject, true);
   c.AddPaths(clipRegion.gnomBndry(), ClipperLib::ptClip, true);

   ClipperLib::Paths* solution = new ClipperLib::Paths();
   c.Execute(ClipperLib::ctIntersection, *solution,
                  ClipperLib::pftNonZero, ClipperLib::pftNonZero);

   return solution;
}

//////////////////////////////////////////////////////////////////////////////
void processOneClipPoly (DgPolygon& polyIn, GridGenParam& dp, const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], DgInShapefileAtt* pAttributeFile)
{
   if (dp.megaVerbose) dgcout << "input: " << polyIn << endl;

   if (dp.geoDens > 0.000000000001)
      DgGeoSphRF::densify(polyIn, dp.geoDens);

   if (dp.megaVerbose) dgcout << "densified: " << polyIn << endl;

   // create a copy to test for intersected quads

   DgPolygon quadVec(polyIn);

   if (dp.megaVerbose) dgcout << "quadVec(polyIn): " << quadVec << endl;

   dgg.q2ddRF().convert(quadVec);

   if (dp.megaVerbose) dgcout << "->: " << quadVec << endl;

   // set up to track which quads are intersected
   bool quadInt[12]; // which quads are intersected?
   for (int q = 0; q < 12; q++)
      quadInt[q] = false;

   // test in which quads vertices fall
   for (int i = 0; i < quadVec.size(); i++) {
      const DgQ2DDCoord& qc = *dgg.q2ddRF().getAddress(quadVec[i]);
      quadInt[qc.quadNum()] = true;
   }

   // test for vertices over 90' from an intersected
   // quad center point, which will make the gnomonic fail

   DgPolygon quadVec2(polyIn);
   for (int q = 1; q < 11; q++) {
      if (quadInt[q]) {
         const DgGeoCoord& cp = clipRegions[q].gnomProj().proj0();
         bool allGood = true;
         for (int i = 0; i < quadVec2.size(); i++) {
            if (DgGeoCoord::gcDist(
              *dgg.geoRF().getAddress(quadVec2[i]), cp, false) > 90.0) {

               dgcerr << "ERROR: polygon intersects quad #"
                 << dgg::util::to_string(q) << " but a vertex of that polygon is "
                 << "more than 90' from the quad center." << endl;
               dgcerr << "polygon is: " << endl;
               dgcerr << polyIn << endl;
               report("break-up polygon or reorient grid",
                       DgBase::Fatal);
               allGood = false;

               break;
            }
         }

         if (allGood)
            if (dp.megaVerbose) dgcout << "intersects quad " << q << endl;
      }
   }

#ifdef USE_GDAL
   int numHoles = 0;
   int** holeQuads = NULL;
   if (dp.useHoles) {
      //// determine which holes have vertices on which quads
      numHoles = polyIn.holes().size();
      holeQuads = new int*[numHoles];
      for (int h = 0; h < numHoles; h++) {

         // initialize the counts
         holeQuads[h] = new int[12];
         for (int q = 0; q < 12; q++)
            holeQuads[h][q] = 0;

         DgPolygon quadVec(*polyIn.holes()[h]);
         dgg.q2ddRF().convert(quadVec);

         // count the quads vertices fall in
         for (int i = 0; i < quadVec.size(); i++) {
            const DgQ2DDCoord& qc = *dgg.q2ddRF().getAddress(quadVec[i]);
            ++holeQuads[h][qc.quadNum()];
         }
      }
   }
#endif

   // now perform the intersection for each quad intersected
   int nQuadsInt = 0;
   for (int q = 1; q < 11; q++) {

      if (dp.megaVerbose) dgcout << "intersecting with quads..." << endl;

      if (!quadInt[q]) continue;

      if (dp.megaVerbose) dgcout << "INTERSECTING quad " << q << endl;

      if (dp.megaVerbose) dgcout << "input: " << polyIn << endl;

      ClipperLib::Paths* solution =
                intersectPolyWithQuad (polyIn, dp, clipRegions[q]);

      if (solution->size() == 0) {
         if (dp.megaVerbose)
            dgcout << "no intersection in quad " << q << endl;

         continue;
      }

      // if we're here we have intersection(s)
      if (dp.megaVerbose)
         dgcout << solution->size() << " intersections FOUND in quad " << q << endl;

      nQuadsInt++;
      clipRegions[q].setIsQuadUsed(true);

      ////// now convert back to Snyder and add to the clipRegions
      DgQuadClipRegion& cr = clipRegions[q];
      for (size_t i = 0; i < solution->size(); i++) {

         DgPolygon locv(cr.gnomProj());
         for (size_t j = 0; j < (*solution)[i].size(); j++) {
            DgDVec2D p0 = DgDVec2D(dp.invClipperFactor * (*solution)[i][j].X,
                        dp.invClipperFactor * (*solution)[i][j].Y);
            DgLocation* tloc = cr.gnomProj().makeLocation(p0);

            locv.push_back(*tloc);
            delete tloc;
         }

         if (dp.megaVerbose) dgcout << "locv: " << locv << endl;

         dgg.geoRF().convert(locv);

         if (dp.megaVerbose) dgcout << "->" << locv << endl;

         dgg.q2ddRF().convert(locv);

         if (dp.megaVerbose) dgcout << "->" << locv << endl;

         // add the intersection to our clipper list

         ClipperLib::Paths cfinVerts(1);
         for (int j = 0; j < locv.size(); j++) {

            const DgQ2DDCoord& qc = *dgg.q2ddRF().getAddress(locv[j]);
            const DgDVec2D& p0 = qc.coord();

            // update the bounding box
            if (p0.x() < cr.minx()) cr.setMinx(p0.x());
            if (p0.y() < cr.miny()) cr.setMiny(p0.y());
            if (p0.x() > cr.maxx()) cr.setMaxx(p0.x());
            if (p0.y() > cr.maxy()) cr.setMaxy(p0.y());

            if (qc.quadNum() != q)
               report("intersect poly crosses quad boundary; adjust "
                   "nudge", DgBase::Fatal);

            cfinVerts[0] << ClipperLib::IntPoint(dp.clipperFactor * p0.x(),
                                          dp.clipperFactor * p0.y());
         }

         // start building the clipping poly definition
         DgClippingPoly clipPoly;
         clipPoly.exterior = cfinVerts;

#ifdef USE_GDAL
         if (dp.useHoles) {
            // add the holes
            for (int h = 0; h < polyIn.holes().size(); h++) {

               // check for vertices in this quad
               int numVertsInQuad = holeQuads[h][q];
               if (numVertsInQuad > 0) {
                  DgPolygon theHole(*polyIn.holes()[h]);
                  DgClippingHole clipHole;

                  // check if all vertices are on this quad
                  //const DgRFBase* rf = NULL;
                  if (numVertsInQuad == theHole.size()) {
                     clipHole.isGnomonic = false; // use snyder
                     dgg.q2ddRF().convert(theHole);
                       // note that we've already done this above

                     DgPolygon ccHole(dgg.ccFrame());
                     for (int i = 0; i < theHole.size(); i++) {
                        const DgQ2DDCoord& q2v = *dgg.q2ddRF().getAddress(theHole[i]);
                        DgLocation* tloc = dgg.ccFrame().makeLocation(q2v.coord());
                        ccHole.push_back(*tloc);
                        delete tloc;
                     }

                     theHole = ccHole;

                  } else {
                     clipHole.isGnomonic = true; // use gnomonic
                     cr.gnomProj().convert(theHole);
                     //rf = &cr.gnomProj();
                  }

                  OGRPolygon* ogrPoly = DgOutGdalFile::createPolygon(theHole);
/*
char* clpStr = ogrPoly->exportToJson();
printf("ogrPoly:\n%s\n", clpStr);
*/

                  clipHole.hole = *ogrPoly;
                  delete ogrPoly;

                  clipPoly.holes.push_back(clipHole);
               }
            }
         }
#endif
         // store the clipping poly definition
         clipRegions[q].clpPolys().push_back(clipPoly);

         //// add the attributes for this polygon
         if (dp.buildShapeFileAttributes) {
            const set<DgDBFfield>& fields = pAttributeFile->curObjFields();
            clipRegions[q].polyFields().push_back(fields);
         }

         //// update the i,j bounds for this quad

         for (int j = 0; j < locv.size(); j++) {

            DgLocation* tloc = dgg.ccFrame().makeLocation(
                               dgg.q2ddRF().getAddress(locv[j])->coord());

            dgg.grid2D().convert(tloc);
            const DgIVec2D& coord = *dgg.grid2D().getAddress(*tloc);

            if (coord.i() < clipRegions[q].offset().i())
               clipRegions[q].setOffset(
                  DgIVec2D(coord.i(), clipRegions[q].offset().j()));

            if (coord.j() < clipRegions[q].offset().j())
               clipRegions[q].setOffset(
                  DgIVec2D(clipRegions[q].offset().i(), coord.j()));

            if (coord.i() > clipRegions[q].upperRight().i())
               clipRegions[q].setUpperRight(
                DgIVec2D(coord.i(), clipRegions[q].upperRight().j()));

            if (coord.j() > clipRegions[q].upperRight().j())
               clipRegions[q].setUpperRight(
                DgIVec2D(clipRegions[q].upperRight().i(), coord.j()));

            delete tloc;
         }
      }

      delete solution;
   }

#ifdef USE_GDAL
      if (dp.useHoles) {
         for (int h = 0; h < numHoles; h++)
            delete [] holeQuads[h];
         delete [] holeQuads;
      }
#endif

} // void processOneClipPoly

//////////////////////////////////////////////////////////////////////////////
void createClipRegions (GridGenParam& dp, const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], set<DgIVec2D> overageSet[],
             map<DgIVec2D, set<DgDBFfield> > overageFields[])
{
   dgcout << "\n* building clipping regions..." << endl;

   // initialize the i,j bounds
   for (int q = 1; q < 11; q++)
   {
      clipRegions[q].setQuadNum(q);
      clipRegions[q].setOffset(DgIVec2D(dgg.maxD(), dgg.maxJ()));
      clipRegions[q].setUpperRight(DgIVec2D(0, 0));
   }

   // create the gnomonic quad boundaries to clip against

   if (dp.megaVerbose)
      dgcout << "creating gnomonic quad boundaries..." << endl;

   //// find center for gnomonic; use a dummy diamond

   DgRFNetwork tmpNet;
   const DgContCartRF* ccRFptr = DgContCartRF::makeRF(tmpNet);
   const DgContCartRF& ccRF = *ccRFptr;
   const DgDmdD4Grid2DS* dmd =
        DgDmdD4Grid2DS::makeRF(tmpNet, ccRF, 1, 4, true, false);

   const DgDmdD4Grid2D& dmd0 =
        *(dynamic_cast<const DgDmdD4Grid2D*>(dmd->grids()[0]));

   // generate the center on the dummy grid

   DgLocation cent;
   dmd0.setPoint(DgIVec2D(0, 0), cent);
   ccRF.convert(&cent);

   // generate the boundary on the dummy grid; adjust the boundary so
   // it will definately be on the quad after reprojection

   DgPolygon verts;
   dmd0.setVertices(DgIVec2D(0, 0), verts);
   ccRF.convert(verts);

   if (dp.megaVerbose) dgcout << "verts:\n" << verts << endl;

   // lower left

   DgDVec2D p = *ccRF.getAddress(verts[0]);
   p += DgDVec2D(0.0, dp.nudge);
   DgLocation* tloc = ccRF.makeLocation(p);
   verts.setLoc(0, *tloc);
   delete tloc;

   // lower right

   p = *ccRF.getAddress(verts[1]);
   p += DgDVec2D(-2.0 * dp.nudge, dp.nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(1, *tloc);
   delete tloc;

   // upper right

   p = *ccRF.getAddress(verts[2]);
   p += DgDVec2D(0.0, -dp.nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(2, *tloc);
   delete tloc;

   // upper left

   p = *ccRF.getAddress(verts[3]);
   p += DgDVec2D(2.0 * dp.nudge, -dp.nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(3, *tloc);
   delete tloc;

   if (dp.megaVerbose) dgcout << "(nudge) ->" << verts << endl;

   for (int q = 1; q < 11; q++) {

      if (dp.megaVerbose) dgcout << "quad " << q << endl;

      // find gnomonic projection center for this quad

      tloc = dgg.q2ddRF().makeLocation(
                            DgQ2DDCoord(q, *ccRF.getAddress(cent)));

      if (dp.megaVerbose) dgcout << "center: " << *tloc;

      dgg.geoRF().convert(tloc);

      if (dp.megaVerbose) dgcout << " -> " << *tloc << endl;

      clipRegions[q].setGnomProj(DgProjGnomonicRF::makeRF(dgg.network(),
           string("gnom") + dgg::util::to_string(q, 2),
           *dgg.geoRF().getAddress(*tloc)));

      delete tloc;

      Dg2WayGeoProjConverter(dgg.geoRF(), clipRegions[q].gnomProj());

      // now find gnomonic quad boundary

      // kludge to jump nets

      DgPolygon v0(dgg.q2ddRF());
      for (int i = 0; i < verts.size(); i++)
      {
         tloc = dgg.q2ddRF().makeLocation(
                               DgQ2DDCoord(q, *ccRF.getAddress(verts[i])));
         v0.push_back(*tloc);
         delete tloc;
      }

      if (dp.megaVerbose)
	  dgcout << v0 << endl;

      dgg.geoRF().convert(v0);

      if (dp.megaVerbose)
	  dgcout << " -> " << v0 << endl;

      clipRegions[q].gnomProj().convert(v0);

      if (dp.megaVerbose)
	  dgcout << " -> " << v0 << endl;

      // finally store the boundary as a clipper polygon
      ClipperLib::Path contour;
      for (int i = 0; i < v0.size(); i++) {
         const DgDVec2D& p0 = *clipRegions[q].gnomProj().getAddress(v0[i]);
         contour << ClipperLib::IntPoint( dp.clipperFactor*p0.x() , dp.clipperFactor*p0.y() );
      }

      clipRegions[q].gnomBndry().push_back(contour);
   }

   // load the clipping polygons or points
   if (dp.regionClip || dp.pointClip) {

      //// read in the region boundary files
      for (unsigned long fc = 0; fc < dp.regionFiles.size(); fc++) {

         DgInLocFile* pRegionFile = NULL;
         DgInShapefileAtt* pAttributeFile = NULL;
         if (dp.clipAIGen)
            pRegionFile = new DgInAIGenFile(dgg.geoRF(), &dp.regionFiles[fc]);
         else if (dp.clipShape) {
            if (dp.buildShapeFileAttributes) {

               pRegionFile = pAttributeFile =
                     new DgInShapefileAtt(dgg.geoRF(), &dp.regionFiles[fc]);

               // add any new fields to the global list
               const set<DgDBFfield>& fields = pAttributeFile->fields();
               for (set<DgDBFfield>::iterator it = fields.begin();
                    it != fields.end(); it++) {
                  if (it->fieldName() == "global_id") continue;

                  set<DgDBFfield>::iterator cur = dp.allFields.find(*it);
                  if (cur == dp.allFields.end()) // new field
                     dp.allFields.insert(*it);
                  else if (cur->type() != it->type())
                     report("input files contain incompatible definitions "
                            "of attribute field " + it->fieldName(), DgBase::Fatal);
               }
            } else {
               pRegionFile = new DgInShapefile(dgg.geoRF(), &dp.regionFiles[fc]);
            }
#ifdef USE_GDAL
         } else if (dp.clipGDAL) {
               pRegionFile = new DgInGDALFile(dgg.geoRF(), &dp.regionFiles[fc]);
#endif
         } else {
               report("invalid dp.clip file parameters.", DgBase::Fatal);
         }

         DgInLocFile& regionFile = *pRegionFile;

         if (!regionFile.isPointFile()) {
            // read in each poly
            while (true) {
               DgPolygon v;
               regionFile >> v;
               if (regionFile.isEOF()) break;

               // add to the clipRegions
               processOneClipPoly(v, dp, dgg, clipRegions, pAttributeFile);
            }
         } else { // point file

            // read in each point and add to sets
            while (true) {

               DgLocVector v;
               regionFile >> v;
               if (regionFile.isEOF()) break;

               if (dp.megaVerbose) dgcout << "input: " << v << endl;

               dgg.convert(&v);
               for (int i = 0; i < v.size(); i++) {

                  const DgQ2DICoord& q2di = *dgg.getAddress(v[i]);
                  int q = q2di.quadNum();
                  const DgIVec2D& coord = q2di.coord();
                  clipRegions[q].setIsQuadUsed(true);

                  clipRegions[q].points().insert(coord);

                  //// add the attributes for this point
                  if (dp.buildShapeFileAttributes) {
                     const set<DgDBFfield>& fields =
                           pAttributeFile->curObjFields();
                     clipRegions[q].ptFields().insert(
                           pair<DgIVec2D, set<DgDBFfield> >(coord, fields));
                  }

                  //// update the i,j bounds for this quad

                  if (coord.i() < clipRegions[q].offset().i())
                     clipRegions[q].setOffset(
                        DgIVec2D(coord.i(), clipRegions[q].offset().j()));

                  if (coord.j() < clipRegions[q].offset().j())
                     clipRegions[q].setOffset(
                        DgIVec2D(clipRegions[q].offset().i(), coord.j()));

                  if (coord.i() > clipRegions[q].upperRight().i())
                     clipRegions[q].setUpperRight(
                      DgIVec2D(coord.i(), clipRegions[q].upperRight().j()));

                  if (coord.j() > clipRegions[q].upperRight().j())
                     clipRegions[q].setUpperRight(
                      DgIVec2D(clipRegions[q].upperRight().i(), coord.j()));
               }
            }
         }

         regionFile.close();
         delete pRegionFile;
      }
   } else if (dp.cellClip) {

      // check for valid state
      if (dgg.gridTopo() != dgg::topo::Hexagon)
          ::report("clip_subset_type of COARSE_CELLS only implemented for hexagon grids",
                   DgBase::Fatal);

      if (dp.clipCellRes >= dp.actualRes)
          ::report("clip_cell_res must be less than the grid resolution being generated",
                   DgBase::Fatal);

      if (dp.clipCellRes == 0)
          ::report("clip_cell_res must be greater than 0", DgBase::Fatal);

      // get the clipping cell dgg resolution
      const DgIDGGBase& clipDgg = dgg.dggs()->idggBase(dp.clipCellRes);

      // parse the input clipping cells
      vector<string> clipCellAddressStrs;
      dgg::util::ssplit(dp.clipCellsStr, clipCellAddressStrs);

//cout << "INRF: " << *dp.pInRF << endl;
//cout << "CLIPDGG: " << clipDgg << endl;
      // the coarse clipping cells
      // use a set to avoid duplicates
      set<unsigned long int> clipSeqNums;
      // parse the clip cell sequence numbers
      for (const auto &seqStr: clipCellAddressStrs) {

         unsigned long int sNum = 0;
         if (dp.inSeqNum) {
            if (sscanf(seqStr.c_str(), "%lu", &sNum) != 1)
               ::report("gridgen(): invalid cell sequence number in clip_cell_addresses" +
                         string(seqStr), DgBase::Fatal);
         } else { // must be indexToPoly
            // parse the address
            DgLocation* tmpLoc = NULL;
            tmpLoc = new DgLocation(*dp.pInRF);
            tmpLoc->fromString(seqStr.c_str(), dp.inputDelimiter);
            clipDgg.convert(tmpLoc);

            sNum = static_cast<const DgIDGGBase&>(clipDgg).bndRF().seqNum(*tmpLoc);
            delete tmpLoc;
         }

         clipSeqNums.insert(sNum);
//cout << " sNum: " << sNum << endl;
      }

      // add the cell boundaries to the clip regions
      for (set<unsigned long int>::iterator i = clipSeqNums.begin();
             i != clipSeqNums.end(); i++){

        DgLocation* loc = static_cast<const DgIDGG&>(clipDgg).bndRF().locFromSeqNum(*i);
//cout << " clip seq num " << *i << " " << *loc << endl;
        if (!clipDgg.bndRF().validLocation(*loc)) {
          dgcerr << "genGrid(): invalid clipping cell res: " << dp.clipCellRes
                    << " address: " << (*i)<< std::endl;
          ::report("genGrid(): Invalid clipping cell address found.", DgBase::Fatal);
        }

        DgPolygon verts(clipDgg);
        clipDgg.setVertices(*loc, verts, dp.nClipCellDensify);

        // add to the clipRegions
        processOneClipPoly(verts, dp, dgg, clipRegions, NULL);

        delete loc;
      }
   } else {
        ::report("genGrid(): Invalid clipping choices.", DgBase::Fatal);
   }

   //// adjust the bounds for boundary buffers if needed ////

   int skipVal = (dp.aperture == 3) ? 3 : 1;
   for (int q = 1; q < 11; q++) {
      if (!clipRegions[q].isQuadUsed()) continue;

      if (dp.megaVerbose) {
         dgcout << "POLY FINAL: ";
         dgcout << "q: " << q << " " << clipRegions[q].offset()
                   << " " << clipRegions[q].upperRight() << endl;
      }

      // create a buffer zone if applicable

      if (dp.gridTopo == Triangle || dp.gridTopo == Hexagon) {

         // build the adjusted minimum

         DgIVec2D newOff(clipRegions[q].offset().i() - skipVal,
                         clipRegions[q].offset().j() - skipVal);

         if (newOff.i() < 0) newOff = DgIVec2D(0, newOff.j());
         if (newOff.j() < 0) newOff = DgIVec2D(newOff.i(), 0);

         // build the adjusted upper right; track for quad overage

         DgIVec2D newUR(clipRegions[q].upperRight());

         if (newUR.i() <= dgg.maxI())
            newUR = DgIVec2D(newUR.i() + skipVal, newUR.j());

         if (newUR.j() <= dgg.maxJ())
            newUR = DgIVec2D(newUR.i(), newUR.j() + skipVal);

         if (newUR.i() > dgg.maxI()) {
            clipRegions[q].setOverI(true);
            newUR = DgIVec2D(dgg.maxI(), newUR.j());
         }

         if (newUR.j() > dgg.maxJ()) {
            clipRegions[q].setOverJ(true);
            newUR = DgIVec2D(newUR.i(), dgg.maxJ());
         }

         clipRegions[q].setUpperRight(newUR);
         clipRegions[q].setOffset(newOff);

         if (dp.megaVerbose) {
            dgcout << "AFTER ADJUSTMENT: ";
            dgcout << "q: " << q << " " << clipRegions[q].offset()
                << " " << clipRegions[q].upperRight() << endl;
         }
      }
   }

   //////// now do a quad at a time ///////

   // look for possible overage

   dp.nCellsTested = 0;
   dp.nCellsAccepted = 0;
   dp.nSamplePts = 0;

   const DgContCartRF& cc1 = dgg.ccFrame();
   const DgDiscRF2D& grid = dgg.grid2D();

   if (!dgg.isCongruent()) {
      for (int q = 1; q < 11; q++) {
         if (!clipRegions[q].isQuadUsed()) continue;

         if (dp.verbosity > 0)
            dgcout << "Checking OVERAGE quad " << q << endl;

         // check for over J

         if (clipRegions[q].overJ()) {
            DgIVec2D lLeft(clipRegions[q].offset());
            DgIVec2D uRight(clipRegions[q].upperRight());
            lLeft = DgIVec2D(lLeft.i(), dgg.maxJ() + 1);
            uRight = DgIVec2D(uRight.i(), dgg.maxJ() + 1);

            if (dp.megaVerbose)
               dgcout << "OVERJ in quad " << q << "-" << lLeft <<
                    " " << uRight << endl;

            DgIVec2D tCoord = lLeft;
            while (true) {
               DgIVec2D coord = tCoord;

               if (dp.buildShapeFileAttributes)
                  dp.curFields.clear();

               bool accepted = evalCell(dp, dgg, cc1, grid,
                                  clipRegions[q], coord);

               if (!accepted) {
                  if (tCoord == uRight) break;
                  tCoord.setI(tCoord.i() + 1);
                  continue;
               }

               // transfer to correct quad

               const DgQuadEdgeCells& ec = DgIDGGBase::edgeTable(q);
               int newQ = q;
               DgIVec2D newC(coord);
               if (ec.isType0()) {
                  if (coord.j() == (dgg.maxJ() + 1)) {
                     if (coord.i() == 0) {
                        newQ = ec.loneVert();
                        newC = DgIVec2D(0, 0);
                     } else {
                        newQ = ec.upQuad();
                        newC = DgIVec2D(0, (dgg.maxI() + 1) - coord.i());
                     }
                  } else { // i == (dgg.maxI() + 1)
                     newQ = ec.rightQuad();
                     newC.setI(0);
                  }
               } else {// type 1
                  if (coord.i() == (dgg.maxI() + 1)) {
                     if (coord.j() == 0) {
                        newQ = ec.loneVert();
                        newC = DgIVec2D(0, 0);
                     } else {
                        newQ = ec.rightQuad();
                        newC = DgIVec2D((dgg.maxJ() + 1) - coord.j(), 0);
                     }
                  } else { // j == (dgg.maxJ() + 1)
                     newQ = ec.upQuad();
                     newC.setJ(0);
                  }
               }

               // add to the overage set
               overageSet[newQ].insert(newC);
               if (dp.buildShapeFileAttributes)
                  overageFields[newQ].insert(
                    pair<DgIVec2D, set<DgDBFfield> >(newC, dp.curFields));

               if (dp.megaVerbose)
                  dgcout << "PUSH OVERAGE: " << q << ": " << coord <<
                       " -> " << newQ << ": " << newC << endl;

               if (tCoord == uRight) break;
               tCoord.setI(tCoord.i() + 1);
            }
         }

         // check for over I

         if (clipRegions[q].overI()) {
            DgIVec2D lLeft(clipRegions[q].offset());
            DgIVec2D uRight(clipRegions[q].upperRight());
            lLeft = DgIVec2D(dgg.maxI() + 1, lLeft.j());
            uRight = DgIVec2D(dgg.maxI() + 1, uRight.j());

            if (uRight.j() == dgg.maxJ() && clipRegions[q].overJ())
               uRight = DgIVec2D(uRight.i(), dgg.maxJ() + 1);

            if (dp.megaVerbose)
               dgcout << "OVERI in quad " << q << "-" << lLeft << " " << uRight << endl;

            DgBoundedRF2D b1(grid, lLeft, uRight);
            for (DgIVec2D tCoord = b1.lowerLeft(); tCoord != b1.invalidAdd();
                    tCoord = b1.incrementAddress(tCoord)) {
               DgIVec2D coord = tCoord;

               if (dp.buildShapeFileAttributes)
                  dp.curFields.clear();

               bool accepted = evalCell(dp, dgg, cc1, grid, clipRegions[q], coord);

               if (!accepted) continue;

               // transfer to correct quad

               const DgQuadEdgeCells& ec = DgIDGGBase::edgeTable(q);
               int newQ = q;
               DgIVec2D newC(coord);
               if (ec.isType0()) {
                  if (coord.j() == (dgg.maxJ() + 1)) {
                     if (coord.i() == 0) {
                        newQ = ec.loneVert();
                        newC = DgIVec2D(0, 0);
                     } else {
                        newQ = ec.upQuad();
                        newC = DgIVec2D(0, (dgg.maxI() + 1) - coord.i());
                     }
                  } else { // i == (dgg.maxI() + 1)
                     newQ = ec.rightQuad();
                     newC.setI(0);
                  }
               } else { // type 1
                  if (coord.i() == (dgg.maxI() + 1)) {
                     if (coord.j() == 0) {
                        newQ = ec.loneVert();
                        newC = DgIVec2D(0, 0);
                     } else {
                        newQ = ec.rightQuad();
                        newC = DgIVec2D((dgg.maxJ() + 1) - coord.j(), 0);
                     }
                  } else { // j == (dgg.maxJ() + 1)
                     newQ = ec.upQuad();
                     newC.setJ(0);
                  }
               }

               // add to the overage set
               overageSet[newQ].insert(newC);
               if (dp.buildShapeFileAttributes)
                  overageFields[newQ].insert(
                    pair<DgIVec2D, set<DgDBFfield> >(newC, dp.curFields));

               if (dp.megaVerbose)
                  dgcout << "ADD OVERAGE: " << q << ": " << coord <<
                          " -> " << newQ << ": " << newC << endl;
            }
         }
      }
   }

} // void createClipRegions

