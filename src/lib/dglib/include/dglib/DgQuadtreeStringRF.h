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

#ifndef DGQUADTREESTRINGRF_H
#define DGQUADTREESTRINGRF_H

#include <dglib/DgHierNdxStringRF.h>

class DgQuadtreeStringRF : public DgHierNdxStringRF {
   public:
      DgQuadtreeStringRF (const DgHierNdxSystemRFBase& sys, int res,
                           const std::string& name)
         : DgHierNdxStringRF(sys, res, name) { }

      DgHierNdxStringCoord quantify (const DgQ2DICoord& point) const override;
      DgQ2DICoord invQuantify (const DgHierNdxStringCoord& add) const override;
      const char* str2add (DgHierNdxStringCoord* add, const char* str,
                           char delimiter) const override;
};

#endif
