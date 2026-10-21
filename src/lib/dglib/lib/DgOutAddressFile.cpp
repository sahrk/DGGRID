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
// DgOutAddressFile.cpp: DgOutAddressFile class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <dglib/DgOutAddressFile.h>
#include <dglib/DgDataList.h>
#include <dglib/DgLocation.h>
#include <dglib/DgPolygon.h>

////////////////////////////////////////////////////////////////////////////////
DgOutAddressFile::DgOutAddressFile (const DgRFBase& rfIn,
                  const std::string& fileNameIn, char delimiterIn,
                  int precision, DgReportLevel failLevel)
   : DgOutLocTextFile (fileNameIn, rfIn, false, "", precision, failLevel),
     delimiter_ (delimiterIn)
{
   setFormatStr();

} // DgOutAddressFile::DgOutAddressFile

////////////////////////////////////////////////////////////////////////////////
std::string
DgOutAddressFile::addressText (const DgLocation& loc) const
{
   return rf().addressText(loc, delimiter_);
}

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAddressFile::writePoint (const DgLocation& loc, const std::string* label,
                  const DgDataList* dataList)
//
// Put the point loc, which is already in this file's frame.
//
////////////////////////////////////////////////////////////////////////////////
{
   if (label)
      *this << *label;
   else
      *this << rf().addressText(loc, delimiter_);

   if (dataList) {
      for (auto const& field : dataList->list())
         *this << std::string(1, delimiter_) << field->valString();
   }

   *this << "\n";

   return *this;

} // DgOutLocFile& DgOutAddressFile::writePoint

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAddressFile::writePolyline (const DgLocVector&, const std::string*,
                  const DgLocation*, const DgDataList*)
{
   DgOutputStream::report("DgOutAddressFile::writePolyline(): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutAddressFile::writePolyline

////////////////////////////////////////////////////////////////////////////////
DgOutLocFile&
DgOutAddressFile::writePolygon (const DgPolygon&, const std::string*,
                  const DgLocation*, const DgDataList*)
{
   DgOutputStream::report("DgOutAddressFile::writePolygon(): not defined.", DgBase::Fatal);
   return *this;

} // DgOutLocFile& DgOutAddressFile::writePolygon
