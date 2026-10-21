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

// USE_H3 is set in MakeIncludes / CMake
#ifdef USE_H3

// Portions copied from H3 v4.5.0 (src/h3lib/include/coordijk.h),
// Copyright 2016-2018, 2020-2022, 2026 Uber Technologies, Inc., licensed under
// the Apache License, Version 2.0 (see LICENSE.H3). Modified by DGGRID
// contributors; each change is marked with a "DGGRID modification" comment.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include <h3api.h>

// constructCell and getIndexDigit were added in H3 4.4.0, but that release
// identifies itself as 4.3.0 (fixed in 4.4.1)
#if (H3_VERSION_MAJOR * 10000 + H3_VERSION_MINOR * 100 + H3_VERSION_PATCH) < 40401
#error DgH3System requires H3 4.4.1 or later
#endif

#include <dglib/DgH3System.h>
#include "DgH3Internal.h"
#include <dglib/DgConstants.h>
#include <dglib/DgIDGGBase.h>
#include <dglib/DgProjIcosaGnomonic.h>

namespace {

// the largest H3 resolution
const int kMaxH3Res = 15;

// H3 resolution 0 (the 122 base cells) is DGGS resolution 2
const int kFirstNdxRes = 2;

// number of H3 base cells; the base cell field of a DIGIT_STRING is
// kBaseCellDigits wide (Z7's field is 2 wide for its 12 base cells)
const int kNumBaseCells = 122;
const int kBaseCellDigits = 3;

const long double M_SQRT3_2_L = 0.8660254037844386467637231707529361834714L;
const long double M_RSIN60_L = 1.1547005383792515290182975610039149112953L;
const long double M_SQRT7_L = 2.6457513110645905905016157536392604257102L;

// DGGRID modification: H3's double Vec2d is replaced by long double
struct Vec2dL { long double x, y; };

/**
 * Normalizes ijk coordinates by setting the components to the smallest possible
 * values. Works in place.
 *
 * This function does not protect against signed integer overflow.
 */
inline void _ijkNormalize(CoordIJK *c) {
    // remove any negative values
    if (c->i < 0) {
        c->j -= c->i;
        c->k -= c->i;
        c->i = 0;
    }

    if (c->j < 0) {
        c->i -= c->j;
        c->k -= c->j;
        c->j = 0;
    }

    if (c->k < 0) {
        c->i -= c->k;
        c->j -= c->k;
        c->k = 0;
    }

    // remove the min value if needed
    int min = c->i;
    if (c->j < min) min = c->j;
    if (c->k < min) min = c->k;
    if (min > 0) {
        c->i -= min;
        c->j -= min;
        c->k -= min;
    }
}

/**
 * Find the center point in 2D cartesian coordinates of a hex.
 */
inline void _ijkToHex2d(const CoordIJK *h, Vec2dL *v) {
    int i = h->i - h->k;
    int j = h->j - h->k;

    v->x = i - 0.5L * j;
    v->y = j * M_SQRT3_2_L;
}

/**
 * Determine the containing hex in ijk+ coordinates for a 2D cartesian
 * coordinate vector (from DGGRID).
 */
inline void _hex2dToCoordIJK(const Vec2dL *v, CoordIJK *h) {
    long double a1, a2;
    long double x1, x2;
    int m1, m2;
    long double r1, r2;

    // quantize into the ij system and then normalize
    h->k = 0;

    a1 = fabsl(v->x);
    a2 = fabsl(v->y);

    // first do a reverse conversion
    x2 = a2 * M_RSIN60_L;
    x1 = a1 + x2 / 2.0L;

    // check if we have the center of a hex
    m1 = (int)x1;
    m2 = (int)x2;

    // otherwise round correctly
    r1 = x1 - m1;
    r2 = x2 - m2;

    if (r1 < 0.5L) {
        if (r1 < 1.0L / 3.0L) {
            if (r2 < (1.0L + r1) / 2.0L) {
                h->i = m1;
                h->j = m2;
            } else {
                h->i = m1;
                h->j = m2 + 1;
            }
        } else {
            if (r2 < (1.0L - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((1.0L - r1) <= r2 && r2 < (2.0L * r1)) {
                h->i = m1 + 1;
            } else {
                h->i = m1;
            }
        }
    } else {
        if (r1 < 2.0L / 3.0L) {
            if (r2 < (1.0L - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((2.0L * r1 - 1.0L) < r2 && r2 < (1.0L - r1)) {
                h->i = m1;
            } else {
                h->i = m1 + 1;
            }
        } else {
            if (r2 < (r1 / 2.0L)) {
                h->i = m1 + 1;
                h->j = m2;
            } else {
                h->i = m1 + 1;
                h->j = m2 + 1;
            }
        }
    }

    // now fold across the axes if necessary

    if (v->x < 0.0L) {
        if ((h->j % 2) == 0)  // even
        {
            long long int axisi = h->j / 2;
            long long int diff = h->i - axisi;
            h->i = (int)(h->i - 2.0L * diff);
        } else {
            long long int axisi = (h->j + 1) / 2;
            long long int diff = h->i - axisi;
            h->i = (int)(h->i - (2.0L * diff + 1));
        }
    }

    if (v->y < 0.0L) {
        h->i = h->i - (2 * h->j + 1) / 2;
        h->j = -1 * h->j;
    }

    _ijkNormalize(h);
}

////////////////////////////////////////////////////////////////////////////////
// DgH3FaceMap: converts between a DGGRID ProjTri location and an H3 FaceIJK of
// resolution r (0-15). The gnomonic ProjTri frame of a face is a similarity of
// H3's res 0 class II Hex2d frame of the same face (see
// DgProjIcosaGnomonic::projTriToHex2d0); the finer H3 resolutions scale it by
// sqrt(7) per resolution, and odd (class III) resolutions rotate the lattice by
// the aperture 7 angle, which the same-sense rotation here matches with
// DgHexC3Grid2D (checked by the lattice residual below).
struct DgH3FaceMap {

   // the lattice coordinates of a continuous ProjTri location; fatal if the
   // location is not (numerically) a lattice point of resolution r, which
   // would mean DGGRID's grid and H3's lattice are not the same
   static FaceIJK toFaceIjk (const DgProjTriCoord& pt, int r)
   {
      DgDVec2D hd = DgProjIcosaGnomonic::projTriToHex2d0(pt.coord());
      Vec2dL h = { hd.x(), hd.y() };

      // scale for the resolution's unit length
      for (int i = 0; i < r; i++) { h.x *= M_SQRT7_L; h.y *= M_SQRT7_L; }

      // class III (odd resolution) lattices are rotated by M_AP7_ROT_RADS;
      // rotate the point by the same angle the other way, as H3 does
      // (theta -= M_AP7_ROT_RADS in _vec3ToHex2d)
      if (r % 2) {
         const long double c = cosl(M_AP7_ROT_RADS), s = sinl(M_AP7_ROT_RADS);
         const long double x = h.x * c + h.y * s;
         const long double y = -h.x * s + h.y * c;
         h.x = x; h.y = y;
      }

      FaceIJK f;
      f.face = pt.triNum();
      _hex2dToCoordIJK(&h, &f.coord);

      Vec2dL back;
      _ijkToHex2d(&f.coord, &back);
      if (fabsl(back.x - h.x) > 1e-6L || fabsl(back.y - h.y) > 1e-6L)
         ::report("H3 lattice mismatch at " + std::string(pt) +
                  " (H3 resolution " + dgg::util::to_string(r) + ")",
                  DgBase::Fatal);

      return f;
   }

   static DgProjTriCoord fromFaceIjk (const FaceIJK& f, int r)
   {
      Vec2dL h;
      _ijkToHex2d(&f.coord, &h);

      if (r % 2) {
         const long double c = cosl(M_AP7_ROT_RADS), s = sinl(M_AP7_ROT_RADS);
         const long double x = h.x * c - h.y * s;
         const long double y = h.x * s + h.y * c;
         h.x = x; h.y = y;
      }

      for (int i = 0; i < r; i++) { h.x /= M_SQRT7_L; h.y /= M_SQRT7_L; }

      return DgProjTriCoord(f.face, DgProjIcosaGnomonic::hex2d0ToProjTri(
                                          DgDVec2D(h.x, h.y)));
   }
};

////////////////////////////////////////////////////////////////////////////////
// Q2DI cell <-> continuous ProjTri location of its centre
DgProjTriCoord
q2diToProjTri (const DgIDGGBase& dgg, const DgQ2DICoord& q2di)
{
   DgLocation* loc = dgg.makeLocation(q2di);
   dgg.projTriRF().convert(loc);
   DgProjTriCoord pt = *dgg.projTriRF().getAddress(*loc);
   delete loc;

   return pt;
}

DgQ2DICoord
projTriToQ2di (const DgIDGGBase& dgg, const DgProjTriCoord& pt)
{
   DgLocation* loc = dgg.projTriRF().makeLocation(pt);
   dgg.convert(loc);
   DgQ2DICoord q2di = *dgg.getAddress(*loc);
   delete loc;

   return q2di;
}

void
checkH3 (H3Error err, const std::string& what)
{
   if (err != E_SUCCESS)
      ::report("H3 " + what + ": " + describeH3Error(err), DgBase::Fatal);
}

// split off the first token, as DgHierNdxIntRF::str2add does; returns the
// token and the remainder of the string (null if none)
const char*
nextToken (const char* str, char delimiter, std::string& tok)
{
   const char* end = str;
   while (*end && *end != delimiter) end++;
   tok.assign(str, end - str);
   if (!*end || !*(end + 1)) return nullptr;

   return end + 1;
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
const int DgH3System::vertToH3BaseCell[12] =
                  { 4, 49, 24, 14, 38, 58, 83, 63, 72, 97, 107, 117 };

////////////////////////////////////////////////////////////////////////////////
void
DgH3System::validateCompatibility (const DgIDGGSBase& dggs,
                                    DgHierNdxFormType form)
{
   if (form != Int64 && form != DigitString)
      ::report("H3 requires INT64 or DIGIT_STRING form", DgBase::Fatal);
   if (dggs.gridTopo() != Hexagon)
      ::report("H3 requires HEXAGON topology", DgBase::Fatal);
   if (dggs.isPure())
      ::report("H3 requires a mixed aperture DGGS", DgBase::Fatal);

   // H3 resolutions 0 through 15 are DGGS resolutions 2 through 17; the grid
   // may stop at any resolution of the fixed sequence, but must reach DGGS
   // resolution 2
   if (dggs.nRes() < 3)
      ::report("H3 requires a DGGS with at least resolutions 0 through 2",
               DgBase::Fatal);

   for (int res = 1; res < dggs.nRes(); ++res) {
      const unsigned int expected = res == 1 ? 4 : (res == 2 ? 3 : 7);
      if (dggs.idggBase(res).aperture() != expected)
         ::report("H3 requires aperture sequence 43 followed by 7s; "
                  "mismatch at resolution " + std::to_string(res), DgBase::Fatal);
   }
}

////////////////////////////////////////////////////////////////////////////////
DgH3System*
DgH3System::makeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                        const std::string& name)
{
   validateCompatibility(dggs, extModeInt ? Int64 : DigitString);
   return new DgH3System(dggs, extModeInt, name);
}

////////////////////////////////////////////////////////////////////////////////
DgH3System::DgH3System (const DgIDGGSBase& dggs, bool extModeInt,
                        const std::string& name)
   : DgHierNdxSystemRFS<DgH3IntRF, DgH3StringRF>(dggs, extModeInt, name,
         kFirstNdxRes, std::min(dggs.nRes() - 1, kFirstNdxRes + kMaxH3Res))
{ }

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord
DgH3IntRF::quantify (const DgQ2DICoord& point) const
{
   const int r = system().hierNdxRFS().sysRes(res());

   FaceIJK fijk = DgH3FaceMap::toFaceIjk(q2diToProjTri(dgg(), point), r);
   H3Index h = _faceIjkToH3(&fijk, r);
   if (h == H3_NULL || !isValidCell(h))
      ::report("DgH3IntRF::quantify(): no valid H3 cell for " +
               std::string(point) + " at H3 resolution " +
               dgg::util::to_string(r), DgBase::Fatal);

   return DgHierNdxIntCoord(h);
}

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgH3IntRF::invQuantify (const DgHierNdxIntCoord& add) const
{
   const int r = system().hierNdxRFS().sysRes(res());

   FaceIJK fijk;
   checkH3(_h3ToFaceIjk(add.value(), &fijk), "index " + add.valString());
   if (getResolution(add.value()) != r)
      ::report("DgH3IntRF::invQuantify(): index " + add.valString() +
               " has H3 resolution " + dgg::util::to_string(getResolution(add.value())) +
               "; expected H3 resolution " + dgg::util::to_string(r), DgBase::Fatal);

   return projTriToQ2di(dgg(), DgH3FaceMap::fromFaceIjk(fijk, r));
}

////////////////////////////////////////////////////////////////////////////////
const char*
DgH3IntRF::str2add (DgHierNdxIntCoord* add, const char* str, char delimiter) const
{
   std::string tok;
   const char* rest = nextToken(str, delimiter, tok);

   // canonical H3 text (lowercase hex without leading zeros) or the zero-padded
   // 16 digit form
   const int r = system().hierNdxRFS().sysRes(res());
   H3Index h = H3_NULL;
   if (tok.empty() || stringToH3(tok.c_str(), &h) != E_SUCCESS || !isValidCell(h))
      ::report("DgH3IntRF::str2add(): invalid H3 index '" + tok + "'", DgBase::Fatal);
   if (getResolution(h) != r)
      ::report("DgH3IntRF::str2add(): H3 index '" + tok + "' has H3 resolution " +
               dgg::util::to_string(getResolution(h)) + "; expected H3 resolution " +
               dgg::util::to_string(r) + " (DGGS resolution " +
               dgg::util::to_string(res()) + ")", DgBase::Fatal);

   if (add) add->setValue(h);

   return rest;
}

////////////////////////////////////////////////////////////////////////////////
std::string
DgH3IntRF::add2str (const DgHierNdxIntCoord& add) const
{
   char buf[17];   // up to 16 hex digits plus the null terminator
   if (h3ToString(add.value(), buf, sizeof buf) != E_SUCCESS)
      return add.valString();

   return std::string(buf);
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord
DgH3StringRF::quantify (const DgQ2DICoord& point) const
{
   const DgHierNdxIntCoord ic = system().intRF()->quantify(point);

   return system().toStringCoord(ic);
}

////////////////////////////////////////////////////////////////////////////////
DgQ2DICoord
DgH3StringRF::invQuantify (const DgHierNdxStringCoord& add) const
{
   return system().intRF()->invQuantify(system().toIntCoord(add));
}

////////////////////////////////////////////////////////////////////////////////
const char*
DgH3StringRF::str2add (DgHierNdxStringCoord* add, const char* str, char delimiter) const
{
   std::string tok;
   const char* rest = nextToken(str, delimiter, tok);

   // a three digit base cell followed by one digit (0-6) per H3 resolution
   const int r = system().hierNdxRFS().sysRes(res());
   const std::string expected = "a " + dgg::util::to_string(kBaseCellDigits) +
         "-digit base cell (000-" + dgg::util::to_string(kNumBaseCells - 1) + ") followed by " +
         dgg::util::to_string(r) + " digits (0-6)";
   if ((int) tok.size() != kBaseCellDigits + r)
      ::report("DgH3StringRF::str2add(): H3 digit string '" + tok + "' for DGGS resolution " +
               dgg::util::to_string(res()) + " must be " + expected, DgBase::Fatal);
   for (char ch : tok)
      if (ch < '0' || ch > '9')
         ::report("DgH3StringRF::str2add(): H3 digit string '" + tok + "' must be " +
                  expected, DgBase::Fatal);
   if (std::stoi(tok.substr(0, kBaseCellDigits)) >= kNumBaseCells)
      ::report("DgH3StringRF::str2add(): H3 digit string '" + tok +
               "' has an invalid base cell; must be " + expected, DgBase::Fatal);
   for (size_t i = kBaseCellDigits; i < tok.size(); i++)
      if (tok[i] > '6')
         ::report("DgH3StringRF::str2add(): H3 digit string '" + tok +
                  "' has an invalid digit; must be " + expected, DgBase::Fatal);

   // validity beyond this (pentagon deleted subsequences) is checked by
   // toIntCoord
   if (add) add->setValue(tok);

   return rest;
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxIntCoord
DgH3System::toIntCoord (const DgHierNdxStringCoord& c, int gridRes) const
{
   const int r = sysRes(gridRes);
   const std::string s = c.valString();
   if ((int) s.size() != kBaseCellDigits + r)
      ::report("DgH3System::toIntCoord(): H3 digit string '" + s +
               "' must have a " + dgg::util::to_string(kBaseCellDigits) +
               "-digit base cell and " + dgg::util::to_string(r) + " digits",
               DgBase::Fatal);

   const int baseCell = std::stoi(s.substr(0, kBaseCellDigits));
   int digits[kMaxH3Res + 1];
   for (int i = 0; i < r; i++) digits[i] = s[kBaseCellDigits + i] - '0';

   H3Index h = H3_NULL;
   checkH3(constructCell(r, baseCell, digits, &h), "digit string " + s);

   return DgHierNdxIntCoord(h);
}

////////////////////////////////////////////////////////////////////////////////
DgHierNdxStringCoord
DgH3System::toStringCoord (const DgHierNdxIntCoord& c, int gridRes) const
{
   const int r = sysRes(gridRes);
   const H3Index h = c.value();
   if (getResolution(h) != r)
      ::report("DgH3System::toStringCoord(): index " + c.valString() +
               " has H3 resolution " + dgg::util::to_string(getResolution(h)) +
               "; expected H3 resolution " + dgg::util::to_string(r), DgBase::Fatal);

   std::string s = dgg::util::to_string(getBaseCellNumber(h), kBaseCellDigits);
   for (int k = 1; k <= r; k++) {
      int digit;
      checkH3(getIndexDigit(h, k, &digit), "index " + c.valString());
      s += (char) ('0' + digit);
   }

   return DgHierNdxStringCoord(s);
}

////////////////////////////////////////////////////////////////////////////////
void
DgH3System::setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                             DgLocation& parent) const
{
   // res has already been verified by the caller
   const int pRes = add.res() - 1;
   H3Index p = H3_NULL;
   checkH3(cellToParent(add.address().intNdx().value(), sysRes(pRes), &p),
           "parent of " + add.address().intNdx().valString());

   DgResAdd<DgHierNdx> pAdd;
   initNdxFromInt(pAdd, pRes, p);
   forceAddress(&parent, pAdd);
}

////////////////////////////////////////////////////////////////////////////////
void
DgH3System::setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                               DgLocVector& children) const
{
   const int chdRes = add.res() + 1;
   const H3Index h = add.address().intNdx().value();

   int64_t n = 0;
   checkH3(cellToChildrenSize(h, sysRes(chdRes), &n),
           "children of " + add.address().intNdx().valString());
   std::vector<H3Index> buf(n);
   checkH3(cellToChildren(h, sysRes(chdRes), buf.data()),
           "children of " + add.address().intNdx().valString());

   children.clearAddress();
   std::vector<DgAddressBase*>& v = children.addressVec();
   for (H3Index c : buf) {
      if (c == H3_NULL) continue;

      DgResAdd<DgHierNdx> chdAdd;
      initNdxFromInt(chdAdd, chdRes, c);
      v.push_back(new DgAddress<DgResAdd<DgHierNdx>>(chdAdd));
   }
}

#endif // USE_H3
