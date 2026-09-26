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
////////////////////////////////////////////////////////////////////////////////
//
// DgIcosaSliceDice.cpp: DgIcosaSliceDice class implementation
//
// ---------------------------------------------------------------------------
// Parts of the vector formulation (fwdTri / invTri main path, the half-chord
// distance measure halfChord and the signed-excess formula) are ported from
// PROJ (https://github.com/OSGeo/PROJ, commit
// 010d62c2daae01a70247a3e4d6b99bee39879195), files
// src/projections/polyhedral/snyder.h and src/sphere.h. PROJ is distributed
// under the MIT licence:
//
//   Permission is hereby granted, free of charge, to any person obtaining a
//   copy of this software and associated documentation files (the
//   "Software"), to deal in the Software without restriction, including
//   without limitation the rights to use, copy, modify, merge, publish,
//   distribute, sublicense, and/or sell copies of the Software, and to
//   permit persons to whom the Software is furnished to do so, subject to
//   the following conditions:
//
//   The above copyright notice and this permission notice shall be included
//   in all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Those PROJ files carry the following notices, retained here:
//
//   Project:  PROJ
//   Purpose:  Snyder equal-area polyhedral projection -- forward and inverse.
//             / Spherical geometry helpers on 3D unit vectors.
//   Author:   Felix Palmer
//
//   Derived from A5 (Apache-2.0).
//   https://github.com/felixpalmer/a5
//   (modules/projections/polyhedral.ts, modules/utils/vector.ts)
//
//   Copyright (c) A5 contributors
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//   Derived from DGGAL (BSD 3-Clause License)
//   Copyright (c) 2014-2025, Ecere Corporation
//   [full BSD-3 text below]
//
//   Closed-form equations adapted from:
//   https://brsr.github.io/2021/08/31/snyder-equal-area.html
//
// The spherical-trigonometry fallback, the (V, M, C) sub-triangle scheme and
// the radial-vertex role permutation follow DGGAL
// (https://github.com/ecere/dggal, commit
// e16cea7d930e603e09a8310edcd8f58218016e8f),
// src/projections/icoVertexGreatCircle.ec:
//
//   BSD 3-Clause License
//
//   Copyright (c) 2014-2025, Ecere Corporation
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are
//   met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//   3. Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////
//
// THE SLICE-AND-DICE MAP  [VLS06]
// -------------------------------
// Spherical triangle ABC (A radial), planar triangle abc, |ABC| its excess.
// A point P of ABC is described by
//   * the great circle through A and P, which meets arc BC in Q ("slice"):
//     Q is determined by r = |ABQ| / |ABC|, the fraction of the area of ABC
//     lying on B's side of the arc AQ;
//   * its position along AQ ("dice"): h in [0,1].
// The map sends Q to q = (1-r) b + r c on edge bc (so the planar area of abq
// is r * |abc|, matching the sphere) and P to a + h (q - a).
//
// Equal area along the slice fixes h. Take a thin wedge between two slices
// through A. On the sphere, the area of the wedge within angular distance d
// of A is  dtheta (1 - cos d) = 2 dtheta sin^2(d/2);  on the plane the area
// of the wedge within fraction h of the segment aq is h^2 times the whole
// wedge. Since the whole wedges already have proportional areas (by the
// choice of q), equal area within the wedge requires
//        h^2 = sin^2(|AP|/2) / sin^2(|AQ|/2)
//    =>  h   = sin(|AP|/2) / sin(|AQ|/2) = |A - P| / |A - Q|      (chord ratio)
// [Rec21]; this is Snyder's rho ~ sin(z/2) of [Sny92] in vector form. Great
// circles through A map to straight lines through a by construction, and the
// map is unique given ABC, abc and the choice of A [VLS06].
//
// Planar barycentric coordinates of the image w.r.t. (a, b, c):
//        (1 - h,  h (1 - r),  h r).
//
// Inverse: from planar barycentrics (u, v, w): h = v + w = 1 - u, r = w / h,
// alpha = r |ABC| = |ABQ|. Q on arc BC with |ABQ| = alpha in closed form
// [Rec21]: write Q = B cos(phi) + U sin(phi), U the unit tangent at B toward
// C. With c01 = A.B, c12 = B.C, c20 = C.A, s12 = |B x C|, tp = A.(B x C), the
// signed-excess formula
//        tan(E/2) = A.(B x Q) / (1 + A.B + B.Q + Q.A)
// becomes, with t = tan(phi/2),
//        tan(alpha/2) = t tp / (s12 (1 + c01) + t (c20 - c01 c12)),
// solved for t and multiplied through by sin(alpha)
// (tan(alpha/2) = (1 - cos alpha)/sin alpha):
//        phi = 2 atan2(g, f),
//        f = sin(alpha) tp + (1 - cos alpha)(c01 c12 - c20),
//        g = (1 - cos alpha) s12 (1 + c01).
// (1 - cos alpha is evaluated as 2 sin^2(alpha/2) to avoid cancellation.)
// Both f and g are even under the winding flip (alpha, tp) -> (-alpha, -tp),
// so the formula is winding-invariant. Finally P lies on arc AQ at angle
// theta = 2 asin(h sin(|AQ|/2)) from A.
//
// Latitudes and angles are computed only with atan2 / 3-vector forms (never
// asin/acos of a quantity near +-1), which keeps full precision near the
// poles, the face centres and the vertices.
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include <dglib/DgIcosaSliceDice.h>

