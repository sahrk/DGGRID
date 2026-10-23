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
// DgOutAIGenFile.cpp: DgOutAIGenFile class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <sstream>

#include <dglib/DgOutAIGenFile.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutAIGenFile::DgOutAIGenFile (const DgRFBase& rfIn, const std::string& fileNameIn,
                          int precision, bool isPointFile,
                          DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, isPointFile, "gen", precision,
                       failLevel)
{
   // test for override of vecAddress
   if (!rfIn.hasVecAddress())
      DgOutputStream::report("DgOutAIGenFile::DgOutAIGenFile(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);

   setFormatStr();

} // DgOutAIGenFile::DgOutAIGenFile

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAIGenFile::writeVec (const DgDVec2D& pt)
//
// Put the point pt.
//
////////////////////////////////////////////////////////////////////////////////
{
   const std::string buff = formatVec(pt.x(), pt.y());

   *this << buff;

   return *this;

} // DgOutLocFile& DgOutAIGenFile::write

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAIGenFile::writePoint (const DgLocation& loc, const std::string* label,
                  const DgDataList* /* dataList */)
//
// Put the point loc.
//
////////////////////////////////////////////////////////////////////////////////
{
   if (label)
     *this << *label << " ";
   else
     *this << "0 ";

   this->writeVec(rf().getVecLocation(loc));

   return *this;

} // DgOutLocFile& DgOutAIGenFile::write

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAIGenFile::writePolyline (const DgLocVector& vec, const std::string* label,
                     const DgLocation* cent,
                  const DgDataList* /* dataList */)
//
// Put the polyline vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   // output the header line
   if (label)
     *this << *label;
   else
     *this << "0";

   if (cent)
   {
      *this << " ";
      this->writeVec(rf().getVecLocation(*cent));
   }
   else
      *this << "\n";

   // output the vertices
   const std::vector<DgAddressBase*>& v = vec.addressVec();
   for (unsigned long i = 0; i < v.size(); i++)
      this->writeVec(rf().getVecAddress(*v[i]));

   *this << "END" << std::endl;

   return *this;

} // DgOutLocFile& DgOutAIGenFile::write

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAIGenFile::writePolygon (const DgPolygon& poly, const std::string* label,
                     const DgLocation* cent,
                     const DgDataList* /* dataList */)
//
// Put the polygon poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   // output the header line
   if (label)
     *this << *label;
   else
     *this << "0";

   if (cent)
   {
      *this << " ";
      this->writeVec(rf().getVecLocation(*cent));
   }
   else
      *this << "\n";

   // output the vertices in reverse order (clockwise winding)
   const std::vector<DgAddressBase*>& v = poly.addressVec();
   for (int i = (int) (v.size() - 1); i >= 0; i--)
      this->writeVec(rf().getVecAddress(*v[i]));

   // rewrite the first vertex
   this->writeVec(rf().getVecAddress(*v[v.size() - 1]));

   *this << "END" << std::endl;

   return *this;

} // DgOutLocFile& DgOutAIGenFile::write

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
