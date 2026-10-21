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

#ifndef DGH3INTERNAL_H
#define DGH3INTERNAL_H

// Private header (it lives in lib/, so it is never installed). Only used when
// USE_H3 is defined.
#ifdef USE_H3

// Declarations of H3 library internals used by DgH3System. They are exported
// by libh3 but not declared in h3api.h. Checked against H3 v4.0.0 through
// v4.5.0, where they did not change (src/h3lib/include/h3Index.h, faceijk.h,
// coordijk.h).
#include <h3api.h>

extern "C" {
   typedef struct { int i; int j; int k; } CoordIJK;
   typedef struct { int face; CoordIJK coord; } FaceIJK;

   H3Index _faceIjkToH3 (const FaceIJK* fijk, int res);
   H3Error _h3ToFaceIjk (H3Index h, FaceIJK* fijk);
}

#endif // USE_H3

#endif
