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

    Parts of the implementation (DgIcosaSliceDice.cpp) are ported from PROJ
    (MIT; itself derived from A5, Apache-2.0, and DGGAL, BSD-3-Clause) and
    from DGGAL (BSD-3-Clause, Copyright (c) 2014-2025, Ecere Corporation).
    The upstream notices are retained in DgIcosaSliceDice.cpp.
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//
// DgIcosaSliceDice.h: DgIcosaSliceDice class definitions
//
// The slice-and-dice equal-area map of a spherical triangle onto a planar
// triangle, and the 120 fundamental triangles of the icosahedron in DGGRID's
// ProjTri face frame. One kernel serves ISEA, IVEA and RTEA; they differ only
// in which vertex of each (V, M, C) sub-triangle is the radial vertex.
//
// References
//   [VLS06] D. van Leeuwen & D. Strebe (2006), "A 'Slice-and-Dice' Approach
//           to Area Equivalence in Polyhedral Map Projections", Cartography
//           and Geographic Information Science 33(4):269-286,
//           doi:10.1559/152304006779500687.
//   [Sny92] J. P. Snyder (1992), "An Equal-Area Map Projection for Polyhedral
//           Globes", Cartographica 29(1):10-21, doi:10.3138/27H7-8K88-4882-1752.
//   [Rec21] B. R. S. Recht (2021), "Snyder equal-area projection, in vector
//           form", https://brsr.github.io/2021/08/31/snyder-equal-area.html
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGICOSA_SLICE_DICE_H
#define DGICOSA_SLICE_DICE_H

#include <dglib/DgConstants.h>
#include <dglib/DgEllipsoidRF.h>

////////////////////////////////////////////////////////////////////////////////
/**
 * The slice-and-dice equal-area kernel [VLS06] on the 120 fundamental
 * triangles of an icosahedron.
 *
 * Each face of the icosahedron is split into 6 right sub-triangles (V, M, C):
 * V a face vertex, M the midpoint of an edge at V, C the face centre. A point
 * P of a sub-triangle ABC (A the radial vertex) is located by the great circle
 * A-P, which meets the opposite side BC at Q ("slice"), and by its position
 * along A-Q ("dice"). Q is mapped so that areas on either side of the slice
 * are preserved, and P so that the area within the slice is preserved. Great
 * circles through A become straight lines through its planar image.
 *
 * The radial vertex selects the projection:
 *   - FaceCentre:   ISEA (Snyder 1992). Used for testing only; DGGRID's ISEA
 *                   is DgProjISEA.
 *   - Vertex:       IVEA (the vertex-oriented slice-and-dice projection).
 *   - EdgeMidpoint: RTEA (rhombic triacontahedral); not exposed.
 *
 * The planar images use DGGRID's ProjTri face frame: unit edge,
 * icotri[f][0] at (0.5, sqrt(3)/2), icotri[f][1] at (0, 0) and icotri[f][2]
 * at (1, 0).
 *
 * Everything is long double; the geometry is built from the icosahedron
 * vertices of a SphIcosa (see DgSphIcosa, which owns the instances).
 */
class DgIcosaSliceDice {

   public:

      /// The sub-triangle vertex that is radial (maps great circles through
      /// it to straight lines).
      enum class RadialVertex {
         FaceCentre   = 0,   ///< ISEA (test only)
         Vertex       = 1,   ///< IVEA
         EdgeMidpoint = 2    ///< RTEA (not exposed)
      };

      /// Result status of a kernel call.
      enum class Status {
         OK       = 0,   ///< interior or exact boundary point
         Clamped  = 1,   ///< clamped within clampTol, or carried into a
                         ///< neighbouring face (inverse face-level rule)
         Fallback = 2,   ///< the inverse used the spherical-trig fallback
         Fail     = 3    ///< outside the tolerances, or invalid input
      };

      /// 3D vector (unit sphere) and 2D point types used by the kernel.
      struct V3 { long double x, y, z; };
      struct V2 { long double x, y; };

