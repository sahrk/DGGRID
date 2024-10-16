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
// SubOpGenHelper.cpp: helper functions for SubOpGen class
//
////////////////////////////////////////////////////////////////////////////////

// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL
#include <gdal.h>
#endif

#include <iostream>
#include <set>
#include <cstdlib>

#include "clipper.hpp"
#include <dglib/DgIVec2D.h>
#include <dglib/DgInputStream.h>
#include <dglib/DgInAIGenFile.h>
#include <dglib/DgInShapefile.h>
#include <dglib/DgInGdalFile.h>
#include <dglib/DgOutShapefile.h>
#include <dglib/DgInShapefileAtt.h>
#include <dglib/DgOutLocFile.h>
#include <dglib/DgOutKMLfile.h>
#include <dglib/DgOutGdalFile.h>
#include <dglib/DgOutPRPtsFile.h>
#include <dglib/DgOutPRCellsFile.h>
#include <dglib/DgOutNeighborsFile.h>
#include <dglib/DgOutChildrenFile.h>
#include <dglib/DgHexIDGG.h>
#include <dglib/DgHexIDGGS.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgIDGGSBase.h>
#include <dglib/DgBoundedIDGG.h>
#include <dglib/DgGeoSphRF.h>
#include <dglib/DgBoundedRF2D.h>
#include <dgaplib/DgApParamList.h>
#include <dglib/DgProjGnomonicRF.h>
#include <dglib/DgGeoProjConverter.h>
#include <dglib/DgCell.h>
#include <dglib/DgLocList.h>
#include <dglib/DgDmdD4Grid2D.h>
#include <dglib/DgDmdD4Grid2DS.h>
#include <dglib/DgTriGrid2D.h>
/*
#include <dglib/DgZOrderRF.h>
#include <dglib/DgZOrderStringRF.h>
#include <dglib/DgZ3RF.h>
#include <dglib/DgZ3StringRF.h>
#include <dglib/DgZ7RF.h>
#include <dglib/DgZ7StringRF.h>
*/
#include "DgHexSF.h"

//using namespace dgg::topo;

#include "OpBasic.h"
#include "SubOpGen.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void
SubOpGen::outputStatus (bool force)
{
   if (force || (op.outOp.nCellsTested &&
             (op.mainOp.updateFreq && (op.outOp.nCellsTested % op.mainOp.updateFreq == 0)))) {
      if (wholeEarth)
         dgcout << "* generated " << dgg::util::addCommas(op.outOp.nCellsAccepted)
              << " cells" << endl;
      else {
         dgcout << "accepted " << dgg::util::addCommas(op.outOp.nCellsAccepted)
              << " cells / ";
         dgcout << dgg::util::addCommas(op.outOp.nCellsTested) << " tested" << endl;
      }
   }

} // void SubOpGen::outputStatus

