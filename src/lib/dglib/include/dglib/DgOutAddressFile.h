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
// DgOutAddressFile.h: DgOutAddressFile class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGOUTADDRESSFILE_H
#define DGOUTADDRESSFILE_H

#include <dglib/DgOutLocTextFile.h>

class DgDVec2D;

////////////////////////////////////////////////////////////////////////////////
// A text file with a line for each point: the point's label (or, with no
// label, its address in this file's frame) followed by the delimited data
// fields. The frame can be any frame that has an address text, including
// those with no 2-D coordinates (a sequence number or a hierarchical index).
class DgOutAddressFile : public DgOutLocTextFile {

   public:

      DgOutAddressFile (const DgRFBase& rfIn, const std::string& fileNameIn = "",
                   char delimiterIn = ' ', int precision = 7,
                   DgReportLevel failLevel = DgBase::Fatal);

      virtual ~DgOutAddressFile (void) { if (good()) close(); }

      virtual void close (void) { std::ofstream::close(); }

      char delimiter (void) const { return delimiter_; }

      // the address of loc in this file's frame as text
      std::string addressText (const DgLocation& loc) const;

   protected:

      virtual DgOutLocFile& writePoint (const DgLocation& loc,
                                const std::string* label,
                                const DgDataList* dataList);

      virtual DgOutLocFile& writePolyline (const DgLocVector& vec,
                                const std::string* label,
                                const DgLocation* cent,
                                const DgDataList* dataList);

      virtual DgOutLocFile& writePolygon (const DgPolygon& poly,
                                const std::string* label,
                                const DgLocation* cent,
                                const DgDataList* dataList);

   private:

      char delimiter_;

      virtual void setFormatStr(void) { }
};

////////////////////////////////////////////////////////////////////////////////

#endif