typedef DgIcosaSliceDice::V3 V3;
typedef DgIcosaSliceDice::V2 V2;
typedef DgIcosaSliceDice::Status Status;

////////////////////////////////////////////////////////////////////////////////
// vector helpers
////////////////////////////////////////////////////////////////////////////////
namespace {

inline V3 add (const V3& a, const V3& b)
   { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline V3 sub (const V3& a, const V3& b)
   { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline V3 scale (const V3& a, long double s)
   { return { a.x * s, a.y * s, a.z * s }; }
inline long double dot (const V3& a, const V3& b)
   { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline V3 cross (const V3& a, const V3& b)
   { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x }; }
inline long double norm (const V3& a) { return sqrtl(dot(a, a)); }
inline V3 normalize (const V3& a) { return scale(a, M_ONE / norm(a)); }
inline bool finite3 (const V3& a)
   { return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z); }

inline V2 add2 (const V2& a, const V2& b) { return { a.x + b.x, a.y + b.y }; }
inline V2 sub2 (const V2& a, const V2& b) { return { a.x - b.x, a.y - b.y }; }
inline V2 scale2 (const V2& a, long double s) { return { a.x * s, a.y * s }; }
inline long double cross2 (const V2& a, const V2& b)
   { return a.x * b.y - a.y * b.x; }

const long double NaN = NAN;

// sin(angle(a, b) / 2) for unit a, b, computed as |a - b| / |(a - b, a + b)|
// (PROJ vector_difference): accurate for tiny angles and self-normalising
// against |a|, |b| != 1 by rounding.
inline long double halfChord (const V3& a, const V3& b)
{
   const long double D = norm(sub(a, b));
   const long double S = norm(add(a, b));
   return D / hypotl(D, S);
}

// Planar barycentrics of x w.r.t. (a, b, c); each is a ratio of signed areas,
// so the result does not depend on the winding of abc.
void planarBary (const V2& x, const V2& a, const V2& b, const V2& c,
                 long double bary[3])
{
   const long double det = cross2(sub2(b, a), sub2(c, a));
   bary[0] = cross2(sub2(b, x), sub2(c, x)) / det;
   bary[1] = cross2(sub2(c, x), sub2(a, x)) / det;
   bary[2] = cross2(sub2(a, x), sub2(b, x)) / det;
}

// The degenerate-input rule (see DgIcosaSliceDice::clampTol) applied to n
// barycentric-like coordinates.
Status clampRule (long double* b, int n, bool renormalise)
{
   Status st = Status::OK;
   for (int i = 0; i < n; i++)
   {
      if (!std::isfinite(b[i])) return Status::Fail;
      if (b[i] < M_ZERO)
      {
         if (b[i] < -DgIcosaSliceDice::clampTol) return Status::Fail;
         b[i] = M_ZERO;
         st = Status::Clamped;
      }
      else if (!renormalise && b[i] > M_ONE)
      {
         // fractions in [0,1] (not a partition of unity): the same rule at
         // the upper end
         if (b[i] > M_ONE + DgIcosaSliceDice::clampTol) return Status::Fail;
         b[i] = M_ONE;
         st = Status::Clamped;
      }
   }
   if (renormalise && st == Status::Clamped)
   {
      long double s = M_ZERO;
      for (int i = 0; i < n; i++) s += b[i];
      for (int i = 0; i < n; i++) b[i] /= s;
   }
   return st;
}

inline Status worse (Status a, Status b)
   { return (int) a > (int) b ? a : b; }

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
long double
DgIcosaSliceDice::angleBetween (const V3& a, const V3& b)
{
   return 2.0L * atan2l(norm(sub(a, b)), norm(add(a, b)));
}

////////////////////////////////////////////////////////////////////////////////
// Signed spherical excess of (v1, v2, v3) [Rec21, "vector spherical geometry"]
//   tan(E/2) = v1.(v2 x v3) / (1 + v1.v2 + v2.v3 + v3.v1)
// E > 0 iff (v1, v2, v3) is counter-clockwise seen from outside the sphere.
// The triple product is formed with edge differences, v1.((v2-v1) x (v3-v1)),
// which is algebraically equal and loses no relative accuracy for small
// triangles.
long double
DgIcosaSliceDice::sphTriArea (const V3& v1, const V3& v2, const V3& v3)
{
   const long double num = dot(v1, cross(sub(v2, v1), sub(v3, v1)));
   const long double den = M_ONE + dot(v1, v2) + dot(v2, v3) + dot(v3, v1);
   return 2.0L * atan2l(num, den);
}

////////////////////////////////////////////////////////////////////////////////
V3
DgIcosaSliceDice::llToVec (const GeoCoord& ll)
{
   const long double cl = cosl(ll.lat);
   return { cl * cosl(ll.lon), cl * sinl(ll.lon), sinl(ll.lat) };
}

////////////////////////////////////////////////////////////////////////////////
GeoCoord
DgIcosaSliceDice::vecToLL (const V3& v)
{
   GeoCoord ll;
   const long double rxy = hypotl(v.x, v.y);
   ll.lat = atan2l(v.z, rxy);
   // at the poles longitude is undefined: report 0 (DGGRID convention)
   ll.lon = (rxy == M_ZERO) ? M_ZERO : atan2l(v.y, v.x);
   return ll;
}

////////////////////////////////////////////////////////////////////////////////
void
DgIcosaSliceDice::prepare (Tri& t, const V3& A, const V3& B, const V3& C,
                           const V2& a, const V2& b, const V2& c)
{
   t.A = A; t.B = B; t.C = C;
   t.a = a; t.b = b; t.c = c;
   t.nBC = cross(B, C);
   t.tp  = dot(A, t.nBC);
   t.c01 = dot(A, B);
   t.c12 = dot(B, C);
   t.c20 = dot(C, A);
   t.s12 = norm(t.nBC);
   t.arcBC = angleBetween(B, C);
   t.arcAB = angleBetween(A, B);
   t.area = sphTriArea(A, B, C);
   // unit tangent at B toward C: component of C orthogonal to B
   t.uB = normalize(sub(C, scale(B, t.c12)));
   // interior angle at B between the arcs BA and BC
   const V3 tBA = normalize(sub(A, scale(B, t.c01)));
   t.angB = angleBetween(tBA, t.uB);

} // void DgIcosaSliceDice::prepare

////////////////////////////////////////////////////////////////////////////////
// Forward slice-and-dice [VLS06], vector form [Rec21] (PROJ snyder_fwd).
////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::fwdTri (const V3& P, const Tri& t, V2& out)
{
   out = { NaN, NaN };
   if (!finite3(P)) return Status::Fail;

   // radial-vertex coincidence: the slice through A is undefined but every
   // slice gives h = 0, so P = A maps to a exactly.
   const long double kAP = halfChord(t.A, P);              // sin(|AP|/2)
   if (kAP == M_ZERO) { out = t.a; return Status::OK; }

   // Q = intersection of the great circle A-P with the great circle B-C. The
   // planes have normals n1 = A x P (formed as A x (P - A), equal and more
   // accurate when P is near A) and nBC = B x C; their intersection line is
   // n1 x nBC (a "quadruple product"). HEMISPHERE RULE: the line meets the
   // sphere in two antipodal points, and which one n1 x nBC gives depends on
   // the windings of (A, P) and (B, C). The arc BC is < 180 deg and Q lies
   // on it, so the correct Q is the one on the same side as the midpoint
   // direction B + C: flip if Q . (B + C) < 0.
   const V3 n1 = cross(t.A, sub(P, t.A));
   V3 Q = cross(n1, t.nBC);
   const long double qn = norm(Q);
   if (!std::isfinite(qn)) return Status::Fail;
   if (qn == M_ZERO)
   {
      // P - A parallel to A: P = +-A up to rounding. Near +A the slice is
      // undefined and irrelevant (h ~ 0): return a. Near -A (the antipode)
      // P is certainly outside the triangle: fail.
      if (dot(P, t.A) > M_ZERO) { out = t.a; return Status::Clamped; }
      return Status::Fail;
   }
   Q = scale(Q, M_ONE / qn);
   if (dot(Q, add(t.B, t.C)) < M_ZERO) Q = scale(Q, -M_ONE);

   // dice: chord ratio, see the derivation at the top of this file.
   // DIRECTION RULE: Q was chosen on BC's side, but P may lie on the other
   // half of the great circle (behind A as seen from Q): then A x P and A x Q
   // point opposite ways, and the true "h" is negative, which the
   // degenerate-input rule below rejects unless |h| is within rounding of 0.
   long double h = kAP / halfChord(t.A, Q);
   if (dot(n1, cross(t.A, Q)) < M_ZERO) h = -h;

   // slice: area fraction r = |ABQ| / |ABC| (both signed with the same
   // winding, so r >= 0 inside); B x (Q - B) form for accuracy near B.
   long double r;
   {
      const long double num = dot(t.A, cross(t.B, sub(Q, t.B)));
      const long double den = M_ONE + t.c01 + dot(t.B, Q) + dot(Q, t.A);
      r = 2.0L * atan2l(num, den) / t.area;
   }

   // barycentrics (1 - h, h (1 - r), h r) w.r.t. (a, b, c), subjected to the
   // degenerate-input rule. Applying the rule to the barycentrics (not to h
   // and r separately) is what makes P ~ A benign: there r is meaningless
   // (the slice direction is lost to rounding) but its weight h is ~0.
   long double bc[3];
   bc[2] = h * r;
   bc[1] = h - bc[2];
   bc[0] = M_ONE - h;
   const Status st = clampRule(bc, 3, true);
   if (st == Status::Fail) return st;
   out.x = bc[0] * t.a.x + bc[1] * t.b.x + bc[2] * t.c.x;
   out.y = bc[0] * t.a.y + bc[1] * t.b.y + bc[2] * t.c.y;
   return st;

} // Status DgIcosaSliceDice::fwdTri

////////////////////////////////////////////////////////////////////////////////
// Spherical-trigonometry fallback for the "slice" step of the inverse (DGGAL
// icoVertexGreatCircle.ec, trigonometric path; Lee & Mortari 2017 via
// J. Hall's thesis, p. 39). Given triangle ABQ with known side c = |AB|,
// angle beta at B and excess E = alpha, find the angle delta at A:
//    E = delta + beta + angQ - pi,  Y := E + pi - beta = delta + angQ,
//    cos(angQ) = -cos(delta) cos(beta) + sin(delta) sin(beta) cos(c)
//    => tan(delta) = -(cos Y + cos beta) / (sin Y - sin beta cos c).
// Using cos Y + cos beta = -2 sin(E/2) sin(beta - E/2) (sum-to-product;
// avoids the cancellation for small E) and sin Y = sin(beta - E):
//    delta = atan2(2 sin(E/2) sin(beta - E/2), sin(beta - E) - sin beta cos c).
// Q is then the intersection of arc BC with the great circle leaving A at
// angle delta from AB toward AC (same hemisphere rule as the forward).
////////////////////////////////////////////////////////////////////////////////
static bool
trigSliceQ (const DgIcosaSliceDice::Tri& t, long double r, V3& Q)
{
   const long double E = fabsl(r * t.area);
   const long double beta = t.angB;
   const long double delta =
         atan2l(2.0L * sinl(E / 2.0L) * sinl(beta - E / 2.0L),
                sinl(beta - E) - sinl(beta) * cosl(t.arcAB));
   // orthonormal tangent frame at A: tB toward B, tP toward C's side
   const V3 tB = normalize(sub(t.B, scale(t.A, t.c01)));
   const V3 tC = sub(t.C, scale(t.A, t.c20));
   const V3 tP = normalize(sub(tC, scale(tB, dot(tC, tB))));
   const V3 d = add(scale(tB, cosl(delta)), scale(tP, sinl(delta)));
   Q = cross(cross(t.A, d), t.nBC);
   const long double qn = norm(Q);
   if (!(qn > M_ZERO)) return false;
   Q = scale(Q, M_ONE / qn);
   if (dot(Q, add(t.B, t.C)) < M_ZERO) Q = scale(Q, -M_ONE);   // hemisphere
   return finite3(Q);

} // static bool trigSliceQ

////////////////////////////////////////////////////////////////////////////////
// Inverse slice-and-dice (PROJ snyder_inv / DGGAL inverseVector, [Rec21]).
////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::invTri (const V2& X, const Tri& t, V3& out, bool forceTrig)
{
   out = { NaN, NaN, NaN };
   if (!std::isfinite(X.x) || !std::isfinite(X.y)) return Status::Fail;

   long double bc[3];
   planarBary(X, t.a, t.b, t.c, bc);
   Status st = clampRule(bc, 3, true);
   if (st == Status::Fail) return st;

   const long double h = bc[1] + bc[2];            // = 1 - u, accurate near A
   if (h == M_ZERO) { out = t.A; return st; }      // P = A exactly
   long double r = bc[2] / h;
   if (r > M_ONE) r = M_ONE;                       // rounding only (bc >= 0)

   // --- slice: Q on arc BC with |ABQ| = r |ABC| ---
   V3 Q;
   bool ok = false;
   if (!forceTrig)
   {
      const long double alpha = r * t.area;         // signed like tp
      const long double sa = sinl(alpha);
      const long double hs = sinl(alpha / 2.0L);
      const long double cc = 2.0L * hs * hs;        // 1 - cos(alpha)
      const long double f = sa * t.tp + cc * (t.c01 * t.c12 - t.c20);
      const long double g = cc * t.s12 * (M_ONE + t.c01);
      // f = g = 0 happens only for alpha == 0 exactly, where atan2(0, 0) = 0
      // gives Q = B, the correct answer. A non-finite result, or f < 0 with
      // g == 0 (phi = pi: impossible for a valid triangle) is degenerate.
      if (std::isfinite(f) && std::isfinite(g) && !(g == M_ZERO && f < M_ZERO))
      {
         const long double phi = 2.0L * atan2l(g, f);   // arc |BQ|
         Q = add(scale(t.B, cosl(phi)), scale(t.uB, sinl(phi)));
         ok = finite3(Q) && phi <= t.arcBC + clampTol;
      }
   }
   if (!ok)
   {
      if (!trigSliceQ(t, r, Q)) return Status::Fail;
      st = worse(st, Status::Fallback);
   }

   // --- dice: P on arc AQ with sin(|AP|/2) = h sin(|AQ|/2) ---
   const long double k = halfChord(t.A, Q);
   const long double theta = 2.0L * asinl(h * k);   // h k <= sin(|AQ|/2) < 1
   const long double aq = dot(t.A, Q);
   const V3 W = sub(Q, scale(t.A, aq));            // tangent at A toward Q
   const long double wn = norm(W);
   if (!(wn > M_ZERO)) return Status::Fail;        // Q = +-A: impossible
   out = add(scale(t.A, cosl(theta)), scale(W, sinl(theta) / wn));
   if (!finite3(out)) { out = { NaN, NaN, NaN }; return Status::Fail; }
   return st;

} // Status DgIcosaSliceDice::invTri

////////////////////////////////////////////////////////////////////////////////
const char*
DgIcosaSliceDice::radialVertexName (RadialVertex rv)
{
   switch (rv)
   {
      case RadialVertex::FaceCentre:   return "ISEA";
      case RadialVertex::Vertex:       return "IVEA";
      case RadialVertex::EdgeMidpoint: return "RTEA";
   }
   return "?";
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// THE ICOSAHEDRON
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// DgProjTriRF.cpp, DgSphIcosa::ico12verts(): face f has vertices
// icoverts[kFaceVerts[f][0..2]] = icotri[f][0..2].
const int DgIcosaSliceDice::kFaceVerts[20][3] = {
   {  0,  1,  2 }, {  0,  2,  3 }, {  0,  3,  4 }, {  0,  4,  5 },
   {  0,  5,  1 }, {  6,  2,  1 }, {  7,  3,  2 }, {  8,  4,  3 },
   {  9,  5,  4 }, { 10,  1,  5 }, {  2,  6,  7 }, {  3,  7,  8 },
   {  4,  8,  9 }, {  5,  9, 10 }, {  1, 10,  6 }, { 11,  7,  6 },
   { 11,  8,  7 }, { 11,  9,  8 }, { 11, 10,  9 }, { 11,  6, 10 }
};

////////////////////////////////////////////////////////////////////////////////
// FACE FRAME TABLE. Image of icotri[f][j] in DGGRID's ProjTri frame (unit
// edge), the same for all faces: j=0 apex (0.5, sqrt3/2), j=1 (0, 0),
// j=2 (1, 0). The centroid (0.5, sqrt3/6) and the edge midpoints are derived
// from these. The frame is orientation-preserving: (V0, V1, V2) winds
// counter-clockwise seen from outside the sphere, as in the plane.
////////////////////////////////////////////////////////////////////////////////
static const V2 kFrame[3] = {
   { M_HALF, M_SQRT3_2 },
   { M_ZERO, M_ZERO },
   { M_ONE,  M_ZERO }
};

V2 DgIcosaSliceDice::framePoint (int j) { return kFrame[j]; }

V2 DgIcosaSliceDice::frameMid (int k)   // midpoint of the edge opposite j=k
{
   const int i = (k + 1) % 3, j = (k + 2) % 3;
   return scale2(add2(kFrame[i], kFrame[j]), M_HALF);
}

V2 DgIcosaSliceDice::frameCentre (void)
{
   // (p0 + p1 + p2) / 3; the exact form of (0.5, sqrt3/6) with this table
   const V2 s = add2(add2(kFrame[0], kFrame[1]), kFrame[2]);
   return { s.x / 3.0L, s.y / 3.0L };
}

////////////////////////////////////////////////////////////////////////////////
// SUB-TRIANGLE INDEX CONVENTION (one table; used by both locators).
// Sub-triangle s of a face is (V_i, M_ij, C), i = kSubVerts[s][0] the face
// vertex (icotri index), j = kSubVerts[s][1] the other end of the edge whose
// midpoint it uses; s = 2 i + (j == i+1 mod 3 ? 0 : 1). It is the set of
// points of the face whose nearest face vertex is V_i and second nearest V_j
// (the medians of an equilateral triangle are the perpendicular bisectors of
// the opposite edges), on the sphere and in the plane alike.
////////////////////////////////////////////////////////////////////////////////
const int DgIcosaSliceDice::kSubVerts[6][2] = {
   { 0, 1 }, { 0, 2 }, { 1, 2 }, { 1, 0 }, { 2, 0 }, { 2, 1 }
};
static const int kSubIndex[3][3] = {   // [i][j] -> s  (i != j)
   { -1, 0, 1 }, { 3, -1, 2 }, { 4, 5, -1 }
};

////////////////////////////////////////////////////////////////////////////////
// RADIAL-VERTEX ROLE TABLE (the only place ISEA / IVEA / RTEA differ).
// kRoles[mode] = sub-triangle vertex (0 = V, 1 = M, 2 = C) playing kernel
// role { A (radial), B, C }. The order of B and C is immaterial to the kernel
// (winding-invariant), fixed here for definiteness.
////////////////////////////////////////////////////////////////////////////////
const int DgIcosaSliceDice::kRoles[3][3] = {
   /* FaceCentre   ISEA */ { 2, 0, 1 },   // A = C, B = V, C = M
   /* Vertex       IVEA */ { 0, 1, 2 },   // A = V, B = M, C = C
   /* EdgeMidpoint RTEA */ { 1, 2, 0 }    // A = M, B = C, C = V
};

V2 DgIcosaSliceDice::subPlanar (int s, int vmc)
{
   const int i = kSubVerts[s][0], j = kSubVerts[s][1];
   switch (vmc)
   {
      case 0:  return kFrame[i];
      case 1:  return frameMid(3 - i - j);
      default: return frameCentre();
   }
}

////////////////////////////////////////////////////////////////////////////////
DgIcosaSliceDice::DgIcosaSliceDice (RadialVertex rv, const SphIcosa& si)
   : rv_ (rv)
{
   // the 12 vertices, as unit vectors (renormalised: changes <= 1 ulp; the
   // kernel formulas assume unit vectors). DgSphIcosa computes icoverts with
   // an exact (atan2) rotation, so this is a regular icosahedron to a few ulp.
   for (int i = 0; i < 12; i++) vert_[i] = normalize(llToVec(si.icoverts[i]));

   for (int f = 0; f < 20; f++)
   {
      const V3& v0 = vert_[kFaceVerts[f][0]];
      const V3& v1 = vert_[kFaceVerts[f][1]];
      const V3& v2 = vert_[kFaceVerts[f][2]];

      // centre: normalised vertex mean
      cen_[f] = normalize(add(add(v0, v1), v2));
      for (int k = 0; k < 3; k++)
      {
         const V3& vi = vert_[kFaceVerts[f][(k + 1) % 3]];
         const V3& vj = vert_[kFaceVerts[f][(k + 2) % 3]];
         mid_[f][k] = normalize(add(vi, vj));
         // median plane through V_k, C_f, M_k: the perpendicular-bisector
         // plane of edge (V_i, V_j), normal V_i - V_j (|V_i| = |V_j|).
         // P . medN_[f][k] > 0  <=>  P is nearer V_i than V_j.
         medN_[f][k] = sub(vi, vj);
      }

      // neighbour across the edge (V_i, V_j) opposite V_k, and the
      // orientation-preserving similarity with T(p_f(i)) = p_g(i'),
      // T(p_f(j)) = p_g(j') (both frames are CCW seen from outside, so the
      // unfolding of the net is a rotation + translation).
      for (int k = 0; k < 3; k++)
      {
         const int i = (k + 1) % 3, j = (k + 2) % 3;
         const int u = kFaceVerts[f][i], v = kFaceVerts[f][j];
         nbr_[f][k] = -1;
         for (int g = 0; g < 20 && nbr_[f][k] < 0; g++)
         {
            if (g == f) continue;
            int a = -1, b = -1;
            for (int m = 0; m < 3; m++)
            {
               if (kFaceVerts[g][m] == u) a = m;
               if (kFaceVerts[g][m] == v) b = m;
            }
            if (a < 0 || b < 0) continue;
            nbr_[f][k] = g;
            const V2 p1 = kFrame[i], p2 = kFrame[j];
            const V2 q1 = kFrame[a], q2 = kFrame[b];
            const long double dpr = p2.x - p1.x, dpi = p2.y - p1.y;
            const long double dqr = q2.x - q1.x, dqi = q2.y - q1.y;
            const long double den = dpr * dpr + dpi * dpi;
            long double* T = nbrSim_[f][k];
            T[0] = (dqr * dpr + dqi * dpi) / den;
            T[1] = (dqi * dpr - dqr * dpi) / den;
            T[2] = q1.x - (T[0] * p1.x - T[1] * p1.y);
            T[3] = q1.y - (T[0] * p1.y + T[1] * p1.x);
         }
      }

      // the 6 sub-triangles, in kernel role order
      const int* R = kRoles[(int) rv_];
      for (int s = 0; s < 6; s++)
      {
         const int i = kSubVerts[s][0], j = kSubVerts[s][1];
         sub_[f][s][0] = vert_[kFaceVerts[f][i]];    // V
         sub_[f][s][1] = mid_[f][3 - i - j];         // M
         sub_[f][s][2] = cen_[f];                    // C
         prepare(tri_[f][s],
                 sub_[f][s][R[0]], sub_[f][s][R[1]], sub_[f][s][R[2]],
                 subPlanar(s, R[0]), subPlanar(s, R[1]), subPlanar(s, R[2]));
      }
   }

} // DgIcosaSliceDice::DgIcosaSliceDice

////////////////////////////////////////////////////////////////////////////////
// Forward locator 1: face = nearest face centre (the equivalent of
// DgSphIcosa::whichIcosaTri, by largest dot product). Ties (points exactly
// on a face edge) go to the lowest face index; both faces give the same
// point, since the map is continuous across face edges.
////////////////////////////////////////////////////////////////////////////////
int
DgIcosaSliceDice::whichFace (const V3& p) const
{
   int best = 0;
   long double bestDot = dot(cen_[0], p);
   for (int f = 1; f < 20; f++)
   {
      const long double d = dot(cen_[f], p);
      if (d > bestDot) { bestDot = d; best = f; }
   }
   return best;
}

////////////////////////////////////////////////////////////////////////////////
// Ranking of the three face vertices from three signed "i nearer than j"
// tests, sgn[k] for the pair (i, j) = (k+1, k+2) mod 3. Returns the
// sub-triangle index. Ties (a value of exactly 0: a point on a median)
// resolve to the lower vertex index; either side is a correct answer because
// the map is continuous across medians.
static int
rankToSub (const long double sgn[3])
{
   auto nearer = [&](int i, int j) -> bool {
      const int k = 3 - i - j;
      // sgn[k] refers to (i', j') = (k+1, k+2); flip if (i, j) is reversed
      const long double v = (i == (k + 1) % 3) ? sgn[k] : -sgn[k];
      return v > M_ZERO || (v == M_ZERO && i < j);
   };
   int wins[3] = { 0, 0, 0 };
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         if (i != j && nearer(i, j)) wins[i]++;
   int i = 0;
   for (int q = 1; q < 3; q++) if (wins[q] > wins[i]) i = q;
   const int o1 = (i + 1) % 3, o2 = (i + 2) % 3;
   const int j = nearer(o1, o2) ? o1 : o2;
   return kSubIndex[i][j];
}

////////////////////////////////////////////////////////////////////////////////
// Forward locator 2: sub-triangle from the signs of p against the 3 median
// great-circle planes of the face.
int
DgIcosaSliceDice::whichSubTriSph (int face, const V3& p) const
{
   long double sgn[3];
   for (int k = 0; k < 3; k++) sgn[k] = dot(p, medN_[face][k]);
   return rankToSub(sgn);
}

////////////////////////////////////////////////////////////////////////////////
// Inverse locator: the same test on the planar medians, via the planar
// barycentrics b of x in the face triangle: "V_i nearer than V_j" <=>
// b_i > b_j (the planar median through p_k is the locus b_i = b_j).
int
DgIcosaSliceDice::whichSubTriPlanar (const V2& x)
{
   long double b[3];
   planarBary(x, kFrame[0], kFrame[1], kFrame[2], b);
   long double sgn[3];
   for (int k = 0; k < 3; k++) sgn[k] = b[(k + 1) % 3] - b[(k + 2) % 3];
   return rankToSub(sgn);
}

////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::forwardVec (const V3& p0, int& face, V2& xy,
                              int* subOut) const
{
   face = -1;
   xy = { NaN, NaN };
   if (!finite3(p0)) return Status::Fail;
   const long double n = norm(p0);
   if (!(n > M_ZERO)) return Status::Fail;
   const V3 p = scale(p0, M_ONE / n);
   face = whichFace(p);
   int s = whichSubTriSph(face, p);
   Status st = forwardInSub(p, face, s, xy);
   if (st == Status::Fail)
   {
      // LOCATOR FALLBACK. For a regular icosahedron the nearest-centre face
      // and the median-plane sub-triangle are exact, so this is reached only
      // if the icosahedron is irregular beyond clampTol. The 120 triangles
      // still tile the sphere: search them all and take the first that
      // contains p under the degenerate-input rule.
      for (int g = 0; g < 20 && st == Status::Fail; g++)
         for (int t = 0; t < 6 && st == Status::Fail; t++)
         {
            st = forwardInSub(p, g, t, xy);
            if (st != Status::Fail) { face = g; s = t; }
         }
   }
   if (subOut) *subOut = s;
   if (st == Status::Fail) face = -1;
   return st;

} // Status DgIcosaSliceDice::forwardVec

////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::forward (const GeoCoord& ll, int& face, Vec2D& xy,
                           int* subOut) const
{
   V2 v = { NaN, NaN };
   Status st = Status::Fail;
   face = -1;
   if (std::isfinite(ll.lon) && std::isfinite(ll.lat) &&
       fabsl(ll.lat) <= M_PI_2_L + clampTol)
      st = forwardVec(llToVec(ll), face, v, subOut);

   xy.x = v.x;
   xy.y = v.y;
   return st;

} // Status DgIcosaSliceDice::forward

////////////////////////////////////////////////////////////////////////////////
V2
DgIcosaSliceDice::toNeighbourFrame (int f, int k, const V2& xy) const
{
   const long double* T = nbrSim_[f][k];
   return { T[0] * xy.x - T[1] * xy.y + T[2],
            T[0] * xy.y + T[1] * xy.x + T[3] };
}

////////////////////////////////////////////////////////////////////////////////
// planar distance of xy outside the face triangle (<= 0 inside), and the
// index k of the vertex opposite the most-violated edge
static long double
outsideDistance (const V2& xy, int& kWorst)
{
   long double b[3];
   planarBary(xy, kFrame[0], kFrame[1], kFrame[2], b);
   const long double det =
          fabsl(cross2(sub2(kFrame[1], kFrame[0]), sub2(kFrame[2], kFrame[0])));
   long double worst = -HUGE_VALL;
   kWorst = 0;
   for (int k = 0; k < 3; k++)
   {
      // height over the edge opposite k = |det| / |edge|
      const V2 e = sub2(kFrame[(k + 2) % 3], kFrame[(k + 1) % 3]);
      const long double d = -b[k] * det / hypotl(e.x, e.y);
      if (d > worst) { worst = d; kWorst = k; }
   }
   return worst;
}

////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::inverseVec (int face, const V2& xy0, V3& p,
                              bool forceTrig) const
{
   p = { NaN, NaN, NaN };
   if (face < 0 || face >= 20) return Status::Fail;
   if (!std::isfinite(xy0.x) || !std::isfinite(xy0.y)) return Status::Fail;

   // face-level rule (see faceOutsideTol and maxFaceCrossings): carry a
   // point outside the face across the most-violated edge into the
   // neighbour, as often as needed
   V2 xy = xy0;
   int k;
   long double d = outsideDistance(xy, k);
   Status st0 = Status::OK;
   for (int n = 0; d > M_ZERO && n < maxFaceCrossings; n++)
   {
      xy = toNeighbourFrame(face, k, xy);
      face = nbr_[face][k];
      st0 = Status::Clamped;
      d = outsideDistance(xy, k);
   }
   if (d > faceOutsideTol) return Status::Fail;
   if (d > M_ZERO)
   {
      // still outside by a rounding amount: clamp onto the closed triangle
      long double b[3];
      planarBary(xy, kFrame[0], kFrame[1], kFrame[2], b);
      long double sum = M_ZERO;
      for (int m = 0; m < 3; m++)
      {
         if (b[m] < M_ZERO) b[m] = M_ZERO;
         sum += b[m];
      }
      for (int m = 0; m < 3; m++) b[m] /= sum;
      xy.x = b[0] * kFrame[0].x + b[1] * kFrame[1].x + b[2] * kFrame[2].x;
      xy.y = b[0] * kFrame[0].y + b[1] * kFrame[1].y + b[2] * kFrame[2].y;
      st0 = Status::Clamped;
   }
   const Status st = invTri(xy, tri_[face][whichSubTriPlanar(xy)], p,
                            forceTrig);
   if (st == Status::Fail) return st;
   return worse(st, st0);

} // Status DgIcosaSliceDice::inverseVec

////////////////////////////////////////////////////////////////////////////////
Status
DgIcosaSliceDice::inverse (int face, const Vec2D& xy, GeoCoord& ll,
                           bool forceTrig) const
{
   V3 p;
   const V2 v = { xy.x, xy.y };
   const Status st = inverseVec(face, v, p, forceTrig);
   if (st == Status::Fail)
   {
      ll.lat = ll.lon = NaN;
      return st;
   }
   ll = vecToLL(p);
   return st;

} // Status DgIcosaSliceDice::inverse

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