////////////////////////////////////////////////////////////////////////////////
bool
SubOpGen::evalCell (const DgIDGGBase& dgg, const DgContCartRF& cc1,
               const DgDiscRF2D& grid, DgQuadClipRegion& clipRegion,
               const DgIVec2D& add2D)
{
   // skip any sub-frequence cells
   if (!dgg.bndRF().bnd2D().validAddressPattern(add2D))
    return false;

   op.outOp.nCellsTested++;

   if (op.mainOp.megaVerbose)
      dgcout << "Testing #" << op.outOp.nCellsTested << ": " << add2D << endl;

   bool accepted = false;

   // start by checking the points
   set<DgIVec2D>::iterator it = clipRegion.points().find(add2D);
   if (it != clipRegion.points().end()) {

      accepted = true;
      clipRegion.points().erase(it);

      if (op.outOp.buildShapeFileAttributes) {

         // add the fields for this point
         map<DgIVec2D, set<DgDBFfield> >::iterator itFields =
                           clipRegion.ptFields().find(add2D);
         const set<DgDBFfield>& fields = itFields->second;
         for (set<DgDBFfield>::iterator it = fields.begin();
                it != fields.end(); it++)
            op.outOp.curFields.insert(*it);

         clipRegion.ptFields().erase(itFields);
      } else // only need one intersection
         return accepted;
   }

   // generate the boundary

   DgLocation* loc = grid.makeLocation(add2D);

   DgPolygon verts;
   grid.setVertices(add2D, verts);

   cc1.convert(loc);
   delete loc;

   cc1.convert(verts);

   // discard cells that don't meet poly-intersect clipping criteria if
   // applicable

   if (doPolyIntersect)
   {
      bool failure = true;

      // check against bounding box
      bool okminx = false;
      bool okmaxx = false;
      bool okminy = false;
      bool okmaxy = false;
      for (int i = 0; i < verts.size(); i++) {
         const DgDVec2D& p0 = *cc1.getAddress((verts)[i]);
         if (!okminx && p0.x() > clipRegion.minx()) okminx = true;
         if (!okminy && p0.y() > clipRegion.miny()) okminy = true;
         if (!okmaxx && p0.x() < clipRegion.maxx()) okmaxx = true;
         if (!okmaxy && p0.y() < clipRegion.maxy()) okmaxy = true;
      }

      if (!(okminx && okmaxx) || !(okminy && okmaxy)) {
         accepted = false;
      } else {
         ClipperLib::Paths cellPoly(1);
         for (int i = 0; i < verts.size(); i++)
           cellPoly[0] <<
              ClipperLib::IntPoint(clipperFactor * cc1.getAddress((verts)[i])->x(),
                                   clipperFactor * cc1.getAddress((verts)[i])->y());

         for (unsigned int i = 0; i < clipRegion.clpPolys().size(); i++) {

           ClipperLib::Clipper c;
           c.AddPaths(cellPoly, ClipperLib::ptSubject, true);
           c.AddPaths(clipRegion.clpPolys()[i].exterior, ClipperLib::ptClip, true);

           ClipperLib::Paths solution;
           c.Execute(ClipperLib::ctIntersection, solution, ClipperLib::pftNonZero,
                     ClipperLib::pftNonZero);

           if (solution.size() != 0) {
              accepted = true; // a hole may make this false
#ifdef USE_GDAL
              if (useHoles) {
                 const int numHoles = (int) clipRegion.clpPolys()[i].holes.size();
                 if (numHoles > 0) {

                    // assume hole on the snyder quad are most likely
                    OGRPolygon* snyderHex = DgOutGdalFile::createPolygon(verts);
                    // lazy instantiate the quad gnomonic version if needed
                    OGRPolygon* gnomHex = NULL;

                    for (int h = 0; h < numHoles; h++) {
                       DgClippingHole clipHole = clipRegion.clpPolys()[i].holes[h];

                       // need to choose correct projection; assume snyder hole
                       const OGRPolygon* hex = snyderHex;
                       if (clipHole.isGnomonic) {
                          // lazy instantiate gnomHex
                          if (!gnomHex) {
                             DgLocation* tLoc = dgg.makeLocation(
                                  DgQ2DICoord(clipRegion.quadNum(), add2D));
                             DgPolygon gHex(dgg);
                             dgg.setVertices(*tLoc, gHex,
                                ((op.outOp.nDensify > 1) ? op.outOp.nDensify : 1));
                             delete tLoc;

                             clipRegion.gnomProj().convert(&gHex);
                             gnomHex = DgOutGdalFile::createPolygon(gHex);
                          }
                          hex = gnomHex;
                       }

                       // check if the hole contains the hex
                       if (clipHole.hole.Contains(hex)) {
                          accepted = false;
                          break;
                       }
                    }

                    delete snyderHex;
                    if (gnomHex) delete gnomHex;
                 }
              }
           }

           if (accepted) {
#endif
              failure  = false;
              if (op.outOp.buildShapeFileAttributes) {
                 // add the fields for this polygon
                 const set<DgDBFfield>& fields = clipRegion.polyFields()[i];
                 for (set<DgDBFfield>::iterator it = fields.begin();
                          it != fields.end(); it++)
                   op.outOp.curFields.insert(*it);
              } else { // only need one intersection
                 goto EVALCELL_FINISH;
              }
           }
        }
     }

     // If we are here, we did not fail:
     failure = false;

     goto EVALCELL_FINISH;

EVALCELL_FINISH:

     if (failure)
        throw "Out of memory in evalCell()";
   }

   return accepted;

} // bool SubOpGen::evalCell

////////////////////////////////////////////////////////////////////////////////
bool
SubOpGen::evalCell (DgEvalData* data, DgIVec2D& add2D)
{
   if (op.outOp.buildShapeFileAttributes)
      op.outOp.curFields.clear();

   bool accepted = false;

   if (!data->overageSet.empty())
   {
      set<DgIVec2D>::iterator it = data->overageSet.find(add2D);
      if (it != data->overageSet.end()) // found add2D
      {
         accepted = true;

         data->overageSet.erase(it);

         if (op.outOp.buildShapeFileAttributes)
         {
            // add the fields for this polygon

            map<DgIVec2D, set<DgDBFfield> >::iterator itFields =
                              data->overageFields.find(add2D);
            const set<DgDBFfield>& fields = itFields->second;
            for (set<DgDBFfield>::iterator it = fields.begin();
                   it != fields.end(); it++)
               op.outOp.curFields.insert(*it);

            data->overageFields.erase(itFields);
         }
         else // only need one intersection
            return true;
      }
   }

   if (op.outOp.buildShapeFileAttributes && accepted)
   {
      evalCell(data->dgg, data->cc1, data->grid,
                      data->clipRegion, add2D);
      return true;
   }

   if (add2D.i() < data->lLeft.i() || add2D.i() > data->uRight.i() ||
       add2D.j() < data->lLeft.j() || add2D.j() > data->uRight.j())
      return false;
   else
      return evalCell(data->dgg, data->cc1, data->grid,
                      data->clipRegion, add2D);

} // bool SubOpGen::evalCell

