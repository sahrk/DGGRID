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
// DgHexSF.cpp: DgHexSF class implementation
//
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <dglib/DgBase.h>
#include "DgHexSF.h"
#include "dggrid.h"
#include <dglib/DgIDGGBase.h>
#include "gridgen.h"

////////////////////////////////////////////////////////////////////////////////

// child sets for each type of descendant in 4*[33r]*
// does NOT handle 4 -> 3r (and 3r -> 3r?) transition
const string DgHexSF::all = "AAAAAAA7";
const string DgHexSF::cs4 = "A1A3B5B7";
const string DgHexSF::cs3A = "A123F5D7";
const string DgHexSF::cs3B = "E123C5B7";
const string DgHexSF::cs3rA = "A123B5B7";
const string DgHexSF::cs3rB = "A123BA67";
const string DgHexSF::cs3rC = "BA234B67";
const string DgHexSF::cs3rD = "BA2A4567";
const string DgHexSF::cs3rE = "B1BA4567";
const string DgHexSF::cs3rF = "A1A345B7";

unsigned long long int
DgHexSF::visitMe (GridGenParam& dp, const DgIDGGSBase& dggs, const DgIDGGBase& dgg,
                  const DgContCartRF& deg, DgEvalData* ed)
{
   DgIVec2D coord2d;
   if (classI_)
      coord2d = (DgIVec2D) this->ijkCoord();
   else
      coord2d = (DgIVec2D) downAp3r().ijkCoord();

//cout << "Visiting: " << coord2d << endl;

   bool accepted = false;
   dp.nCellsTested++;

   if (dp.wholeEarth)
      accepted = true;
   else
   {
      // first check if the cell is in the overage set
      if (!ed->overageSet.empty())
      {
         set<DgIVec2D>::iterator it = ed->overageSet.find(coord2d);
         if (it != ed->overageSet.end()) // found coord2d
         {
            accepted = true;
            ed->overageSet.erase(it);
            if (dp.megaVerbose)
               dgcout << "found OVERAGE coord " << coord2d << endl;
         }
      }

      if (!accepted)
         accepted = ::evalCell(ed, coord2d);
   }

   if (accepted)
   {
      dp.nCellsAccepted++;
/*
cout << "accepted: " << quadNum_ << " " << coord2d
     << " " << superFundIndex() << endl;
*/
      DgLocation* addLoc = dgg.makeLocation(DgQ2DICoord(quadNum_, coord2d));

      DgPolygon verts(dgg);
      dgg.setVertices(*addLoc, verts, dp.nDensify);

      ::outputCell(dp, dggs, dgg, *addLoc, verts, deg, superFundIndex());

      delete addLoc;
   }

   outputStatus(dp);

   if (dp.megaVerbose)
      dgcout << coord2d << " " << ciNdx_ << " " << sfNdx_ << endl;

   return 1;
}