      /**
       * The degenerate-input rule, the one tolerance of the kernel. Every map
       * works through barycentric coordinates b relative to a triangle:
       *    b >= 0                 used unchanged;
       *    -clampTol <= b < 0     clamped to 0, renormalised (Clamped);
       *    b < -clampTol          the call fails (Fail).
       * It is dimensionless (units of the triangle). 1e-12 admits %.17g
       * rounded double input on an 80-bit long double build, and is ~1e11
       * times smaller than any feature of the 120-triangle geometry.
       */
      static constexpr long double clampTol = 1.0e-12L;

      /**
       * The face-level inverse rule. Planar points can reach the inverse
       * outside their face triangle: cell vertices near a face edge at the
       * rounding level, and points given directly in PROJTRI or PLANE
       * coordinates. (Before DgQ2DDtoVertex2DDConverter re-expressed points
       * past a quad's far edge in the neighbouring quad, this also included
       * the vertices of class III aperture 7 cells that straddle a face
       * edge, up to 0.014 of an edge outside at 7H res 3; the rule is kept
       * as a safety net.) Such a
       * point is carried across the most-violated edge by the
       * net-unfolding similarity into the neighbouring face, and inverted
       * there, repeated up to maxFaceCrossings times (status Clamped). This
       * is exact: the result is the neighbour's own inverse of that point
       * of the unfolded net, so the inverse is continuous across face edges
       * and forward(inverse(x)) returns x in the neighbour's frame. A point
       * still outside after that by at most faceOutsideTol (planar distance,
       * unit-edge frame; rounding near a vertex) is clamped onto the closed
       * triangle, moving it by < 2 faceOutsideTol. Otherwise the inverse
       * fails.
       */
      static constexpr int maxFaceCrossings = 4;

      /// Rounding allowance of the face-level rule (see maxFaceCrossings).
      static constexpr long double faceOutsideTol = 1.0e-9L;

      /**
       * One spherical triangle ABC, A radial, with its planar image abc and
       * the quantities that depend only on the triangle. ABC may have either
       * winding; the formulas are winding-invariant.
       */
      struct Tri {
         V3 A, B, C;              ///< spherical vertices (unit), A radial
         V2 a, b, c;              ///< planar images of A, B, C
         long double area;        ///< signed spherical excess of ABC
         long double tp;          ///< triple product A . (B x C)
         long double c01, c12, c20; ///< A.B, B.C, C.A
         long double s12;         ///< |B x C| = sin|BC|
         long double arcBC;       ///< |BC| (rad)
         V3 nBC;                  ///< B x C
         V3 uB;                   ///< unit tangent at B toward C
         long double angB;        ///< interior angle at B (trig fallback)
         long double arcAB;       ///< |AB| (rad) (trig fallback)
      };

      /// Precompute a kernel triangle; A is radial.
      static void prepare (Tri& t, const V3& A, const V3& B, const V3& C,
                           const V2& a, const V2& b, const V2& c);

      /// Forward slice-and-dice: unit vector P in ABC -> planar point in abc.
      static Status fwdTri (const V3& P, const Tri& t, V2& out);

      /// Inverse slice-and-dice: planar point X in abc -> unit vector in
      /// ABC. forceTrig selects the spherical-trigonometry path (testing).
      static Status invTri (const V2& X, const Tri& t, V3& out,
                            bool forceTrig = false);

      /**
       * Build the 120 fundamental triangles of the icosahedron si (the
       * vertices si.icoverts and faces si.icotri, as built by DgSphIcosa)
       * for the given radial vertex.
       */
      DgIcosaSliceDice (RadialVertex rv, const SphIcosa& si);

      RadialVertex radialVertex (void) const { return rv_; }

      /// Human-readable name of a radial vertex mode ("ISEA", "IVEA", "RTEA").
      static const char* radialVertexName (RadialVertex rv);

      /**
       * Forward map. ll is lat/lon in radians. Returns the face (0-19) and
       * the ProjTri (x, y) of that face. On Fail, face is -1 and xy is NaN.
       */
      Status forward (const GeoCoord& ll, int& face, Vec2D& xy,
                      int* subOut = nullptr) const;

      /// Forward map of a (not necessarily unit) vector.
      Status forwardVec (const V3& p, int& face, V2& xy,
                         int* subOut = nullptr) const;

