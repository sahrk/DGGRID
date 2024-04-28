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
// DgIVec3D.cpp: DgIVec3D class implementation
//
////////////////////////////////////////////////////////////////////////////////

#include <climits>
#include <string.h>

#include <dglib/DgBase.h>
#include <dglib/DgIVec3D.h>

////////////////////////////////////////////////////////////////////////////////

const DgIVec3D& DgIVec3D::undefDgIVec3D = DgIVec3D(INT_MAX, INT_MAX, INT_MAX);

////////////////////////////////////////////////////////////////////////////////
const char*
DgIVec3D::fromString (const char* str, char delimiter)
{
   char delimStr[2];
   delimStr[0] = delimiter;
   delimStr[1] = '\0';

   char* tmpStr = new char[strlen(str) + 1];
   strcpy(tmpStr, str);

   // Get i, j, and k
   char* tok;

   long long int	iIn(0),
	   		jIn(0),
                        kIn(0);

   try
    {
   	tok = strtok(tmpStr, delimStr);
   	iIn = dgg::util::from_string<long long int>(tok);
	
   	tok = strtok(NULL, delimStr);
   	jIn = dgg::util::from_string<long long int>(tok);

   	tok = strtok(NULL, delimStr);
   	kIn = dgg::util::from_string<long long int>(tok);
    }
   catch(...)
    {
      ::report("DgIVec3D::fromString() invalid value in string " + string(tok),
               DgBase::Fatal);
    }

   setI(iIn);
   setJ(jIn);
   setK(kIn);

   unsigned long long int offset = (tok - tmpStr) + strlen(tok) + 1;
   if (offset >= strlen(str))
    return 0;

   return &str[offset];

} // const char* DgIVec3D::fromString

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
DgIVec3D::ijkPlusNormalize(void) {
    // remove any negative values
    if (i_ < 0) {
        j_ -= i_;
        k_ -= i_;
        i_ = 0;
    }

    if (j_ < 0) {
        i_ -= j_;
        k_ -= j_;
        j_ = 0;
    }

    if (k_ < 0) {
        i_ -= k_;
        j_ -= k_;
        k_ = 0;
    }

    // remove the min value if needed
    int min = i_;
    if (j_ < min) min = j_;
    if (k_ < min) min = k_;
    if (min > 0) {
        i_ -= min;
        j_ -= min;
        k_ -= min;
    }
}

/**
 * Determines the GBT digit corresponding to a unit vector or the zero vector
 * in ijk coordinates.
 *
 * @param ijk The ijk coordinates; must be a unit vector or zero vector.
 * @return The H3 digit (0-6) corresponding to the ijk unit vector, zero vector,
 * or INVALID_DIGIT (7) on failure.
 */
static Direction unitIjkPlusToDigit(const DgIVec3D& ijk) {
    DgIVec3D c = ijk;
    c.ijkPlusNormalize();

    Direction digit = INVALID_DIGIT;
    for (Direction i = CENTER_DIGIT; i < NUM_DIGITS; i++) {
        if (c == UNIT_VECS[i]) {
            digit = i;
            break;
        }
    }

    return digit;
}

/**
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * counter-clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::upAp7(void) {
    // convert to CoordIJ
    int i = i_ - k_;
    int j = j_ - k_;

    i_ = (int)lround((3 * i - j) / 7.0);
    j_ = (int)lround((i + 2 * j) / 7.0);
    k_ = 0;
    ijkPlusNormalize();
}

/**
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::upAp7r(void) {
    // convert to CoordIJ
    int i = i_ - k_;
    int j = j_ - k_;

    i_ = (int)lround((2 * i + j) / 7.0);
    j_ = (int)lround((3 * j - i) / 7.0);
    k_ = 0;
    ijkPlusNormalize();
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 7 counter-clockwise resolution. Works in
 * place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::downAp7(void) {
    // res r unit vectors in res r+1
    DgIVec3D iVec = {3, 0, 1};
    DgIVec3D jVec = {1, 3, 0};
    DgIVec3D kVec = {0, 1, 3};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;
    *this = iVec + jVec + kVec;
/*
    ijkAdd(&iVec, &jVec, ijk);
    ijkAdd(ijk, &kVec, ijk);
*/
    ijkPlusNormalize();
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 7 clockwise resolution. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::downAp7r(void) {
    // res r unit vectors in res r+1
    DgIVec3D iVec = {3, 1, 0};
    DgIVec3D jVec = {0, 3, 1};
    DgIVec3D kVec = {1, 0, 3};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;

    *this = iVec + jVec + kVec;
    ijkPlusNormalize();
/*
    ijkAdd(&iVec, &jVec, ijk);
    ijkAdd(ijk, &kVec, ijk);
*/
}