unsigned long long int
DgHexSF::depthFirstTraversal (GridGenParam& dp, const DgIDGGSBase& dggs,
       const DgIDGGBase& dgg, const DgContCartRF& deg, int numAp4Res,
       DgEvalData* ed)
{
/*
cout << "depthFirstTrav res: " << res_
     <<  " ijk: " << ijkCoord() << endl;
*/

   if (res_ == dgg.res())
      return visitMe(dp, dggs, dgg, deg, ed);

   // otherwise recursively descend

   int childRes = res_ + 1;
   int aperture = 4;
   bool childClassI = true;
   const int startSFDigit = 1;
   int numAp3 = childRes - numAp4Res;
   if (childRes > numAp4Res)
   {
      aperture = 3;
      if (numAp3 % 2) childClassI = false;
   }
   DgHexSF center;

   unsigned long long int numAccepted = 0;
   if (quadNum_ == 0 || quadNum_ == 11) // only one child
   {
      center = DgHexSF(0, 0, 0, childRes);
      center.classI_ = childClassI;
      center.type_ = type_; // singleton pentagon
      center.quadNum_ = quadNum_;
      center.setCiNdx(ciNdx_ + dgg::util::to_string(startSFDigit));
      if (numAp3 <= 1)
         center.setSfNdx(sfNdx_ + dgg::util::to_string(startSFDigit));
      else
      {
         center.setSfNdx(sfNdx_);
         center.addSf3Digit(startSFDigit);
      }

      numAccepted = center.depthFirstTraversal(dp, dggs, dgg, deg, numAp4Res, ed);
   }
   else // quadNum 1-10
   {
      // coarse filter on res 1 quads (4 per icosa quad)
      if (ed && res_ == 1)
      {
         bool useTile = false;

         unsigned long long int i = ijkCoord().i();
         unsigned long long int j = ijkCoord().j();

         // all base tiles (except for the upper right)
         // contain some portion of the quad edge
         if (!ed->overageSet.empty() && (i != 1 || j != 1))
            useTile = true;

         const DgQuadClipRegion& cr = ed->clipRegion;
         if (!useTile && cr.isQuadUsed())
         {
            // find bounds of children at target res

            long long int midI = (dgg.maxI() + 1) / 2;
            long long int midJ = (dgg.maxJ() + 1) / 2;
            long long int left = 0, right = 0, top = 0, bottom = 0;

            if (i == 0) // base tile child 1 or 2
            {
               left = 0;
               right = midI - 1;
            }
            else if (i == 1) // base tile child 3 or 4
            {
               left = midI;
               right = dgg.maxI();
            }
            else
               ::report("depthFirstTraversal(). Invalid res 1 tile.",
                        DgBase::Fatal);

            if (j == 0) // base tile child 1 or 3
            {
               bottom = 0;
               top = midJ - 1;
            }
            else if (j == 1) // base tile child 2 or 4
            {
               bottom = midJ;
               top = dgg.maxJ();
            }
            else
               ::report("depthFirstTraversal(). Invalid res 1 tile.",
                        DgBase::Fatal);

            useTile = cr.offset().i() <= right &&
                      cr.upperRight().i() >= left &&
                      cr.offset().j() <= top &&
                      cr.upperRight().j() >= bottom;
         }

/*
         cout << "QUAD: " << quadNum_ << " TILE: " << ijkCoord()
              << ((useTile) ? " USED" : " SKIPPED") << endl;
*/

         if (!useTile) return 0;
      }

      ///// process each child
      string cs; // child set
      if (aperture == 4)
      {
         cs = cs4;
         center = downAp4();
      }
      else if (aperture == 3 && !childClassI)
      {
         center = downAp3();

         if (type_ == 'A')
            cs = cs3A;
         else
            cs = cs3B;
      }
      else if (aperture == 3) // 3r
      {
         switch (type_)
         {
            case 'A': cs = cs3rA; break;
            case 'B': cs = cs3rB; break;
            case 'C': cs = cs3rC; break;
            case 'D': cs = cs3rD; break;
            case 'E': cs = cs3rE; break;
            case 'F': cs = cs3rF; break;
         }

         center = downAp3r();
      }
      else
         ::report("ERROR: HexHeir.createSets().", DgBase::Fatal);

      center.classI_ = childClassI;
      center.type_ = cs[0];
      center.quadNum_ = quadNum_;
      center.setCiNdx(ciNdx_ + dgg::util::to_string(startSFDigit));
      if (numAp3 <= 1)
         center.setSfNdx(sfNdx_ + dgg::util::to_string(startSFDigit));
      else
      {
         center.setSfNdx(sfNdx_);
         center.addSf3Digit(startSFDigit);
      }

      unsigned long long int numAccepted =
            center.depthFirstTraversal(dp, dggs, dgg, deg, numAp4Res, ed);

      int sfDigit = startSFDigit + 1;
      for (int d = 1; d < 8; d++)
      {
         char childType = cs[d];
         if (isalpha(childType))
         {
            DgHexSF h = center.dirFromCenter(d);
            h.type_ = childType;
            h.classI_ = childClassI;
            h.quadNum_ = quadNum_;
            h.setCiNdx(ciNdx_ + dgg::util::to_string(d));
            if (numAp3 <= 1)
               h.setSfNdx(sfNdx_ + dgg::util::to_string(sfDigit));
            else
            {
               h.setSfNdx(sfNdx_);
               h.addSf3Digit(sfDigit);
            }

            numAccepted += h.depthFirstTraversal(dp, dggs, dgg, deg, numAp4Res, ed);
            sfDigit++;
         }
      } // else quad 1-10
   }

   return numAccepted;
}

DgHexSF
DgHexSF::downAp4 (void)
{
   int i, j, k;

   i = (int) ijkCoord_.i() * 2;
   j = (int) ijkCoord_.j() * 2;
   k = (int) ijkCoord_.k() * 2;

   return DgHexSF(i, j, k, res_ + 1);
}

DgHexSF
DgHexSF::downAp3 (void)
{
   // res r unit vectors in res r+1
   DgIVec3D iVec(2, 0, 1);
   DgIVec3D jVec(1, 2, 0);
   DgIVec3D kVec(0, 1, 2);

   iVec *= ijkCoord_.i();
   jVec *= ijkCoord_.j();
   kVec *= ijkCoord_.k();

   DgIVec3D sum = iVec + jVec + kVec;

   return DgHexSF(sum, res_ + 1);
}

DgHexSF
DgHexSF::downAp3r (void)
{
   // res r unit vectors in res r+1
   DgIVec3D iVec(2, 1, 0);
   DgIVec3D jVec(0, 2, 1);
   DgIVec3D kVec(1, 0, 2);

   iVec *= ijkCoord_.i();
   jVec *= ijkCoord_.j();
   kVec *= ijkCoord_.k();

   DgIVec3D sum = iVec + jVec + kVec;

   return DgHexSF(sum, res_ + 1);
}

DgHexSF
DgHexSF::dirFromCenter (int digit)
{
   int i = (int) ijkCoord_.i();
   int j = (int) ijkCoord_.j();
   int k = (int) ijkCoord_.k();

   switch (digit)
   {
      case 0:
      case 7:
         break;
      case 1:
         i += 0;
         j += 0;
         k += 1;
         break;
      case 2:
         i += 0;
         j += 1;
         k += 0;
         break;
      case 3:
         i += 0;
         j += 1;
         k += 1;
         break;
      case 4:
         i += 1;
         j += 0;
         k += 0;
         break;
      case 5:
         i += 1;
         j += 0;
         k += 1;
         break;
      case 6:
         i += 1;
         j += 1;
         k += 0;
         break;
      default:
         ::report(string("ERROR: invalid digit ") + dgg::util::to_string(digit)
            + string(" in HexHeir.dirFromCenter()."), DgBase::Fatal);
   }

   return DgHexSF(i, j, k, res_);
}

////////////////////////////////////////////////////////////////////////////////

