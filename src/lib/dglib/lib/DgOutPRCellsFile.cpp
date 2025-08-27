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
// DgOutPRCellsFile.cpp: DgOutPRCellsFile class implementation
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <sstream>

#include <dglib/DgOutPRCellsFile.h>
#include <dglib/DgLocList.h>
#include <dglib/DgPolygon.h>
#include <dglib/DgLocation.h>
#include <dglib/DgCell.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutPRCellsFile::DgOutPRCellsFile (const DgRFBase& rfIn,
        const std::string& fileNameIn, int precision, DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, true, "cells", precision, failLevel)
{
   // test for override of vecAddress
   DgAddressBase* dummy = rfIn.vecAddress(DgDVec2D(M_ZERO, M_ZERO));
   if (!dummy)
      DgOutputStream::report("DgOutPRCellsFile::DgOutPRCellsFile(): RF " + rfIn.name() +
             " must override the vecAddress() method", DgBase::Fatal);
   delete dummy;

   setFormatStr();

} // DgOutPRCellsFile::DgOutPRCellsFile

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRCellsFile::insert (const DgDVec2D& pt)
//
// Put the point pt.
//
////////////////////////////////////////////////////////////////////////////////
{
   const int maxBuffSize = 200;
   char buff[maxBuffSize];

   // switch to lat/lon order
   snprintf(buff, maxBuffSize, formatStr(), pt.y(), pt.x());

   *this << buff;

   return *this;

} // DgOutLocFile& DgOutPRCellsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRCellsFile::insert (DgLocation&, const std::string*,
                  const DgDataList* /* dataList */)
//
// Put the point loc.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPRCellsFile::insert(DgLocation): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutPRCellsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRCellsFile::insert (DgLocVector&, const std::string*, const DgLocation*,
            const DgDataList*)
//
// Put the polyline vec.
//
////////////////////////////////////////////////////////////////////////////////
{
   DgOutputStream::report("DgOutPRCellsFile::insert(DgLocVector): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutPRCellsFile::insert

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutPRCellsFile::insert (DgPolygon& poly, const std::string* label,
                     const DgLocation* /* cent */,
                  const DgDataList* /* dataList */)
//
// Put the polygon poly.
//
////////////////////////////////////////////////////////////////////////////////
{
   rf().convert(poly);

   // output the header line
   if (label)
     *this << *label;
   else
     *this << "0";

   // output the vertices in reverse order (clockwise winding)
   const std::vector<DgAddressBase*>& v = poly.addressVec();
   for (int i = 0; i < (int) v.size(); i++)
   {
      this->insert(rf().getVecAddress(*v[i]));
   }

   *this << std::endl;

   return *this;

} // DgOutLocFile& DgOutPRCellsFile::insert


