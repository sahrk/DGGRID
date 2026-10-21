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
// DgOutNdxChildrenFile.h: DgOutNdxChildrenFile class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGOUTNDXCHILDREN_H
#define DGOUTNDXCHILDREN_H

#include <fstream>
#include <string>

#include <dglib/DgOutLocFile.h>
#include <dglib/DgOutputStream.h>
#include <dglib/DgUtil.h>

class DgIDGGBase;

////////////////////////////////////////////////////////////////////////////////
class DgOutNdxChildrenFile : public DgOutputStream {

   public:

      // outRF and ndxChdOutRF are the frames of the cell and indexing child
      // addresses
      DgOutNdxChildrenFile (const std::string& fileName,
                        const DgRFBase& outRF,
                        const DgRFBase& ndxChdOutRF,
                        const std::string& suffix = std::string("ndxChd"),
                        DgReportLevel failLevel = DgBase::Fatal);

      virtual DgOutNdxChildrenFile& insert
                      (const DgLocation& center, const DgLocVector& vec);

   private:

      const DgRFBase& outRF_;    // primary res output RF
      const DgRFBase& ndxChdOutRF_; // indexing child res output RF

};

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, const char* str)
              { std::ostream& o = file; o << str; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, const std::string& str)
              { std::ostream& o = file; o << str; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, long double val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, float val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, int val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, unsigned long long val)
              { std::ostream& o = file; o << val; return file; }

#endif