      /// Forward map within a given face / sub-triangle (tests).
      Status forwardInSub (const V3& p, int face, int sub, V2& xy) const
                  { return fwdTri(p, tri_[face][sub], xy); }

      /**
       * Inverse map from (face, x, y) in the ProjTri frame to lat/lon in
       * radians. Applies the face-level rule (maxFaceCrossings). On Fail, ll
       * is NaN.
       */
      Status inverse (int face, const Vec2D& xy, GeoCoord& ll,
                      bool forceTrig = false) const;

      /// Inverse map to a unit vector.
      Status inverseVec (int face, const V2& xy, V3& p,
                         bool forceTrig = false) const;

      /// Face whose centre is nearest p (largest dot product; ties go to
      /// the lower index).
      int whichFace (const V3& p) const;

      /// Sub-triangle of face containing p, from the median planes.
      int whichSubTriSph (int face, const V3& p) const;

      /// Sub-triangle containing planar point x, from the planar medians.
      static int whichSubTriPlanar (const V2& x);

      /// Neighbour of face f across the edge opposite icotri vertex k.
      int neighbourFace (int f, int k) const { return nbr_[f][k]; }

      /// f's planar frame mapped into the frame of neighbourFace(f, k).
      V2 toNeighbourFrame (int f, int k, const V2& xy) const;

      /// Unit vector of icosahedron vertex i (0-11).
      const V3& vertex (int i) const { return vert_[i]; }

      /// Unit vector of the centre of face f.
      const V3& centre (int f) const { return cen_[f]; }

      /// Unit vector of the midpoint of the edge of face f opposite vertex k.
      const V3& midpoint (int f, int k) const { return mid_[f][k]; }

      /// Vertex vmc (0 = V, 1 = M, 2 = C) of sub-triangle s of face f.
      const V3& subSph (int f, int s, int vmc) const { return sub_[f][s][vmc]; }

      /// Prepared kernel triangle s (0-5) of face f.
      const Tri& kernelTri (int f, int s) const { return tri_[f][s]; }

      /// Image of icotri[f][j] in the ProjTri frame.
      static V2 framePoint (int j);

      /// Image of the midpoint of the edge opposite icotri[f][k].
      static V2 frameMid (int k);

      /// Image of the face centre, (0.5, sqrt(3)/6).
      static V2 frameCentre (void);

      /// Planar image of vertex vmc (0 = V, 1 = M, 2 = C) of sub-triangle s.
      static V2 subPlanar (int s, int vmc);

      /// DGGRID's face-vertex table (icotri[f][j] = icoverts[kFaceVerts[f][j]]).
      static const int kFaceVerts[20][3];

      /// Sub-triangle s = (V_i, M_ij, C): kSubVerts[s] = { i, j }.
      static const int kSubVerts[6][2];

      /// Kernel role (A radial, B, C) -> sub-triangle vertex (0 V, 1 M, 2 C),
      /// indexed by RadialVertex.
      static const int kRoles[3][3];

      /// Unit vector from lat/lon (radians).
      static V3 llToVec (const GeoCoord& ll);

      /// Lat/lon (radians) of a vector, from atan2 (accurate at the poles).
      static GeoCoord vecToLL (const V3& v);

      /// Angle between two unit vectors, 2 atan2(|a-b|, |a+b|).
      static long double angleBetween (const V3& a, const V3& b);

      /// Signed spherical excess of the triangle (v1, v2, v3).
      static long double sphTriArea (const V3& v1, const V3& v2, const V3& v3);

   private:

      RadialVertex rv_;
      V3 vert_[12];
      V3 cen_[20];
      V3 mid_[20][3];      // mid_[f][k]: midpoint of the edge opposite vertex k
      V3 medN_[20][3];     // medN_[f][k] = V_i - V_j, (i, j, k) cyclic
      int nbr_[20][3];     // neighbour across the edge opposite vertex k
      // similarity z -> (T0 + i T1) z + (T2 + i T3) from f's frame to the
      // frame of nbr_[f][k] (the net unfolding along the shared edge)
      long double nbrSim_[20][3][4];
      V3 sub_[20][6][3];   // [f][s][V, M, C]
      Tri tri_[20][6];     // role-permuted, prepared kernel triangles
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
