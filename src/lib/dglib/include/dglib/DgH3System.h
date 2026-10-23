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

// H3 hierarchical indexing (https://h3geo.org), using the H3 library.
//
// Resolutions: H3 resolution 0 (the 122 base cells) is DGGS resolution 2, so
// H3 resolutions 0 through 15 are DGGS resolutions 2 through 17 (DGGS
// resolutions 0 and 1 have no index). The first indexed resolution has no
// indexing parent, and the last no indexing children.
//
// Compatibility: a supported grid is hexagonal, non-pure, has at least
// resolutions 0 through 2, and its transition apertures are 4, 3, then 7s
// (the aperture sequence 4377...). Both INT64 and DIGIT_STRING are accepted.
// Invalid grids/forms fail during construction.
//
// Text forms:
//  - INT64 is H3's canonical form (h3ToString: lowercase hex without leading
//    zeros, e.g. 8a2a1072b59ffff). Input also accepts the zero-padded 16 digit
//    form.
//  - DIGIT_STRING, as for the Z systems, is a zero-padded base cell field
//    followed by exactly one digit (0-6) for each H3 resolution: three digits
//    (000-121) for the 122 H3 base cells (Z7's field is two digits for its 12
//    base cells). H3 resolution 0 is "042", H3 resolution 2 is "042" followed
//    by two digits.
//
// Geometry: indexes are computed from the cell's topology on the DGGS's
// icosahedral projection, whatever its projection and orientation, so they are
// always valid H3 indexes. The cells are geometrically the same as Uber H3's
// only for dggs_proj GNOMONIC with dggs_orient_preset DYMAXION and the
// aperture sequence 4377..., which is what dggs_type H3 sets.
//
// Dependencies: this uses the H3 library's internal FaceIJK functions
// (_faceIjkToH3 and _h3ToFaceIjk, exported by libh3 but not declared in
// h3api.h; see DgH3Internal.h), and the public constructCell and
// getIndexDigit functions added in H3 4.4.0 (so DGGRID requires H3 4.4.1 or
// later: the 4.4.0 release identifies itself as 4.3.0).
class DgH3IntRF : public DgHierNdxIntRF {
   public:
      DgH3IntRF (const DgHierNdxSystemRFBase& sys, int res,
                 const std::string& name)
         : DgHierNdxIntRF(sys, res, name) { }

      DgHierNdxIntCoord quantify (const DgQ2DICoord& point) const override;
      DgQ2DICoord invQuantify (const DgHierNdxIntCoord& add) const override;
      const char* str2add (DgHierNdxIntCoord* add, const char* str,
                           char delimiter) const override;
      std::string add2str (const DgHierNdxIntCoord& add) const override;
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
      // With dggs_orient_preset DYMAXION, DGGRID vertex v is the H3 pentagon
      // base cell vertToH3BaseCell[v]. For every orientation, since DGGRID's
      // triangle numbering is topological, DGGRID ProjTri triangle t is H3
      // face t, with DGGRID V0, V1, V2 the H3 i, j, k vertices of the face.
      // Not needed at run time; here as documentation and for verification.
      static const int vertToH3BaseCell[12];
      static void validateCompatibility (const DgIDGGSBase& dggs,
                                          DgHierNdxFormType form);
      static DgH3System* makeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                                     const std::string& name = "H3");

      // Set verts (in dgg.geoRF()) to the boundary of the cell at add as
      // H3's cellToBoundary reports it: the cell vertices plus the points
      // where its edges cross icosahedron edges (and, for a pentagon, the
      // distortion vertices), joined by great circle arcs. Each arc is then
      // divided into densify + 1 equal parts. The cell is the H3 cell with
      // the topology of the DGGRID cell, so this is the boundary of the
      // DGGRID cell only if dgg has H3's geometry (see above).
      static void setCellBoundary (const DgIDGGBase& dgg, const DgQ2DICoord& add,
                                   DgPolygon& verts, int densify);

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
