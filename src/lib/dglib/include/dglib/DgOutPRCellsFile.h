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
// DgOutPRCellsFile.h: DgOutRandPtsText class definitions
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGOUTPRCELLSFILE_H
#define DGOUTPRCELLSFILE_H

#include <dglib/DgOutLocTextFile.h>

#include <cstdio>

class DgDVec2D;
class DgPolygon;

////////////////////////////////////////////////////////////////////////////////
class DgOutPRCellsFile : public DgOutLocTextFile {

   using DgOutLocFile::insert;

   public:

      DgOutPRCellsFile (const DgRFBase& rfIn, const std::string& fileNameIn = "",
                   int precision = 7, DgReportLevel failLevel = DgBase::Fatal);

      virtual ~DgOutPRCellsFile (void) { if (good()) close(); }

      virtual void close (void) { std::ofstream::close(); }

      virtual DgOutLocFile& insert (DgLocation& loc, const std::string* label = nullptr,
                                const DgDataList* dataList = nullptr);

      virtual DgOutLocFile& insert (DgLocVector& vec, const std::string* label = nullptr,
                                const DgLocation* cent = nullptr,
                                const DgDataList* dataList = nullptr);

      virtual DgOutLocFile& insert (DgPolygon& poly, const std::string* label = nullptr,
                                const DgLocation* cent = nullptr,
                                const DgDataList* dataList = nullptr);

   private:

      virtual DgOutLocFile& insert (const DgDVec2D& pt);

      virtual void setFormatStr(void)
      {
          std::ostringstream os;
          os << " %#." << getPrecision() << "LF, "
             << "%#." << getPrecision() << "LF";

          formatStr_ = os.str();
      }
};

////////////////////////////////////////////////////////////////////////////////

#endif
