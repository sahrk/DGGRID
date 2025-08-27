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
// DgInAIGenFile.cpp: DgInAIGenFile class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgCell.h>
#include <dglib/DgContCartRF.h>
#include <dglib/DgEllipsoidRF.h>
#include <dglib/DgInAIGenFile.h>
#include <dglib/DgLocation.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>

#include <sstream>

const static int maxLine = 256;

////////////////////////////////////////////////////////////////////////////////
static void fixSciNotation (char* std::string)
//
// convert 'D' to 'E' scientific notation
//
{
   char* c = string;
   while ((c = strchr(c, 'D')) && (c[1] == '+' || c[1] == '-')) *c = 'E';

} // void fixSciNotation

////////////////////////////////////////////////////////////////////////////////
DgInAIGenFile::DgInAIGenFile (const DgRFBase& rfIn, const std::string* fileNameIn,
                        DgReportLevel failLevel)
   : DgInLocStreamFile (rfIn, fileNameIn, false, failLevel),
     forcePolyLine_ (false), forceCells_ (false)
{
   // test for override of vecAddress
   DgAddressBase* dummy = rfIn.vecAddress(DgDVec2D(M_ZERO, M_ZERO));
   if (!dummy)
      report("DgInAIGenFile::DgInAIGenFile(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);
   delete dummy;

} // DgInAIGenFile::DgInAIGenFile

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgInLocFile&
DgInAIGenFile::extract (DgLocVector& vec)
//
// Get the next polyline from me and put it in vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   vec.clearAddress();
   rf().convert(vec);

   // get the header line
   char nextLine[maxLine];
   getline(nextLine, maxLine);

   // check to see if we're at EOF
   if (std::string(nextLine) == std::string("END")) {
      // force feof()
      while (!eof()) getline(nextLine, maxLine);
      return *this;
   }

   long double x, y;
   //vector<DgAddressBase*>& v = vec.addressVec();
   while (!eof()) {
      getline(nextLine, maxLine);

      // check for end-of-polyline
      if (std::string(nextLine) == std::string("END")) break;

      fixSciNotation(nextLine);

      istringstream iss(nextLine);
      iss >> x >> y;

      DgAddressBase* add = rf().vecAddress(DgGeoCoord(x, y));
      vec.addressVec().push_back(add); // polyline should delete when done
   }

   //cout << "HERE: " << poly << std::endl;

   return *this;

} // DgInAIGenFile& DgInAIGenFile::extract

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgInLocFile&
DgInAIGenFile::extract (DgPolygon& poly)
//
// Get the next polygon from me and put it in poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   poly.clearAddress();
   rf().convert(poly);

   // get the header line
   char nextLine[maxLine];
   getline(nextLine, maxLine);

   // check to see if we're at EOF
   if (std::string(nextLine) == std::string("END")) {
      // force feof()
      while (!eof()) getline(nextLine, maxLine);
      return *this;
   }

   long double x, y;
   std::vector<DgAddressBase*>& v = poly.addressVec();
   while (!eof()) {
      getline(nextLine, maxLine);

      // check for end-of-polygon
      // delete the duplicate first/last vertex
      if (std::string(nextLine) == std::string("END")) {
         delete v.back();
         v.back() = NULL;
         v.pop_back();

         //delete v[v.size() - 1];
         //v[v.size() - 1] = NULL;
         //v.pop_back;

         break;
      }
      fixSciNotation(nextLine);

      istringstream iss(nextLine);
      iss >> x >> y;

      DgAddressBase* add = rf().vecAddress(DgGeoCoord(x, y));
      poly.addressVec().push_back(add); // polygon should delete when done
   }

   //cout << "HERE: " << poly << std::endl;

   return *this;

} // DgInAIGenFile& DgInAIGenFile::extract

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgInLocFile&
DgInAIGenFile::extract (DgCell& cell)
//
// Get the next cell from me and put it in cell.
//
////////////////////////////////////////////////////////////////////////////////
{
   char nextLine[maxLine];

   // get the id;

   int id;
   long double x, y;

   cell.clearAddress();
   rf().convert(&cell);

   getline(nextLine, maxLine);
   fixSciNotation(nextLine);

   istringstream iss(nextLine);

   iss >> id;
   cell.setLabel(dgg::util::to_string(id));

   iss >> x >> y;
   if (iss.fail()) x = y = 0.0L;
   DgLocation* tmpLoc = rf().vecLocation(DgDVec2D(x, y));
   cell.setNode(*tmpLoc);
   delete tmpLoc;

   if (!isPointFile())
   {
      DgPolygon* poly = new DgPolygon(rf());
      while (!eof())
      {
         getline(nextLine, maxLine);
         if (string(nextLine) == string("END"))
         {
            poly->addressVec().erase(poly->addressVec().end() - 1);

            break;
         }
         fixSciNotation(nextLine);

         istringstream iss(nextLine);
         iss >> x >> y;

         DgAddressBase* add = rf().vecAddress(DgDVec2D(x, y));
         poly->addressVec().push_back(add); // polygon should delete when done
     }
     cell.setRegion(poly);
   }

   return *this;

} // DgInAIGenFile& DgInAIGenFile::extract

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgInLocFile&
DgInAIGenFile::extract (DgLocList& list)
//
// Determine whether the file is a point or polygon/polyline file. If it's
// a point file, read-in the points. If not, get the sets which constitute
// me. If the last point in a set is the same as the first, assume it's a
// polygon. Otherwise, make it a polyline.
//
////////////////////////////////////////////////////////////////////////////////
{
   list.destroy();
   rf().convert(&list);
   list.setIsOwner(true);

   // determine whether this is a points file.

   char nextLine[maxLine];
   char tmp[maxLine];

   // get the second line

   getline(nextLine, maxLine);
   getline(nextLine, maxLine);
   if (this->eof()) return *this;

   fixSciNotation(nextLine);

   istringstream iss(nextLine);

   iss >> tmp;

   if (tmp[0] == 'E') {
      setIsPointFile(true);
   } else {
      // try to get values

      long double x, y;

      iss >> x >> y;

      setIsPointFile(!iss.fail());
   }

   // go back to the beginning

   rewind();

   if (!isPointFile()) {
      // read-in the sets

      while (true) {
         if (forceCells()) {
            DgCell* cell = new DgCell();
            this->extract(*cell);
            if (this->eof()) {
               delete cell;
               break;
            }

            list.push_back(cell);
         } else {
            DgLocVector* vec = new DgLocVector();
            this->extract(*vec);
            if (this->eof()) {
               // determine whether it's a polygon

               if (!forcePolyLine() && vec->size() > 2 &&
                   rf().getVecAddress(*(vec->addressVec().front())) ==
                   rf().getVecAddress(*(vec->addressVec().back()))) {
                  vec->addressVec().erase(vec->addressVec().end() - 1);

                  DgPolygon* poly = new DgPolygon(*vec);

                  list.push_back(poly);

                  //vec->destroy();
                  delete vec;
               } else {
                  list.push_back(vec);
               }
            } else {
               delete vec;
               break;
            }
         }
      }
   } else { // point file

      // read-in the points
      while (true) {
         if (forceCells()) {
            DgCell* cell = new DgCell();
            this->extract(*cell);
            if (this->eof()) {
               delete cell;
               break;
            }

            list.push_back(cell);
         } else {
            DgLocation* pt = new DgLocation();
            this->extract(*pt);
            if (!(this->eof()) &&
                rf().getVecLocation(*pt) != DgDVec2D::undefDgDVec2D) {
               list.push_back(pt);
            } else {
               delete pt;
               break;
            }
         }
      }
   }

   return *this;

} // DgInLocFile& DgInAIGenFile::extract

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgInLocFile&
DgInAIGenFile::extract (DgLocation& loc)
//
// Get the next DgLocation. For speed will mistake file corruption for the "END"
// at the end of the gen file.
//
////////////////////////////////////////////////////////////////////////////////
{
   char nextLine[maxLine];

   int id;
   long double x, y;

   // get the values

   getline(nextLine, maxLine);
   fixSciNotation(nextLine);
   istringstream iss(nextLine);
   iss >> id >> x >> y;

   // set the values

   rf().convert(&loc);
   DgDVec2D v(x, y);
   if (iss.fail()) { // hopefully "END"
      v = DgDVec2D::undefDgDVec2D;
   }

   DgLocation* tmpLoc = rf().vecLocation(v);
   loc = *tmpLoc;
   delete tmpLoc;

   return *this;

} // DgInLocFile& DgInAIGenFile::extract

////////////////////////////////////////////////////////////////////////////////
