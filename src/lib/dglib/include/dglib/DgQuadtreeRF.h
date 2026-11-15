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

#ifndef DGQUADTREERF_H
#define DGQUADTREERF_H

#include <dglib/DgHierNdxIntRF.h>

class DgQuadtreeRF : public DgHierNdxIntRF {
   public:
      DgQuadtreeRF (const DgHierNdxSystemRFBase& sys, int res,
                     const std::string& name)
         : DgHierNdxIntRF(sys, res, name) { }

      DgHierNdxIntCoord quantify (const DgQ2DICoord& point) const override;
      DgQ2DICoord invQuantify (const DgHierNdxIntCoord& add) const override;
      const char* str2add (DgHierNdxIntCoord* add, const char* str,
                           char delimiter) const override;
};

#endif