////////////////////////////////////////////////////////////////////////////////
int
SubOpGen::executeOp (void)
{
   const DgIDGGBase& dgg = op.dggOp.dgg();
   const DgIDGGSBase* dggs = dgg.dggs();

   // set-up the input reference frame
   // this should be moved as much as possible to SubOpIn
   const DgRFBase* chdRF = nullptr;
   if (cellClip && op.inOp.inAddType != dgg::addtype::SeqNum) {
      if (clipCellRes < 0 || clipCellRes > op.dggOp.actualRes)
         ::report("genGrid(): invalid clipCellRes", DgBase::Fatal);

      op.inOp.inSeqNum = op.dggOp.addressTypeToRF(op.inOp.inAddType, &op.inOp.pInRF, &chdRF, clipCellRes);

   } else {
      op.inOp.inSeqNum = op.dggOp.addressTypeToRF(op.inOp.inAddType, &op.inOp.pInRF, &chdRF);
   }

   if (!op.inOp.pInRF)
      ::report("genGrid(): invalid input RF", DgBase::Fatal);

   // output setup in the SubOpOut object

   ////// do applicable clipping mode /////

   char delimStr[2];
   delimStr[0] = op.inOp.inputDelimiter;
   delimStr[1] = '\0';
   const int maxLine = 1000;
   char buff[maxLine];
   if (seqToPoly || indexToPoly) {
      op.outOp.nCellsAccepted = 0;
      op.outOp.nCellsTested = 0;

      // convert any incoming addresses to seqnums
      // use a set to ensure each cell is printed only once
      set<unsigned long int> seqnums;

      // read-in the sequence numbers
      for (const auto &regionfile: regionFiles) {
         DgInputStream fin(regionfile.c_str(), "", DgBase::Fatal);

         while (1) {
            op.outOp.nCellsTested++;

            // get the next line
            fin.getline(buff, maxLine);
            if (fin.eof()) break;

            unsigned long int sNum = 0;
            if (seqToPoly) {
              if (sscanf(buff, "%lu", &sNum) != 1)
                 ::report("genGrid(): invalid SEQNUM " + string(buff), DgBase::Fatal);
            } else { // must be indexToPoly
               // parse the address
               DgLocation* tmpLoc = NULL;
               tmpLoc = new DgLocation(*op.inOp.pInRF);
               tmpLoc->fromString(buff, op.inOp.inputDelimiter);
               dgg.convert(tmpLoc);

               sNum = static_cast<const DgIDGGBase&>(dgg).bndRF().seqNum(*tmpLoc);
               delete tmpLoc;
            }

            seqnums.insert(sNum);
         }

         fin.close();
      }

      // generate the cells
      for (set<unsigned long int>::iterator i=seqnums.begin();i!=seqnums.end();i++) {

        DgLocation* loc = static_cast<const DgIDGG&>(dgg).bndRF().locFromSeqNum(*i);
        if (!dgg.bndRF().validLocation(*loc)){
          dgcerr<<"genGrid(): SEQNUM " << (*i)<< " is not a valid location"<<std::endl;
          ::report("genGrid(): Invalid SEQNUM found.", DgBase::Fatal);
        }

        op.outOp.nCellsAccepted++;
        outputStatus();

        op.outOp.outputCellAdd2D(*loc);

        delete loc;
      }
/*
   } else if (pointClip) {
      op.outOp.nCellsAccepted = 0;

      set<DgQ2DICoord> cells; // To ensure each cell is printed only output once

      // read-in and bin the points
      for (unsigned long fc = 0; fc < regionFiles.size(); fc++) {
         DgInLocFile* pRegionFile = new DgInAIGenFile(dgg.geoRF(), &regionFiles[fc]);
         DgInLocFile& regionFile = *pRegionFile;

         DgLocList points;
         regionFile >> points;
         regionFile.close();
         delete pRegionFile;

         if (op.mainOp.megaVerbose) dgcout << "input: " << points << endl;

         dgg.convert(&points);

         if (op.mainOp.megaVerbose) dgcout << " -> " << points << endl;

         list<DgLocBase*>::const_iterator it;
         for (it = points.begin(); it != points.end(); it++) {
           DgQ2DICoord q2di = *(dgg.getAddress(*(static_cast<const DgLocation*>(*it))));
           cells.insert(q2di);
         }
      }

      // generate the cells
      for(set<DgQ2DICoord>::iterator i=cells.begin(); i!=cells.end(); i++){
        DgLocation* loc = dgg.makeLocation(*i);

        op.outOp.nCellsTested++;
        op.outOp.nCellsAccepted++;
        outputStatus();

        op.outOp.outputCellAdd2D(*loc);

        delete loc;
      }
*/
   } else if (wholeEarth) {
      op.outOp.nCellsAccepted = 0;
      op.outOp.nCellsTested = 0;
      if (!op.dggOp.isSuperfund)
      {
         unsigned long long int startCell = op.outOp.outFirstSeqNum - 1;
         //unsigned long long int startCell = op.outOp.maxCellsPerFile * (op.outOp.outFirstSeqNum - 1);
         DgLocation* addLoc = new DgLocation(dgg.bndRF().first());
         bool more = true;

         // skip cells up to the first desired output file; this needs to be done
         // with seqNum to address converters
         while (op.outOp.nCellsTested < startCell)
         {
            //op.outOp.nCellsAccepted++;
            op.outOp.nCellsTested++;
            outputStatus();

            dgg.bndRF().incrementLocation(*addLoc);
            if (!dgg.bndRF().validLocation(*addLoc))
            {
               more = false;
               break;
            }
         }

         if (more)
         {
            while (op.outOp.nCellsTested < op.outOp.outLastSeqNum)
            {
               op.outOp.nCellsAccepted++;
               op.outOp.nCellsTested++;
               outputStatus();

               op.outOp.outputCellAdd2D(*addLoc);

               dgg.bndRF().incrementLocation(*addLoc);
               if (!dgg.bndRF().validLocation(*addLoc)) break;
            }
         }
         delete addLoc;
      }
      else { // isSuperfund
         for (int q = 0; q < 12; q++) {
            DgHexSF baseTile(*this, 0, 0, 0, 0, true, q);
            baseTile.setType('P');
            baseTile.depthFirstTraversal(*dggs, dgg, op.dggOp.deg(), 2);
         }
      }
   } else { // use clip regions

// USE_GDAL is set in MakeIncludes
#ifdef USE_GDAL
      if (clipGDAL) {
        report("Registering GDAL drivers...", DgBase::Info);
        GDALAllRegister();
      }
#endif

      DgQuadClipRegion clipRegions[12]; // clip regions for each quad
      set<DgIVec2D> overageSet[12];     // overage sets
      map<DgIVec2D, set<DgDBFfield> > overageFields[12]; // associated fields

      try {
         createClipRegions(dgg, clipRegions, overageSet, overageFields);
      } catch (ClipperLib::clipperException& e) {
         dgcerr << "ERROR: a clipping polygon vertex exceeds the range for the clipping library.\n";
         report("Try reducing the value of parameter clipper_scale_factor and/or breaking-up large clipping polygons.", DgBase::Fatal);
      }

      if (op.outOp.buildShapeFileAttributes)
      {
         if (op.outOp.outCellAttributes)
            op.outOp.cellOutShp->addFields(op.outOp.allFields);

         if (op.outOp.outPointAttributes)
            op.outOp.ptOutShp->addFields(op.outOp.allFields);
      }

      //// now process the cells by quad ////

      const DgContCartRF& cc1 = dgg.ccFrame();
      const DgDiscRF2D& grid = dgg.grid2D();

      dgcout << "\n";
      for (int q = 0; q < 12; q++)
      {
         if (overageSet[q].empty() && !clipRegions[q].isQuadUsed())
         {
            dgcout << string("* No intersections in quad ")
                 << dgg::util::to_string(q) << "." << endl;
            continue;
         }

         dgcout << string("* Testing quad ") << dgg::util::to_string(q)
              << "... " << endl;

         if (op.mainOp.megaVerbose)
            dgcout << "Generating: " << q << " " << clipRegions[q].offset()
                 << " " << clipRegions[q].upperRight() << endl;

         DgIVec2D lLeft;
         DgIVec2D uRight;

         if (clipRegions[q].isQuadUsed())
         {
            lLeft = clipRegions[q].offset();
            uRight = clipRegions[q].upperRight();
         }

         // assume isSuperfund
         if (op.dggOp.isSuperfund)
         {
            DgEvalData ed(dgg, cc1, grid, clipRegions[q], overageSet[q],
                          overageFields[q], op.dggOp.deg(), lLeft, uRight);

            DgHexSF baseTile(*this, 0, 0, 0, 0, true, q);
            baseTile.setType('P');
            baseTile.depthFirstTraversal(*dggs, dgg, op.dggOp.deg(), 2, &ed);
         }
         else // !isSuperfund
         {
            DgBoundedRF2D b1(grid, DgIVec2D(0, 0), (uRight - lLeft));
            DgIVec2D tCoord = lLeft; // where are we on the grid?
            while (!overageSet[q].empty() || clipRegions[q].isQuadUsed())
            {
               DgIVec2D coord = tCoord;
               bool accepted = false;

               // first check if there are cells on the overage set

               if (!overageSet[q].empty())
               {
                  if (clipRegions[q].isQuadUsed())
                  {
                     set<DgIVec2D>::iterator it = overageSet[q].find(tCoord);
                     if (it != overageSet[q].end()) // found tCoord
                     {
                        accepted = true;
                        overageSet[q].erase(it);
                        if (op.mainOp.megaVerbose) dgcout << "found OVERAGE coord " << coord << endl;

                        tCoord -= lLeft;
                        tCoord = b1.incrementAddress(tCoord);
                        if (tCoord == b1.invalidAdd())
                           clipRegions[q].setIsQuadUsed(false);

                        tCoord += lLeft;
                     }
                     else
                     {
                        set<DgIVec2D>::iterator it = overageSet[q].begin();
                        if (*it < tCoord)
                        {
                           accepted = true;
                           coord = *it;
                           overageSet[q].erase(it);
                           if (op.mainOp.megaVerbose) dgcout << "processing OVERAGE " << coord << endl;
                        }
                        else
                        {
                           tCoord -= lLeft;
                           tCoord = b1.incrementAddress(tCoord);
                           if (tCoord == b1.invalidAdd())
                              clipRegions[q].setIsQuadUsed(false);

                           tCoord += lLeft;
                        }
                     }
                  }
                  else
                  {
                     set<DgIVec2D>::iterator it = overageSet[q].begin();
                     coord = *it;
                     overageSet[q].erase(it);
                     accepted = true;
                     if (op.mainOp.megaVerbose) dgcout << "processing OVERAGE " << coord << endl;
                  }
               }
               else if (clipRegions[q].isQuadUsed())
               {
                  tCoord -= lLeft;
                  tCoord = b1.incrementAddress(tCoord);
                  if (tCoord == b1.invalidAdd())
                     clipRegions[q].setIsQuadUsed(false);

                  tCoord += lLeft;
               }

               // skip subfrequency cells as appropriate if doing classII
               // (this should all be done using the seqNum methods, would be
               // much cleaner)

               if (op.mainOp.megaVerbose) {
                  dgcout << "# testing coord: " << coord << endl;
                  //dgcout << "DGG: " << dgg << endl;
               }
/* this gets checked in evalCell
                if (op.dggOp.gridTopo == Hexagon && !dgg.isClassI()) {
                    printf("HERE\n");
                    fflush(stdout);
                    if ((coord.j() + coord.i()) % 3) continue;
                }
 */

               outputStatus();

               if (!accepted)
                  accepted = evalCell(dgg, cc1, grid, clipRegions[q], coord);

               if (!accepted) continue;

               // if we're here we have a good one

               op.outOp.nCellsAccepted++;
               //cout << "XX " << q << " " << coord << endl;

               DgLocation* addLoc = dgg.makeLocation(DgQ2DICoord(q, coord));
               op.outOp.outputCellAdd2D(*addLoc);
	           delete addLoc;

               // check for special cases
               if (q == 0 || q == 11) break; // only one cell
            }
         } // else !dp.isSuperfund

         dgcout << "...quad " << q << " complete." << endl;
      }

   } // end if wholeEarth else

   dgcout << "\n** grid generation complete **" << endl;
   outputStatus(true);
   if (!wholeEarth && !seqToPoly && !indexToPoly)
      dgcout << "acceptance rate is " <<
          100.0 * (long double) op.outOp.nCellsAccepted / (long double) op.outOp.nCellsTested <<
          "%" << endl;

   dgcout << endl;

   return 0;

} // int SubOpGen::executeOp

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
ClipperLib::Paths*
SubOpGen::intersectPolyWithQuad (const DgPolygon& v, DgQuadClipRegion& clipRegion)
//
// caller takes responsibility for the returned memory
//
{
   //// create a local copy and project to gnomonic
   DgPolygon polyVec(v);
   if (op.mainOp.megaVerbose) dgcout << "input polyVec(v): " << polyVec << endl;
   clipRegion.gnomProj().convert(polyVec);
   if (op.mainOp.megaVerbose) dgcout << " -> input: " << polyVec << endl;

   //// now create a clipper poly version
   ClipperLib::Paths clpPoly(1);

   for (int i = 0; i < polyVec.size(); i++) {
      const DgDVec2D& p0 = *clipRegion.gnomProj().getAddress(polyVec[i]);

      if (op.mainOp.megaVerbose) dgcout << "clipper: \n" << " i: " << i << " " << p0 << endl;

      clpPoly[0] <<
           ClipperLib::IntPoint(clipperFactor * p0.x(), clipperFactor * p0.y());
   }

   //// find the intersections

   ClipperLib::Clipper c;
   c.AddPaths(clpPoly, ClipperLib::ptSubject, true);
   c.AddPaths(clipRegion.gnomBndry(), ClipperLib::ptClip, true);

   ClipperLib::Paths* solution = new ClipperLib::Paths();
   c.Execute(ClipperLib::ctIntersection, *solution,
                  ClipperLib::pftNonZero, ClipperLib::pftNonZero);

   return solution;
} // ClipperLib::Paths* SubOpGen::intersectPolyWithQuad

