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
// DgOutPRPtsFile.cpp: DgOutPRPtsFile class implementation
//
// Version 7.0 - Kevin Sahr, 11/25/14
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <sstream>

#include <dglib/DgOutPRPtsFile.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutPRPtsFile::DgOutPRPtsFile (const DgRFBase& rfIn,
        const std::string& fileNameIn, int precision, DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, true, "pts", precision, failLevel)
{
   // test for override of vecAddress
   if (!rfIn.hasVecAddress())
      DgOutputStream::report("DgOutPRPtsFile::DgOutPRPtsFile(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);

   setFormatStr();

} // DgOutPRPtsFile::DgOutPRPtsFile

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::writeVec (const DgDVec2D& pt)
//
// Put the point pt.
//
////////////////////////////////////////////////////////////////////////////////
{
   const int maxBuffSize = 200;
   char buff[maxBuffSize];

   // want latitude, longitude
   snprintf(buff, maxBuffSize, formatStr(), pt.y(), pt.x());

   *this << buff;

   return *this;

} // DgOutLocFile& DgOutPRPtsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::writePoint (const DgLocation& loc, const std::string* label,
                  const DgDataList* /* dataList */)
//
// Put the point loc.
//
////////////////////////////////////////////////////////////////////////////////
{
   return writeTypedPoint(loc, "H", label);

} // DgOutPRPtsFile::writePoint

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::insert (const DgLocation& loc, const std::string& type,
                 const std::string* label)
//
// Put a copy of the point loc, converted to this file's frame.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgLocation tmpLoc(loc);
   rf().convert(&tmpLoc);

   return writeTypedPoint(tmpLoc, type, label);

} // DgOutLocFile& DgOutPRPtsFile::insert

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::writeTypedPoint (const DgLocation& loc, const std::string& type,
                 const std::string* label)
//
// Put the point loc, which is already in this file's frame.
//
////////////////////////////////////////////////////////////////////////////////
{
   if (label)
     *this << *label << " ";
   else
     *this << "0 ";

   *this << type << " ";

   this->writeVec(rf().getVecLocation(loc));

   return *this;

} // DgOutLocFile& DgOutPRPtsFile::writeTypedPoint

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::writePolyline (const DgLocVector&, const std::string*, const DgLocation*,
            const DgDataList* /* dataList */)
//
// Put the polyline vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPRPtsFile::writePolyline(): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutPRPtsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRPtsFile::writePolygon (const DgPolygon&, const std::string*, const DgLocation*,
           const DgDataList*)
//
// Put the polygon poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPRPtsFile::writePolygon(): not defined.", DgBase::Fatal);

   return *this;

} // DgOutLocFile& DgOutPRPtsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
