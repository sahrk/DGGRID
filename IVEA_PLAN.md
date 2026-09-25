# Plan: Adding the IVEA (Slice-and-Dice) Projection to DGGRID

Status: draft plan, 2026-09-25. Branch: `ivea`.
Scope for now: the **spherical** projection only. Ellipsoidal ISEA/IVEA will come
later, built on the authalic-latitude work another agent is doing (see Phase 7).

---

## 0. Background and key findings (this shapes the design)

### 0.1 What IVEA is

IVEA stands for Icosahedral Vertex-oriented great-circle Equal-Area. It applies
the slice-and-dice construction of van Leeuwen & Strebe (2006,
doi:10.1559/152304006779500687) to the icosahedron.

Split each icosahedron face into its **6 right sub-triangles** (V, M, C). V is a face
vertex, M is an edge midpoint and C is the face centre. Across the 20 faces this gives
the 120 fundamental triangles of the spherical disdyakis triacontahedron, which is the
fundamental domain of the icosahedral symmetry group I_h.

Slice-and-dice maps a spherical triangle ABC onto a planar triangle A'B'C' so that:

1. area is preserved (up to a global constant), and
2. every great-circle arc through the chosen **radial vertex** A maps to a straight
   line through A'.

For a given sub-triangle and radial vertex, the map is **unique**. The equal-area
condition fixes where each radial line meets the opposite edge. It also fixes the
radial distance along that line.

The consequence:

| Projection | Radial vertex of each (V, M, C) sub-triangle | Angles (V, M, C) |
|------------|----------------------------------------------|------------------|
| **ISEA** (Snyder 1992) | C, the face centre | 36°, 90°, 60° |
| **IVEA** | V, the icosahedron vertex | same |
| RTEA (rhombic triacontahedron) | M, the edge midpoint | same |

**ISEA and IVEA use the same 120 sub-triangles and the same kernel. The only
difference is which vertex is radial.** DGGAL implements all three as one class,
`SliceAndDiceGreatCircleIcosahedralProjection`, with a `radialVertex` enum. PROJ's
`ivea` is `+proj=dsea +dual`: Snyder's dodecahedral projection with its radial vertex
at the pentagon centre, which is an icosahedron vertex, unfolded onto the icosahedral
net. Because the map is unique, "DSEA dual" and DGGAL's IVEA must be the same
projection mathematically. Phase 2 checks that numerically.

**Design consequence (no special cases):** implement **one** generic slice-and-dice
kernel over the 120 fundamental triangles, parameterized by radial vertex. `DgProjIVEA`
is that kernel with radialVertex = V. The same kernel with radialVertex = C must
reproduce DGGRID's existing Snyder ISEA. That gives a strong **independent oracle**
inside DGGRID itself. RTEA comes almost for free if we ever want it.

### 0.2 External implementations (verified 2026-09-25)

- **PROJ**:
  - OSGeo/PROJ#4758, "Snyder Polyhedral Equal Area projections" (merged 2026-04-17).
  - OSGeo/PROJ#4817, "IVEA: DSEA projection onto icosahedron net" (**merged
    2026-08-18**, `versionadded 9.9`). This PR is no longer open, so we use PROJ
    `master` pinned to a commit.
  - Code: `src/projections/polyhedral/{snyder.h, conway.h, unfold.h, state.h,
    nets/isea/isea.h, nets/dsea/dsea.h, polyhedra/icosahedron.h}` and
    `src/projections/polyhedral.cpp`.
  - Kernel: `snyder_fwd` / `snyder_inv` in `snyder.h`. It is a vector (Recht, 2021)
    closed form in barycentric coordinates, with vertex `a` radial, in `double`.
  - Defaults: `+orient=isea`, orient_lat = atan(φ) ≈ 58.2825° (authalic),
    orient_lon = 11.25° on the sphere (11.20° on the ellipsoid, which matters for
    Phase 7), azi = 0.
  - `snyder.h` says it is **derived from DGGAL and A5**. PROJ and DGGAL are therefore
    *not independent*. If they agree, that shows the port is faithful, not that the
    math is correct. The independent checks are DGGRID's Snyder ISEA (via the ISEA mode
    of the kernel) and the property tests in Phase 2.
