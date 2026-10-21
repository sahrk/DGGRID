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
// DgOutRandPtsText.cpp: DgOutRandPtsText class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <sstream>

#include <dglib/DgOutRandPtsText.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutRandPtsText::DgOutRandPtsText (const DgRFBase& rfIn,
        const std::string& fileNameIn, int precision, DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, true, "txt", precision, failLevel)
{
   // test for override of vecAddress
   if (!rfIn.hasVecAddress())
      DgOutputStream::report("DgOutRandPtsText::DgOutRandPtsText(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);

   setFormatStr();

} // DgOutRandPtsText::DgOutRandPtsText

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutRandPtsText::writeVec (const DgDVec2D& pt)
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

} // DgOutLocFile& DgOutRandPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutRandPtsText::writePoint (const DgLocation& loc, const std::string* label,
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

} // DgOutLocFile& DgOutRandPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutRandPtsText::writePolyline (const DgLocVector&, const std::string*, const DgLocation*,
          const DgDataList*)
//
// Put the polyline vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutRandPtsText::writePolyline(): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutRandPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutRandPtsText::writePolygon (const DgPolygon&, const std::string*, const DgLocation*,
          const DgDataList*)
//
// Put the polygon poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutRandPtsText::writePolygon(): not defined.", DgBase::Fatal);

   return *this;

} // DgOutLocFile& DgOutRandPtsText::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
