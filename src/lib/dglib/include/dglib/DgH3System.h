/*******************************************************************************
    Copyright (C) 2026 DGGRID contributors

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

#ifndef DGH3SYSTEM_H
#define DGH3SYSTEM_H

// USE_H3 is set in MakeIncludes / CMake
#ifdef USE_H3

#include <dglib/DgHierNdxSystemRFS.h>
#include <dglib/DgHierNdxIntRF.h>
#include <dglib/DgHierNdxStringRF.h>

// H3 integration scaffold. A supported grid is hexagonal, non-pure, has at
// most 18 resolutions (0 through 17), and its transition apertures are a
// prefix of 43 followed by fifteen 7s. Both INT64 and DIGIT_STRING are
// accepted. Invalid grids/forms fail during construction. The index math
// methods below are deliberately stubs and fail explicitly when called.
class DgH3IntRF : public DgHierNdxIntRF {
   public:
      DgH3IntRF (const DgHierNdxSystemRFBase& sys, int res,
                 const std::string& name)
         : DgHierNdxIntRF(sys, res, name) { }

      DgHierNdxIntCoord quantify (const DgQ2DICoord& point) const override;
      DgQ2DICoord invQuantify (const DgHierNdxIntCoord& add) const override;
      const char* str2add (DgHierNdxIntCoord* add, const char* str,
                           char delimiter) const override;
};

class DgH3StringRF : public DgHierNdxStringRF {
   public:
      DgH3StringRF (const DgHierNdxSystemRFBase& sys, int res,
                    const std::string& name)
         : DgHierNdxStringRF(sys, res, name) { }

      DgHierNdxStringCoord quantify (const DgQ2DICoord& point) const override;
      DgQ2DICoord invQuantify (const DgHierNdxStringCoord& add) const override;
      const char* str2add (DgHierNdxStringCoord* add, const char* str,
                           char delimiter) const override;
};

class DgH3System : public DgHierNdxSystemRFS<DgH3IntRF, DgH3StringRF> {
   public:
      static void validateCompatibility (const DgIDGGSBase& dggs,
                                          DgHierNdxFormType form);
      static DgH3System* makeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                                     const std::string& name = "H3");

   protected:
      DgH3System (const DgIDGGSBase& dggs, bool extModeInt,
                  const std::string& name);

      DgHierNdxIntCoord toIntCoord (const DgHierNdxStringCoord& c,
                                    int gridRes) const override;
      DgHierNdxStringCoord toStringCoord (const DgHierNdxIntCoord& c,
                                          int gridRes) const override;
      void setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                            DgLocation& parent) const override;
      void setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                              DgLocVector& children) const override;
};

#endif // USE_H3

#endif
