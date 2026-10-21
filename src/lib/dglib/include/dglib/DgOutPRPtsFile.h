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
// DgOutPRPtsFile.h: DgOutPRPtsFile class definitions
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGOUTPRPTSFILE_H
#define DGOUTPRPTSFILE_H

#include <dglib/DgOutLocTextFile.h>

#include <cstdio>

class DgDVec2D;
class DgPolygon;

////////////////////////////////////////////////////////////////////////////////
class DgOutPRPtsFile : public DgOutLocTextFile {

   public:

      using DgOutLocFile::insert;

      DgOutPRPtsFile (const DgRFBase& rfIn, const std::string& fileNameIn = "",
                   int precision = 7, DgReportLevel failLevel = DgBase::Fatal);

      virtual ~DgOutPRPtsFile (void) { if (good()) close(); }

      virtual void close (void) { std::ofstream::close(); }

      // converts a copy of loc to this file's frame
      DgOutLocFile& insert (const DgLocation& loc, const std::string& type,
                       const std::string* label = NULL);

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

      DgOutLocFile& writeTypedPoint (const DgLocation& loc,
                       const std::string& type, const std::string* label);

      virtual DgOutLocFile& writeVec (const DgDVec2D& pt);

      virtual void setFormatStr(void)
      {
          std::ostringstream os;
          os << "%#." << getPrecision() << "LF "
             << "%#." << getPrecision() << "LF\n";

          formatStr_ = os.str();
      }
};

////////////////////////////////////////////////////////////////////////////////

#endif