/**
 * Find the normalized ijk coordinates of the hex in the specified digit
 * direction from the specified ijk coordinates. Works in place.
 *
 * @param ijk The ijk coordinates.
 * @param digit The digit direction from the original ijk coordinates.
 */
void
DgIVec3D::neighbor(Direction digit) {
    if (digit > CENTER_DIGIT && digit < NUM_DIGITS) {
        *this += UNIT_VECS[digit];
        //ijkAdd(ijk, &UNIT_VECS[digit], ijk);
        ijkPlusNormalize();
    }
}

/**
 * Rotates ijk coordinates 60 degrees counter-clockwise. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::ijkRotate60ccw(void) {
    // unit vector rotations
    DgIVec3D iVec = {1, 1, 0};
    DgIVec3D jVec = {0, 1, 1};
    DgIVec3D kVec = {1, 0, 1};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;

    *this = iVec + jVec + kVec;
    ijkPlusNormalize();
}

/**
 * Rotates ijk coordinates 60 degrees clockwise. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::ijkRotate60cw(void) {
    // unit vector rotations
    DgIVec3D iVec = {1, 0, 1};
    DgIVec3D jVec = {1, 1, 0};
    DgIVec3D kVec = {0, 1, 1};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;

    *this = iVec + jVec + kVec;
    ijkPlusNormalize();
}

/**
 * Rotates indexing digit 60 degrees counter-clockwise. Returns result.
 *
 * @param digit Indexing digit (between 1 and 6 inclusive)
 */
static Direction rotate60ccw(Direction digit) {
    switch (digit) {
        case K_AXES_DIGIT:
            return IK_AXES_DIGIT;
        case IK_AXES_DIGIT:
            return I_AXES_DIGIT;
        case I_AXES_DIGIT:
            return IJ_AXES_DIGIT;
        case IJ_AXES_DIGIT:
            return J_AXES_DIGIT;
        case J_AXES_DIGIT:
            return JK_AXES_DIGIT;
        case JK_AXES_DIGIT:
            return K_AXES_DIGIT;
        default:
            return digit;
    }
}

/**
 * Rotates indexing digit 60 degrees clockwise. Returns result.
 *
 * @param digit Indexing digit (between 1 and 6 inclusive)
 */
static Direction rotate60cw(Direction digit) {
    switch (digit) {
        case K_AXES_DIGIT:
            return JK_AXES_DIGIT;
        case JK_AXES_DIGIT:
            return J_AXES_DIGIT;
        case J_AXES_DIGIT:
            return IJ_AXES_DIGIT;
        case IJ_AXES_DIGIT:
            return I_AXES_DIGIT;
        case I_AXES_DIGIT:
            return IK_AXES_DIGIT;
        case IK_AXES_DIGIT:
            return K_AXES_DIGIT;
        default:
            return digit;
    }
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 3 counter-clockwise resolution. Works in
 * place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::downAp3(void) {
    // res r unit vectors in res r+1
    DgIVec3D iVec = {2, 0, 1};
    DgIVec3D jVec = {1, 2, 0};
    DgIVec3D kVec = {0, 1, 2};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;

    *this = iVec + jVec + kVec;
    ijkPlusNormalize();
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 3 clockwise resolution. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::downAp3r(void) {
    // res r unit vectors in res r+1
    DgIVec3D iVec = {2, 1, 0};
    DgIVec3D jVec = {0, 2, 1};
    DgIVec3D kVec = {1, 0, 2};

    iVec *= i_;
    jVec *= j_;
    kVec *= k_;
    *this = iVec + jVec + kVec;
    ijkPlusNormalize();
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 4  resolution. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void
DgIVec3D::downAp4(void) {
    *this *= 2;

    // KEVIN: necessary?
    ijkPlusNormalize();
}