//////////////////////////////////////////////////////////////////////////////
void
SubOpGen::processOneClipPoly (DgPolygon& polyIn, const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], DgInShapefileAtt* pAttributeFile)
{
   if (op.mainOp.megaVerbose) dgcout << "processOneClipPoly input: " << polyIn << endl;
   dgg.geoRF().convert(polyIn);
   if (op.mainOp.megaVerbose) dgcout << "-> geoRF: " << polyIn << endl;

   if (geoDens > 0.000000000001)
      DgGeoSphRF::densify(polyIn, geoDens);

   if (op.mainOp.megaVerbose) dgcout << "densified: " << polyIn << endl;

   // create a copy to test for intersected quads

   DgPolygon quadVec(polyIn);

   if (op.mainOp.megaVerbose) dgcout << "quadVec(polyIn): " << quadVec << endl;

   dgg.q2ddRF().convert(quadVec);

   if (op.mainOp.megaVerbose) dgcout << "->: " << quadVec << endl;

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
            if (op.mainOp.megaVerbose) dgcout << "intersects quad " << q << endl;
      }
   }

#ifdef USE_GDAL
   int numHoles = 0;
   int** holeQuads = NULL;
   if (useHoles) {
      //// determine which holes have vertices on which quads
      numHoles = (int) polyIn.holes().size();
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
   //int nQuadsInt = 0;
   for (int q = 1; q < 11; q++) {

      if (op.mainOp.megaVerbose) dgcout << "intersecting with quads..." << endl;

      if (!quadInt[q]) continue;

      if (op.mainOp.megaVerbose) dgcout << "INTERSECTING quad " << q << endl;

      if (op.mainOp.megaVerbose) dgcout << "input: " << polyIn << endl;

      ClipperLib::Paths* solution =
                intersectPolyWithQuad (polyIn, clipRegions[q]);

      if (solution->size() == 0) {
         if (op.mainOp.megaVerbose)
            dgcout << "no intersection in quad " << q << endl;

         continue;
      }

      // if we're here we have intersection(s)
      if (op.mainOp.megaVerbose)
         dgcout << solution->size() << " intersections FOUND in quad " << q << endl;

      //nQuadsInt++;
      clipRegions[q].setIsQuadUsed(true);

      ////// now convert back to Snyder and add to the clipRegions
      DgQuadClipRegion& cr = clipRegions[q];
      for (size_t i = 0; i < solution->size(); i++) {

         DgPolygon locv(cr.gnomProj());
         for (size_t j = 0; j < (*solution)[i].size(); j++) {
            DgDVec2D p0 = DgDVec2D(invClipperFactor * (*solution)[i][j].X,
                        invClipperFactor * (*solution)[i][j].Y);
            DgLocation* tloc = cr.gnomProj().makeLocation(p0);

            locv.push_back(*tloc);
            delete tloc;
         }

         if (op.mainOp.megaVerbose) dgcout << "locv: " << locv << endl;

         dgg.geoRF().convert(locv);

         if (op.mainOp.megaVerbose) dgcout << "->" << locv << endl;

         dgg.q2ddRF().convert(locv);

         if (op.mainOp.megaVerbose) dgcout << "->" << locv << endl;

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

            cfinVerts[0] << ClipperLib::IntPoint(clipperFactor * p0.x(),
                                          clipperFactor * p0.y());
         }

         // start building the clipping poly definition
         DgClippingPoly clipPoly;
         clipPoly.exterior = cfinVerts;

#ifdef USE_GDAL
         if (useHoles) {
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
         if (op.outOp.buildShapeFileAttributes) {
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
      if (useHoles) {
         for (int h = 0; h < numHoles; h++)
            delete [] holeQuads[h];
         delete [] holeQuads;
      }
#endif

} // void SubOpGen::processOneClipPoly

//////////////////////////////////////////////////////////////////////////////
void
SubOpGen::createClipRegions (const DgIDGGBase& dgg,
             DgQuadClipRegion clipRegions[], set<DgIVec2D> overageSet[],
             map<DgIVec2D, set<DgDBFfield> > overageFields[])
{
   dgcout << "\n* building clipping regions..." << endl;

   // initialize the i,j bounds
   for (int q = 1; q < 11; q++) {
      clipRegions[q].setQuadNum(q);
      clipRegions[q].setOffset(DgIVec2D(dgg.maxD(), dgg.maxJ()));
      clipRegions[q].setUpperRight(DgIVec2D(0, 0));
   }

   // create the gnomonic quad boundaries to clip against

   if (op.mainOp.megaVerbose)
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

   if (op.mainOp.megaVerbose) dgcout << "verts:\n" << verts << endl;

   // lower left

   DgDVec2D p = *ccRF.getAddress(verts[0]);
   p += DgDVec2D(0.0, nudge);
   DgLocation* tloc = ccRF.makeLocation(p);
   verts.setLoc(0, *tloc);
   delete tloc;

   // lower right

   p = *ccRF.getAddress(verts[1]);
   p += DgDVec2D(-2.0 * nudge, nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(1, *tloc);
   delete tloc;

   // upper right

   p = *ccRF.getAddress(verts[2]);
   p += DgDVec2D(0.0, -nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(2, *tloc);
   delete tloc;

   // upper left

   p = *ccRF.getAddress(verts[3]);
   p += DgDVec2D(2.0 * nudge, -nudge);
   tloc = ccRF.makeLocation(p);
   verts.setLoc(3, *tloc);
   delete tloc;

   if (op.mainOp.megaVerbose) dgcout << "(nudge) ->" << verts << endl;

   for (int q = 1; q < 11; q++) {

      if (op.mainOp.megaVerbose) dgcout << "quad " << q << endl;

      // find gnomonic projection center for this quad

      tloc = dgg.q2ddRF().makeLocation(
                            DgQ2DDCoord(q, *ccRF.getAddress(cent)));

      if (op.mainOp.megaVerbose) dgcout << "center: " << *tloc;

      dgg.geoRF().convert(tloc);

      if (op.mainOp.megaVerbose) dgcout << " -> " << *tloc << endl;

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

      if (op.mainOp.megaVerbose)
	  dgcout << v0 << endl;

      dgg.geoRF().convert(v0);

      if (op.mainOp.megaVerbose)
	  dgcout << " -> " << v0 << endl;

      clipRegions[q].gnomProj().convert(v0);

      if (op.mainOp.megaVerbose)
	  dgcout << " -> " << v0 << endl;

      // finally store the boundary as a clipper polygon
      ClipperLib::Path contour;
      for (int i = 0; i < v0.size(); i++) {
         const DgDVec2D& p0 = *clipRegions[q].gnomProj().getAddress(v0[i]);
         contour << ClipperLib::IntPoint( clipperFactor*p0.x() , clipperFactor*p0.y() );
      }

      clipRegions[q].gnomBndry().push_back(contour);
   }

   // load the clipping polygons or points
   if (regionClip || pointClip) {

      //// read in the region boundary files
      for (unsigned long fc = 0; fc < regionFiles.size(); fc++) {

         DgInLocFile* pRegionFile = NULL;
         DgInShapefileAtt* pAttributeFile = NULL;
         if (clipAIGen)
            pRegionFile = new DgInAIGenFile(dgg.geoRF(), &regionFiles[fc]);
         else if (clipShape) {
            if (op.outOp.buildShapeFileAttributes) {

               pRegionFile = pAttributeFile =
                     new DgInShapefileAtt(dgg.geoRF(), &regionFiles[fc]);

               // add any new fields to the global list
               const set<DgDBFfield>& fields = pAttributeFile->fields();
               for (set<DgDBFfield>::iterator it = fields.begin();
                    it != fields.end(); it++) {
                  if (it->fieldName() == "global_id") continue;

                  set<DgDBFfield>::iterator cur = op.outOp.allFields.find(*it);
                  if (cur == op.outOp.allFields.end()) // new field
                     op.outOp.allFields.insert(*it);
                  else if (cur->type() != it->type())
                     report("input files contain incompatible definitions "
                            "of attribute field " + it->fieldName(), DgBase::Fatal);
               }
            } else {
               pRegionFile = new DgInShapefile(dgg.geoRF(), &regionFiles[fc]);
            }
#ifdef USE_GDAL
         } else if (clipGDAL) {
               //pRegionFile = new DgInGdalFile(dgg.geoRF(), &regionFiles[fc]);
               pRegionFile = new DgInGdalFile(op.dggOp.deg(), &regionFiles[fc]);
#endif
         } else {
               report("invalid clip file parameters.", DgBase::Fatal);
         }

         DgInLocFile& regionFile = *pRegionFile;

         if (!regionFile.isPointFile()) {
            // read in each poly
            while (true) {
               DgPolygon v;
               regionFile >> v;
               if (regionFile.isEOF()) break;

               // add to the clipRegions
               processOneClipPoly(v, dgg, clipRegions, pAttributeFile);
            }
         } else { // point file

            // read in each point and add to sets
            while (true) {

               DgLocVector v;
               regionFile >> v;
               if (regionFile.isEOF()) break;

               if (op.mainOp.megaVerbose) dgcout << "input: " << v << endl;

               dgg.convert(&v);
               for (int i = 0; i < v.size(); i++) {

                  const DgQ2DICoord& q2di = *dgg.getAddress(v[i]);
                  int q = q2di.quadNum();
                  const DgIVec2D& coord = q2di.coord();
                  clipRegions[q].setIsQuadUsed(true);

                  clipRegions[q].points().insert(coord);

                  //// add the attributes for this point
                  if (op.outOp.buildShapeFileAttributes) {
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
   } else if (cellClip) {

      // check for valid state
      if (dgg.gridTopo() != dgg::topo::Hexagon)
          ::report("clip_subset_type of COARSE_CELLS only implemented for hexagon grids",
                   DgBase::Fatal);

      if (clipCellRes >= op.dggOp.actualRes)
          ::report("clip_cell_res must be less than the grid resolution being generated",
                   DgBase::Fatal);

      if (clipCellRes == 0)
          ::report("clip_cell_res must be greater than 0", DgBase::Fatal);

      // get the clipping cell dgg resolution
      const DgIDGGBase& clipDgg = dgg.dggs()->idggBase(clipCellRes);

      // parse the input clipping cells
      vector<string> clipCellAddressStrs;
      dgg::util::ssplit(clipCellsStr, clipCellAddressStrs);

//cout << "INRF: " << *op.inOp.pInRF << endl;
//cout << "CLIPDGG: " << clipDgg << endl;
      // the coarse clipping cells
      // use a set to avoid duplicates
      set<unsigned long int> clipSeqNums;
      // parse the clip cell sequence numbers
      for (const auto &seqStr: clipCellAddressStrs) {

         unsigned long int sNum = 0;
         if (op.inOp.inSeqNum) {
            if (sscanf(seqStr.c_str(), "%lu", &sNum) != 1)
               ::report("gridgen(): invalid cell sequence number in clip_cell_addresses" +
                         string(seqStr), DgBase::Fatal);
         } else { // must be indexToPoly
            // parse the address
            DgLocation* tmpLoc = NULL;
            tmpLoc = new DgLocation(*op.inOp.pInRF);
            tmpLoc->fromString(seqStr.c_str(), op.inOp.inputDelimiter);
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
          dgcerr << "genGrid(): invalid clipping cell res: " << clipCellRes
                    << " address: " << (*i)<< std::endl;
          ::report("genGrid(): Invalid clipping cell address found.", DgBase::Fatal);
        }

        DgPolygon verts(clipDgg);
        clipDgg.setVertices(*loc, verts, nClipCellDensify);

        // add to the clipRegions
        processOneClipPoly(verts, dgg, clipRegions, NULL);

        delete loc;
      }
   } else {
        ::report("genGrid(): Invalid clipping choices.", DgBase::Fatal);
   }

   //// adjust the bounds for boundary buffers if needed ////

   int skipVal = (op.dggOp.aperture == 3) ? 3 : 1;
   for (int q = 1; q < 11; q++) {
      if (!clipRegions[q].isQuadUsed()) continue;

      if (op.mainOp.megaVerbose) {
         dgcout << "POLY FINAL: ";
         dgcout << "q: " << q << " " << clipRegions[q].offset()
                   << " " << clipRegions[q].upperRight() << endl;
      }

      // create a buffer zone if applicable

      if (op.dggOp.gridTopo == Triangle || op.dggOp.gridTopo == Hexagon) {

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

         if (op.mainOp.megaVerbose) {
            dgcout << "AFTER ADJUSTMENT: ";
            dgcout << "q: " << q << " " << clipRegions[q].offset()
                << " " << clipRegions[q].upperRight() << endl;
         }
      }
   }

   //////// now do a quad at a time ///////

   // look for possible overage

   op.outOp.nCellsTested = 0;
   op.outOp.nCellsAccepted = 0;
   op.dggOp.nSamplePts = 0;

   const DgContCartRF& cc1 = dgg.ccFrame();
   const DgDiscRF2D& grid = dgg.grid2D();

   if (!dgg.isCongruent()) {
      for (int q = 1; q < 11; q++) {
         if (!clipRegions[q].isQuadUsed()) continue;

         if (op.mainOp.verbosity > 0)
            dgcout << "Checking OVERAGE quad " << q << endl;

         // check for over J

         if (clipRegions[q].overJ()) {
            DgIVec2D lLeft(clipRegions[q].offset());
            DgIVec2D uRight(clipRegions[q].upperRight());
            lLeft = DgIVec2D(lLeft.i(), dgg.maxJ() + 1);
            uRight = DgIVec2D(uRight.i(), dgg.maxJ() + 1);

            if (op.mainOp.megaVerbose)
               dgcout << "OVERJ in quad " << q << "-" << lLeft <<
                    " " << uRight << endl;

            DgIVec2D tCoord = lLeft;
            while (true) {
               DgIVec2D coord = tCoord;

               if (op.outOp.buildShapeFileAttributes)
                  op.outOp.curFields.clear();

               bool accepted = evalCell(dgg, cc1, grid,
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
               if (op.outOp.buildShapeFileAttributes)
                  overageFields[newQ].insert(
                    pair<DgIVec2D, set<DgDBFfield> >(newC, op.outOp.curFields));

               if (op.mainOp.megaVerbose)
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

            if (op.mainOp.megaVerbose)
               dgcout << "OVERI in quad " << q << "-" << lLeft << " " << uRight << endl;

            DgBoundedRF2D b1(grid, lLeft, uRight);
            for (DgIVec2D tCoord = b1.lowerLeft(); tCoord != b1.invalidAdd();
                    tCoord = b1.incrementAddress(tCoord)) {
               DgIVec2D coord = tCoord;

               if (op.outOp.buildShapeFileAttributes)
                  op.outOp.curFields.clear();

               bool accepted = evalCell(dgg, cc1, grid, clipRegions[q], coord);

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
               if (op.outOp.buildShapeFileAttributes)
                  overageFields[newQ].insert(
                    pair<DgIVec2D, set<DgDBFfield> >(newC, op.outOp.curFields));

               if (op.mainOp.megaVerbose)
                  dgcout << "ADD OVERAGE: " << q << ": " << coord <<
                          " -> " << newQ << ": " << newC << endl;
            }
         }
      }
   }

} // void SubOpGen::createClipRegions

////////////////////////////////////////////////////////////////////////////////
