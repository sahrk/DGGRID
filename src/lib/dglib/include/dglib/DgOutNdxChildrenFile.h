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

using namespace std;

class DgIDGGBase;

////////////////////////////////////////////////////////////////////////////////
class DgOutNdxChildrenFile : public DgOutputStream {

   public:

      DgOutNdxChildrenFile (const string& fileName,
                        const DgIDGGBase& dgg,
                        const DgIDGGBase& chdDgg,
                        const DgRFBase* outRF = NULL,
                        const DgRFBase* ndxChdOutRF = NULL,
                        const string& suffix = string("ndxChd"),
                        DgReportLevel failLevel = DgBase::Fatal);

      virtual DgOutNdxChildrenFile& insert 
                      (const DgLocation& center, DgLocVector& vec);

   private:

      const DgIDGGBase& dgg_;    // primary res dgg
      const DgIDGGBase& chdDgg_; // indexing (and spatial) child res dgg
      const DgRFBase* outRF_;    // primary res output RF (NULL indicates seqNum)
      const DgRFBase* ndxChdOutRF_; // indexing child res output RF

};

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, const char* str)
              { ostream& o = file; o << str; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, const string& str)
              { ostream& o = file; o << str; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, long double val)
              { ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, float val)
              { ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, int val)
              { ostream& o = file; o << val; return file; }

inline DgOutNdxChildrenFile& operator<< (DgOutNdxChildrenFile& file, unsigned long long val)
              { ostream& o = file; o << val; return file; }

#endif
