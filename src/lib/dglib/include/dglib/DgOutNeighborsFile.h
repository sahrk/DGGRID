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
// DgOutNeighborsFile.h: DgOutNeighborsFile class definitions
//
// Version 7.0 - Kevin Sahr, 12/14/14
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGOUTNEIGHBORS_H
#define DGOUTNEIGHBORS_H

#include <dglib/DgOutLocFile.h>
#include <dglib/DgOutputStream.h>
#include <dglib/DgUtil.h>

#include <fstream>
#include <string>

class DgIDGGBase;

////////////////////////////////////////////////////////////////////////////////
class DgOutNeighborsFile : public DgOutputStream {

   public:

      // outRF is the frame of the addresses written; it must be able to
      // convert from the DGG's frame
      DgOutNeighborsFile (const std::string& fileName,
                        const DgRFBase& outRF,
                        const std::string& suffix = std::string("nbr"),
                        DgReportLevel failLevel = DgBase::Fatal);

      virtual DgOutNeighborsFile& insert (const DgLocation& center,
                                          const DgLocVector& vec);

      virtual bool open (const std::string& fileName,
                         DgReportLevel failLevel = DgBase::Fatal)
              { return DgOutputStream::open(fileName, failLevel); }

      virtual void close (void) { DgOutputStream::close(); }

   private:

      const DgRFBase& outRF_;    // output frame of the cell addresses
};

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, const char* str)
              { std::ostream& o = file; o << str; return file; }

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, const std::string& str)
              { std::ostream& o = file; o << str; return file; }

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, long double val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, float val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, int val)
              { std::ostream& o = file; o << val; return file; }

inline DgOutNeighborsFile& operator<< (DgOutNeighborsFile& file, unsigned long long val)
              { std::ostream& o = file; o << val; return file; }

#endif
