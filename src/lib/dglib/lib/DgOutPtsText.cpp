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
// DgOutPtsText.cpp: DgOutPtsText class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <sstream>

#include <dglib/DgOutPtsText.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutPtsText::DgOutPtsText (const DgRFBase& rfIn, const std::string& fileNameIn,
                          int precision, DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, true, "txt", precision, failLevel)
{
   // test for override of vecAddress
   if (!rfIn.hasVecAddress())
      DgOutputStream::report("DgOutPtsText::DgOutPtsText(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);

   setFormatStr();

} // DgOutPtsText::DgOutPtsText

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPtsText::writeVec (const DgDVec2D& pt)
//
// Put the point pt.
//
////////////////////////////////////////////////////////////////////////////////
{
   const int maxBuffSize = 200;
   char buff[maxBuffSize];

   snprintf(buff, maxBuffSize, formatStr(), pt.x(), pt.y());

   *this << buff;

   return *this;

} // DgOutLocFile& DgOutPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPtsText::writePoint (const DgLocation& loc, const std::string* label,
                  const DgDataList* /* dataList */)
//
// Put the point loc.
//
////////////////////////////////////////////////////////////////////////////////
{
   if (label)
     *this << *label << ",";
   else
     *this << "0,";

   this->writeVec(rf().getVecLocation(loc));

   return *this;

} // DgOutLocFile& DgOutPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPtsText::writePolyline (const DgLocVector&, const std::string*, const DgLocation*,
                  const DgDataList*)
//
// Put the polyline vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPtsText::writePolyline(): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPtsText::writePolygon (const DgPolygon&, const std::string*, const DgLocation*,
                  const DgDataList*)
//
// Put the polygon poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPtsText::writePolygon(): not defined.", DgBase::Fatal);

   return *this;

} // DgOutLocFile& DgOutPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