- **DGGAL** (ecere/dggal, BSD-3):
  - `src/projections/icoVertexGreatCircle.ec`: the slice-and-dice kernel for ISEA,
    IVEA and RTEA, with both vector and trigonometric paths.
  - `src/projections/ri5x6.ec` (about 2150 lines): the "RI5x6" planar layout, a 5×6
    rhombic arrangement. It has its **own face numbering and planar frame, and is
    possibly skewed/affine**.
  - `src/projections/barycentric5x6.ec`.
  - `src/dggrs/IVEA{3H,4R,7H,7H_Z7,9R}.ec`.
  - Written in eC. **PyPI `dggal` 0.0.6 exists**, which may save us building the eC
    toolchain.
- **Licensing:** DGGRID is AGPL-3.0. DGGAL is BSD-3 and PROJ is MIT, with parts derived
  from A5 under Apache-2.0. All of these are compatible with AGPLv3. Keep the upstream
  copyright notices on any ported code (as PROJ did).

### 0.3 DGGRID internals relevant to IVEA

- Projections plug in through `DgIcosaProj`
  (`src/lib/dglib/include/dglib/DgIcosaProj.h`). This is a `Dg2WayConverter` between
  `DgGeoCoord` (a sphere lat/lon in radians) and `DgProjTriCoord` (`triNum` plus
  (x, y) in that face's frame).
  - Implementations: `DgProjISEA` (Snyder; Song & Sahr) and `DgProjFuller`.
- The projection is selected in **exactly one place**:
  `DgIDGGBase::createConverters()` (`src/lib/dglib/lib/DgIDGGBase.cpp:174`), with
  `if projType()=="ISEA" ... else if "FULLER"`.
- Metafile parameters are in `SubOpDGG::initializeOp()` / `setupOp()`
  (`src/apps/dggrid/SubOpDGG.cpp:167-345`):
  - `dggs_proj` accepts `{ISEA, FULLER}`.
  - `dggs_type` presets are `ISEA*` and `FULLER*`.
  - The preset parser checks for an "isea" prefix and otherwise **silently assumes
    FULLER**, which is a latent bug.
- Grid names are built as `projType + "3H"` etc. (`DgIDGGS.cpp:63-95`), so "IVEA3H"
  will work automatically.
- The icosahedron geometry is shared: `DgSphIcosa` (`DgProjTriRF.{h,cpp}`) builds 12
  vertices and 20 faces (`verts[20][3]`) from `vert0` and `azimuth`. It also provides
  face centres, `dazh` (the azimuth from the centre to face vertex 0), and
  `whichIcosaTri()` (nearest face centre). All of this is independent of the
  projection, so IVEA reuses it unchanged.
- **The ProjTri face frame**:
  - Unit edge. Vertex `icotri[f][0]` is at the apex (0.5, √3/2). Confirmed:
    vert0 → face 2, (0.500000, 0.866025).
  - The centroid is at (0.5, √3/6).
  - My derivation from `sllxy` says `icotri[f][1]` → (0,0) and `icotri[f][2]` → (1,0).
    Here `sllxy` sets x = ρ·sin(az), y = ρ·cos(az), az is measured clockwise from the
    vertex-0 direction, and faces are wound CCW when seen from outside. **This must be
    confirmed empirically in step 1.6.**
- The build uses long double throughout. **On this arm64 Mac `long double` ==
  `double`**, so about 1e-16 is the best agreement possible anywhere.
- **Existing precision limits** (these matter for the comparisons):
  - `DgSphIcosa::ico12verts` uses 26.565051177° instead of atan(1/2) =
    26.56505117707799°, about 1.4e-12 rad.
  - The default vert0 latitude is 58.28252559, where atan(φ) = 58.282525588538994°.
  - `DgProjISEA.cpp` uses 10-digit Snyder constants (R1, DH, originXOff/YOff).
  - Expected result: DGGRID-ISEA vs PROJ-ISEA agree to about 1e-10, not 1e-15. That
    gap is not a bug. Do not "fix" the comparisons to hide it. Record it, and see
    open question Q1.
- **The existing `TRANSFORM_POINTS` GEO→PROJTRI path quantizes to cell centres**
  (tested: at res 3 every point came out as a cell centre). It cannot compare raw
  projection output. The test environment needs a small driver that calls
  `DgIcosaProj` forward/inverse directly (step 2.4).
- Build lists: `src/lib/dglib/CMakeLists.txt` names every source and header
  (lines ~86-88 and ~220-222). `Makefile.noCMake` uses a wildcard, so it needs no edit.

---

## 1. High-level phases

| Phase | Goal | Depends on |
|-------|------|------------|
| **P1** | Metafile parameters and an `IVEA` skeleton, a copy of ISEA wired end to end | none |
| **P2** | Test environment: PROJ and DGGAL built, plus a DGGRID direct-projection driver | none (P1 only for the IVEA driver mode) |
| **P3** | Frame calibration: map every implementation into DGGRID's ProjTri frame; derive face numbering and orientation tables | P2 |
| **P4** | Cross-comparison: PROJ vs DGGAL vs DGGRID, for ISEA first, then IVEA | P3 |
| **P5** | Extract the kernel: a clean long-double slice-and-dice kernel and a 120-triangle locator, as standalone code | P2 (sources only); validated by P4 |
| **P6** | Put P5 inside `DgProjIVEA`; generate grids; regression and property tests | P1, P5, P4 |
| **P7** | Ellipsoidal ISEA/IVEA via authalic latitude (the other agent) | P6 and the authalic work |
| **P8** | Docs, examples, CHANGELOG, manual | P1 (stubs), P6 (final) |

### Parallelization (workstreams for multiple agents)

```
          ┌──────────── WS-A: P1 DGGRID params + skeleton ──────────────┐
          │                                                             │
start ────┼──── WS-B: PROJ build + adapter (P2.2) ──┐                   │
          │                                         ├─ WS-E: P3 → P4 ───┤
          ├──── WS-C: DGGAL build + adapter (P2.3) ─┤  (calibration,    │
          │                                         │   comparisons)    ├─ WS-F: P6 integrate
          ├──── WS-D: P2.4 DGGRID direct driver ────┘                   │        │
          │                                                             │        ▼
          └──── WS-G: P5 kernel extraction (reads sources only) ────────┘   WS-H: P8 docs
                                                                           P7 (other agent)
```

- **Can start immediately, in parallel:** WS-A (P1), WS-B (PROJ), WS-C (DGGAL),
  WS-D (DGGRID driver, ISEA mode first), WS-G (kernel extraction from the paper,
  PROJ `snyder.h` and DGGAL `icoVertexGreatCircle.ec`).
- **WS-E** (calibration and comparison) starts once at least two adapters exist. The
  pilot is DGGRID-ISEA vs PROJ-ISEA.
- **WS-F** (integration) needs WS-A's skeleton, WS-G's kernel, and WS-E showing that
  the kernel matches both references.
- **WS-H** (docs) can stub parameter docs as soon as P1 lands.
- **Shared-file conflicts:** only WS-A and WS-F edit the DGGRID repo. Everything else
  lives in the external test environment. Give WS-A, WS-D and WS-F their own worktrees
  and merge WS-A first.

**Projects:** this plan maps directly onto a Project with one session per workstream
(A–H), sharing this file as the brief. No Project was created here.

---

## 2. Phase 1 (detailed): metafile parameters and the IVEA skeleton

Goal: `dggs_proj IVEA` and `dggs_type IVEA3H|IVEA4H|IVEA7H|IVEA43H|IVEA4T|IVEA4D` work
end to end. They produce output **identical to ISEA** because the skeleton delegates to
the Snyder code. That proves the plumbing, and gives the kernel a place to drop in
later without touching any other code.

### 1.1 Metafile parameters (`src/apps/dggrid/SubOpDGG.cpp`)

1. `initializeOp()`:
   - Add `IVEA3H, IVEA4H, IVEA7H, IVEA43H, IVEA4T, IVEA4D` to the `dggs_type` choice
     list and its comment block (lines ~167-173).
   - Add `"IVEA"` to `dggs_proj` (line ~182) and update its comment to
     `<ISEA | IVEA | FULLER>`.
2. `setupOp()`, the preset parse (lines ~330-339). Replace the two-way
   `if "isea" … else /*must be FULLER*/` with a **table-driven prefix match** over
   `{ {"isea","ISEA"}, {"ivea","IVEA"}, {"fuller","FULLER"} }`, and **report Fatal** if
   nothing matches.
   - Root cause: the current code treats "anything not ISEA" as Fuller. Adding IVEA
     that way would silently produce Fuller grids.
   - With the table, adding the next projection is a one-row change.
3. No new parameters are needed now:
   - `dggs_vert0_lon/lat/azimuth` apply to IVEA unchanged. PROJ and DGGAL both use the
     ISEA orientation for IVEA.
   - The `proj_datum*` parameters are untouched. Ellipsoid parameters belong to P7 and
     the authalic agent. Coordinate names with them. Do not add them here.
4. Check the other preset branches: SUPERFUND stays FULLER, PLANETRISK and IGEO7 stay
   ISEA. Do not add an IVEA-based IGEO7 variant yet (open question Q5).

### 1.2 Projection dispatch (`src/lib/dglib/lib/DgIDGGBase.cpp:170-180`)

- Add `#include <dglib/DgProjIVEA.h>` and an `else if (projType() == "IVEA")` branch.
- *Recommended small refactor:* move the string→projection dispatch into one factory,
  e.g. `DgIcosaProj* DgIcosaProj::makeIcosaProj(const std::string& projType, geoRF,
  projTriRF)` in a new `DgIcosaProj.cpp`, and have `createConverters()` call it. This
  keeps "the list of projections" in one library location. Optional; keep it if it
  stays tiny.

### 1.3 Skeleton classes

Create `src/lib/dglib/include/dglib/DgProjIVEA.h` and `src/lib/dglib/lib/DgProjIVEA.cpp`
by copying `DgProjISEA.{h,cpp}`:

- Rename `DgProjISEAFwd/Inv/DgProjISEA` → `DgProjIVEAFwd/Inv/DgProjIVEA`, along with
  the header guard, file banners and report strings.
- **Do not copy the free C functions** (`sllxy`, `snyderFwd`, `snyderInv` and the
  `static const` Snyder constants). They have external linkage in `DgProjISEA.h`, so
  copies would cause duplicate-symbol link errors, and they are not IVEA math anyway.
- Instead, `DgProjIVEA.cpp` includes `DgProjISEA.h`, and the skeleton
  `convertTypedAddress` bodies call `snyderFwd` / `snyderInv`. Mark each with
  `// SKELETON: delegates to ISEA; replaced in Phase 6`.
- Keep the constructor `dynamic_cast` checks exactly as in ISEA.
- Add both files to `src/lib/dglib/CMakeLists.txt`, alphabetically next to
  `DgProjISEA`. `Makefile.noCMake` uses a wildcard, so it needs no change.

### 1.4 Other projType-string consumers

- `grep` showed that the only `projType() ==` comparison is in `DgIDGGBase.cpp`.
  Re-check after the change: `grep -rn '"ISEA"\|"FULLER"' src | grep -v proj4lib`.
- Default `projType = "ISEA"` arguments in the IDGGS headers stay as they are.
- `DgIDGGutil.cpp` and `DgIDGG.cpp` include the projection headers but don't dispatch.
  No change is needed.

### 1.5 Build and smoke test

```
cmake --build build -j
```

Run each metafile pair below in the scratch/test environment (not in `examples/` yet).
Each pair is identical except for `dggs_type`:

- `GENERATE_GRID` with `ISEA3H` vs `IVEA3H` at res 3, KML/GeoJSON output: **byte-identical**
  apart from the DGGS name string.
- The same for 4H, 7H, 43H, 4T and 4D.
- A `CUSTOM` metafile with `dggs_proj IVEA`.
- `TRANSFORM_POINTS` GEO→SEQNUM and GEO→Q2DI on `examples/transform/inputfiles/20k.txt`:
  identical output.
- Negative test: `dggs_type IVEX3H` must fail at parameter validation, and the preset
  parser's Fatal branch must be unreachable from valid choices.
- Run `examples/doexamples.sh` and diff against `examples/sampleOutput`: no change for
  any existing example.

### 1.6 Pin down the ProjTri face frame (needed by P3 and P5)

This can be a throwaway driver run under WS-D. For each face f and each j ∈ {0,1,2},
forward-project a point at 1 − 1e-9 of the way from the face centre to `icotri[f][j]`,
using `DgProjISEA`, and record (x, y).

- Expected: j=0 → apex (0.5, √3/2), j=1 → (0,0), j=2 → (1,0), for all 20 faces.
- Also record which sub-triangle convention falls out: the (V, M, C) index order
  around the centroid.
- Write the result into this file (§0.3), replacing "derivation".

### 1.7 Commit

Commit P1 as one small commit ("add IVEA projection parameter and skeleton"). Add a
CHANGELOG line, marked as work in progress.

---

## 3. Phase 2 (detailed): test environment

Location: **outside the repo**, at `/Users/sahrk/CODE/IVEA_TESTENV/`. It is not tracked
in DGGRID and can become its own git repo.

```
IVEA_TESTENV/
  README.md              # pinned commits, build commands, how to run
  ext/PROJ/              # git clone OSGeo/PROJ (master, pinned hash, ≥ PR #4817 merge)
  ext/dggal/             # git clone ecere/dggal (pinned hash)
  ext/ecere-sdk/         # only if the PyPI package is insufficient
  build/proj/            # PROJ build dir
  harness/
    points/gen_points.py     # deterministic point sets (see 2.5)
    adapters/proj_adapter.c  # links libproj; fwd/inv for isea, ivea, dsea
    adapters/dggal_adapter.py (or .ec/.c)
    adapters/dggrid_adapter.cpp  # links DGGRID libdglib; calls DgIcosaProj directly
    ref/                     # extracted standalone kernels (P5)
    compare/                 # calibration + comparison + property tests (P3/P4)
  results/                   # CSVs + summary report per run
```

### 2.1 Common I/O contract (all adapters)

- Input is a text file, one record per line.
  - Forward: `lon_deg lat_deg`.
  - Inverse: implementation-native planar coordinates.
- Output uses the same record order, `%.17g` for every float, and `nan nan` on
  failure (never skip a line).
- Sphere with R = 1 everywhere. PROJ: `+R=1`. DGGRID: pass `DgGeoCoord` directly, since
  the projection is on the unit sphere.
- **Explicit, identical orientation in all three:**
  - vertex lat = atan(φ) in full precision (58.282525588538994°)
  - lon = 11.25°
  - azimuth = 0
  - Do not rely on defaults. DGGRID's default 58.28252559 is truncated.
  - Also run a second orientation, e.g. vertex on the pole or a random one, to catch
    orientation handling bugs.

### 2.2 PROJ (WS-B)

- `git clone https://github.com/OSGeo/PROJ ext/PROJ`, then check out a pinned commit
  that includes the merges of #4758 and #4817.
- Build a minimal PROJ:

  ```
  cmake -S ext/PROJ -B build/proj -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_APPS=ON -DBUILD_TESTING=ON
  ```

  Homebrew's `/opt/homebrew/bin/proj` is older and **must not be used**; put
  `build/proj/bin` first in PATH.
- Run PROJ's own tests: `ctest -R 'polyhedral|gie'`.
- Adapter: `proj_create(ctx, "+proj=ivea +R=1 +orient_lat=… +orient_lon=11.25 +azi=0")`,
  then `proj_trans` for forward and inverse. Also build the same for `+proj=isea`, and
  for `+proj=dsea` (the non-dual form, for the dodecahedral check in P4).
  - Find out which code backs `+proj=isea` at that commit: the new polyhedral code or
    the legacy `isea.cpp`. Test both if both are reachable.
  - Also compare `ivea` with `dsea +dual`; they should be the same string.
- Also produce **raw kernel access**: a tiny C++ test that includes
  `polyhedral/snyder.h` directly and calls `snyder_fwd/inv` on one sub-triangle. The
  header is self-contained apart from `sphere.h`/`vec3.h`. This gives us net-free
  numbers.

### 2.3 DGGAL (WS-C)

- `git clone https://github.com/ecere/dggal ext/dggal` at a pinned hash.
- **First try `pip install dggal==0.0.6`** in a venv. Check whether it exposes
  `IVEAProjection`, `ISEAProjection` and `RTEAProjection` `forward`/`inverse`
  (GeoPoint ↔ Pointd), or at least the DGGRS zone-geometry APIs.
  `bindings_examples/py/authalic.py` suggests projection-level objects are reachable.
- If they are not exposed, build the ecere SDK (the eC compiler and `ecrt`) and DGGAL
  from source, then write a small eC or C-binding adapter. Budget time for this, since
  it is the riskiest setup step.
- Record DGGAL's planar output frame. RI5x6 is a 5×6 layout and may be an **affine
  (sheared)** image of the triangle net, not a similarity. `barycentric5x6.ec` may give
  per-face barycentric coordinates directly. If so, prefer that: it removes most of the
  P3 calibration for DGGAL.
- Record DGGAL's default orientation constants, and whether it uses 11.25° or 11.20°
  on the sphere.

### 2.4 DGGRID direct-projection driver (WS-D)

- `harness/adapters/dggrid_adapter.cpp` links `build/src/lib/dglib/libdglib.a` (plus
  the shapelib/proj4lib deps if needed). It builds:
  - a `DgRFNetwork`
  - `DgGeoSphRF` (R = 1)
  - `DgSphIcosa(vert0, az)`
  - `DgProjTriRF`
  - then `DgProjISEA`, `DgProjIVEA` or `DgProjFuller`, chosen by a command-line flag
- Forward: GEO → (triNum, x, y). Inverse: (triNum, x, y) → GEO.
- This bypasses the cell quantization that `TRANSFORM_POINTS` does.
- Model it on `src/apps/appex/appex.cpp`.
- Keep it outside the repo for now. It may later become `src/apps/projtest` if we want
  it in CI (open question Q6).

### 2.5 Point sets (`gen_points.py`, seeded and deterministic)

1. N = 10⁶ uniform random points on the sphere, using a fixed seed.
2. Special loci, computed from the *same* icosahedron as DGGRID. Recompute the
   vertices in closed form, and also with DGGRID's truncated constants to measure the
   effect.
   - the 12 vertices
   - the 20 face centres
   - the 30 edge midpoints
3. Boundaries of the 120 fundamental triangles, sampled densely along each of the
   three edge types:
   - V–M (on a face edge)
   - V–C (a median, from vertex to centre)
   - M–C (a median, from mid-edge to centre)

   Include points ±1e-12 rad off each boundary, on both sides.
4. Near-singular points: distances of 1e-15 … 1e-6 rad from each vertex, centre and
   midpoint.
5. The poles, the antimeridian ±180°, and lat ±(90 − 1e-12).
6. Inverse sets, generated in the DGGRID ProjTri frame (face, x, y) and mapped to each
   implementation's plane through the P3 calibration:
   - a uniform barycentric grid per face
   - fundamental-triangle boundaries
   - points 1e-12 outside the face. DGGRID's vertex2DD → projTri path can produce these
     for cells that straddle an edge, so inverse must degrade gracefully, not blow up.

---

## 4. Phase 3: frame calibration (WS-E), mapping everything into one frame

Differences in face numbering, triangle orientation and net layout are **not
projection differences**. Handle them once, systematically, instead of hand-editing
face tables.

- **Hub frame:** DGGRID ProjTri. A point is (DGGRID face f, x, y), where face f is
  defined by its three spherical vertices `icotri[f][0..2]`.
- For each implementation I and each DGGRID face f, find an **affine map A_{I,f}** from
  the ProjTri frame of f to I's native plane. Every projection in this family fixes the
  vertices of the 120 fundamental triangles, so A_{I,f} is **independent of the
  projection** and can be found from these fixed points:
  - The face centroid C_f. It is an exact interior point with no ambiguity.
  - The three edge midpoints and three vertices. These lie on shared boundaries, so
    approach them from inside face f (for example at 1 − 1e-9 toward C_f) and pick the
    image nearest I(C_f).
  - Snap the result using the net's known structure: for a similarity net (PROJ), the
    rotation is a multiple of 60° plus a reflection flag, and the scale is known exactly
    from equal area (face area = 4πR²/20). For DGGAL's RI5x6, use its documented linear
    transform.
  - Verify: the residual at all 7 fixed points should be < 1e-14 after snapping.
- **By-product: the face correspondence tables**, which answer "is face numbering or
  orientation different?". For each implementation, record DGGRID face f → I's face id
  (if it has one), which vertex of I's planar triangle receives `icotri[f][0]`, and the
  handedness. Write them to `results/face_maps_{proj,dggal}.csv`. Summarize them here
  once known. Check that the mapping is a proper icosahedral symmetry, i.e. consistent
  across shared edges.
- **Calibrate on ISEA first** (WS-E pilot). All three implementations have ISEA, and
  DGGRID's ISEA is the trusted, long-standing one. Then check that IVEA in PROJ
  (`+dual`, "aligned with isea") and in DGGAL (the same RI5x6 base) uses the same
  A_{I,f}. If it doesn't, that is itself a finding: the net differs between ISEA and
  IVEA in that implementation.

## 5. Phase 4: cross-comparison (WS-E)

Everything is compared in the hub frame. Report the max, p99.9 and RMS error, plus the
worst 20 points with their loci.

| # | Comparison | Expected | What it proves |
|---|-----------|----------|----------------|
| C1 | DGGRID-ISEA vs PROJ-ISEA (fwd & inv) | ≈1e-10 (DGGRID's truncated constants) | frame calibration; baseline |
| C2 | DGGRID-ISEA vs DGGAL-ISEA | ≈1e-10 | same, for DGGAL |
| C3 | PROJ-ISEA vs DGGAL-ISEA | ≈1e-15 | ports agree |
| C4 | PROJ-IVEA vs DGGAL-IVEA (fwd & inv) | ≈1e-15 | **do they agree?** |
| C5 | PROJ `dsea` (non-dual) composed with the per-sub-triangle affine map vs PROJ-IVEA | ≈1e-15 | the uniqueness argument in §0.1; the dual unfolding |
| C6 | P5 extracted kernel (ISEA mode) vs DGGRID-ISEA | ≈1e-10; → 1e-15 if constants are upgraded | **independent oracle** for the kernel |
| C7 | P5 kernel (IVEA mode) vs PROJ-IVEA & DGGAL-IVEA | ≈1e-15 | the extracted code is correct |
| C8 | Round trip inv(fwd(p)) for every implementation | ≈1e-15 rad | internal consistency |

**Property tests.** These are independent of every implementation.

- **Equal area:**
  - For small spherical triangles, compare planar area × (4πR²/20) / (√3/4) with the
    spherical excess. The ratio should be 1 ± 1e-12.
  - Do this per fundamental triangle, and globally over the whole sphere.
- **Radial straightness (IVEA):** points on a great circle through vertex V, within one
  sub-triangle, map to collinear points through V′ (the cross-product residual is about
  0).
- **Radial straightness (ISEA):** the same with the face centre C.
- **Continuity:** across all three boundary types, points ±1e-12 either side map to
  within about 1e-12 of each other.
  - Across face edges, compare in the hub frame after mapping the neighbour face's
    coordinates with DGGRID's own vertex2DD / neighbour transforms.
- **Fixed points:** vertices, midpoints and centres map to their exact planar images.
- **Symmetry:** applying an icosahedral rotation to the input commutes with the
  projection, up to the face relabelling.

**Deliverable:** `results/REPORT.md`, which answers:

- Do PROJ and DGGAL agree?
- To what tolerance?
- Are face numbering, orientation and handedness different, and how?
- Are there degenerate-point failures, e.g. at vertices or at the IVEA radial vertex
  (DGGAL has a commented-out `poleFixIVEA`, so check near-vertex points in particular)?

## 6. Phase 5: kernel extraction (WS-G, can start now)

Write standalone, dependency-free C++ in `harness/ref/`, in long double style to match
DGGRID. Later it moves into dglib.

1. **`sliceDiceFwd(P, A, B, C, A', B', C')` / `sliceDiceInv(...)`** for a single
   spherical triangle, with A radial and a planar target triangle. Base it on the
   vector form: PROJ `snyder_fwd/inv`, DGGAL `inverseVector`, and Recht (2021). Keep a
   spherical-trigonometry fallback for degenerate inverses, as DGGAL does.
   - Handle degenerate inputs explicitly, as one principled rule rather than scattered
     epsilons: P = A, P on BC, and P on AB/AC. The rule: clamp the barycentric
     coordinates to the closed triangle only within a documented tolerance, and fail
     loudly outside it.
   - Keep sign and hemisphere conventions (the quadruple-product antipode) explicit,
     with a comment for each.
2. **Fundamental-triangle geometry**, precomputed once per `DgSphIcosa`:
   - 3D unit vectors for all 12 vertices, 20 centres and 30 midpoints
   - for each face, its 6 sub-triangles (V, M, C) as spherical vectors
   - their planar images in the ProjTri frame: V at the corner, M at the edge midpoint,
     C at (0.5, √3/6)

   Forward locator: face from `whichIcosaTri` (reused), then sub-triangle by sign
   tests against the 3 median great-circle planes. Inverse locator: sub-triangle from
   the planar point's position relative to the 3 planar medians. Use the **same
   index convention** in both directions (fixed in step 1.6).
3. **The radial vertex is a parameter**: {FaceCentre = ISEA, Vertex = IVEA,
   EdgeMidpoint = RTEA}. The permutation of (V, M, C) into kernel roles (A radial, B,
   C) lives in one table. There is no per-projection code path.
4. Cross-read DGGAL's trigonometric derivation against the paper. Document the
   equations in comments, and cite van Leeuwen & Strebe 2006, Snyder 1992 and Recht
   2021.
5. Validate with C6 and C7 in P4 before integration.

## 7. Phase 6: integration into DGGRID (WS-F)

1. Add `DgIcosaSliceDice` (the kernel and the 120-triangle tables) to dglib. The
   precomputed geometry lives with `DgSphIcosa`, so ISEA, IVEA and later RTEA share it.
2. `DgProjIVEAFwd/Inv::convertTypedAddress`: replace the skeleton bodies with the
   kernel in Vertex mode. The public class shape from P1 stays the same.
3. Leave `DgProjISEA` (legacy Snyder) **unchanged**, so ISEA output stays byte-stable.
   Keep a kernel-in-ISEA-mode path as a test-only check (C6). Whether to retire the
   legacy code is open question Q2.
4. Precision: compute the IVEA constants in closed form: atan(1/φ), acos(√((φ+1)/3)),
   atan(2/φ²), and the 6° sub-triangle excess. The shared icosahedron vertex precision
   is open question Q1.
5. Grid-level regression. These invariants follow from the design:
   - **Every non-GEO output is identical between ISEA and IVEA** for the same grid
     parameters: SEQNUM, Q2DI, Q2DD, PROJTRI, PLANE, neighbours, children, and
     hierarchical indices. Only the projection between PROJTRI and GEO changes. Diff
     these automatically.
   - GEO outputs differ from ISEA, and match PROJ/DGGAL IVEA at the point level (C7
     through the full DGGRID pipeline, using the driver).
   - **Equal area at the cell level:** at res n, every hexagon has the same spherical
     area, and each of the 12 pentagons has the expected fraction. Check with a script
     over the GENERATE_GRID polygon output, for aperture 3, 4 and 7 and for triangle and
     diamond grids.
   - Cell-set match with DGGAL's IVEA3H/IVEA7H: nearest-neighbour matching of cell
     centroids and vertex sets. No zone-ID mapping is needed.
6. Run all `examples/` scripts to confirm existing outputs are unchanged. Add IVEA
   examples.

## 8. Phase 7: ellipsoidal versions (after the authalic-latitude work lands)

- Keep projections **purely spherical**. Handle the ellipsoid as a separate
  geodetic↔authalic converter in the RF network, placed in front of the
  `DgGeoSphRF` → ProjTri step. It then applies to ISEA, IVEA and Fuller in the same way,
  with no per-projection ellipsoid code.
- Orientation semantics must be decided with the other agent: is `dggs_vert0_lat`
  authalic or geodetic? PROJ uses about 58.40° geodetic, which is atan(φ) authalic, and
  11.20° lon on the ellipsoid. Compare against PROJ `+proj=ivea +ellps=WGS84` and
  DGGAL's authalic path, reusing the P2–P4 harness unchanged.

## 9. Phase 8: documentation (WS-H)

- Manual (`documentation/source/dggrid_man_V841.md`):
  - the `dggs_proj` description (§3 "Specifying the projection")
  - the parameter table (line ~432)
  - the `dggs_type` table and Appendix B presets
  - the IVEA description and citations
- `CHANGELOG.md`, and README if it lists projections.
- Examples: `examples/ivea3hGen` (or similar), added to `examples.lst` with sample
  output.
- Doxygen comments on the new classes.

---

## 10. Open questions / decisions for Kevin (none block Phase 1)

- **Q1.** Upgrade `DgSphIcosa`'s icosahedron constants (26.565051177°, and the default
  vert0 lat of 58.28252559) and the Snyder ISEA constants to full precision? This
  improves agreement from about 1e-10 to about 1e-15, but changes existing ISEA
  outputs slightly.
- **Q2.** Once C6 shows the kernel matches, replace the legacy Snyder ISEA with the
  kernel in ISEA mode? That gives one code path, but ISEA output changes at the
  precision level.
- **Q3.** Expose RTEA too? It is almost free, since it only needs a different radial
  vertex, but it needs its own validation.
- **Q4.** Should the IVEA default orientation be the same as ISEA's? This plan assumes
  yes, matching PROJ and DGGAL.
- **Q5.** Add an IVEA-based analogue of the IGEO7 preset (DGGAL has IVEA7H_Z7)?
- **Q6.** Should the direct-projection driver (§2.4) move into the repo as a test app
  and CI check?
