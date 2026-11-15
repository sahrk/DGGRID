/*******************************************************************************
    Copyright (C) 2026 Kevin Sahr

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

#ifndef DGQUADTREESYSTEM_H
#define DGQUADTREESYSTEM_H

#include <dglib/DgHierNdxSystemRFS.h>
#include <dglib/DgQuadtreeRF.h>
#include <dglib/DgQuadtreeStringRF.h>

// Pure aperture-4 triangles, indexed at DGGS resolutions 0 through 29.
// DIGIT_STRING: two decimal root characters (00-19), then one digit per level.
// Root = 2 * (quad - 1) + t; t=0 is Q2DI(quad,0,0), t=1 is (quad,0,1).
// INT64: root in bits 63..59, two-bit digits in bits 58..1 (coarsest first),
// with zero padding in unused slots and bit 0. Resolution belongs to the RF.
// Children: 0=center, 1=apex opposite the edge parallel to the quad's shared
// triangle edge, then 2 and 3 counter-clockwise in the local projection plane.
class DgQuadtreeSystem : public
      DgHierNdxSystemRFS<DgQuadtreeRF, DgQuadtreeStringRF> {
   public:
      static DgQuadtreeSystem* makeSystem (const DgIDGGSBase& dggs,
            bool extModeInt = true, const std::string& name = "Quadtree");
      static void validateCompatibility (const DgIDGGSBase& dggs);

      // Shared by the string RF and the bit-packing conversions.
      static void validateResolution (int res);
      static int validateString (const std::string& value, int res);

   protected:
      DgQuadtreeSystem (const DgIDGGSBase& dggs, bool extModeInt,
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

#endif
