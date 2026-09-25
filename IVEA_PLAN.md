# Plan: Adding the IVEA (Slice-and-Dice) Projection to DGGRID

Status: draft plan, 2026-09-25 (rev 2). Branch: `ivea`.
Scope for now: the **spherical** projection only. Ellipsoidal ISEA/IVEA will come
later, built on the authalic-latitude work another agent is doing (see Phase 7).

**Rev 2 changes:**
- Added Phase 0: example-regression tooling and a baseline for this machine.
- Added Phase 1A: every geometric constant goes to full precision, as an early step.
  This resolves former Q1.
- Added the **standing regression rule R** (§1.1). Every step that touches DGGRID code
  must run the examples suite and validate its output.
- The skeleton phase is now Phase 1B.

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
projection mathematically. Phase 4 checks that numerically.

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
    of the kernel) and the property tests in Phase 4.
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
  - `icotri[f][1]` → (0,0) and `icotri[f][2]` → (1,0), per my derivation from `sllxy`.
    Partly confirmed: the north pole projects to face 0 at (0.25, 0.433013), the
    midpoint of the apex–(0,0) edge. Full confirmation is step 1B.6.
- The build uses long double throughout, and precision depends on the platform:
  - **On this arm64 Mac, `long double` == `double`** (53-bit mantissa).
  - On **ai00** (Ubuntu, AMD Ryzen, x86-64), `long double` is 80-bit extended (64-bit
    mantissa). The dggrid log reports this as "big double: 128 bits" (storage size).
    That is why ai00 produces the canonical `sampleOutput`.
- **Truncated constants, fixed in Phase 1A:**
  - `DgSphIcosa::ico12verts` uses 26.565051177° instead of atan(1/2).
  - The default vert0 latitude is 58.28252559.
  - `DgProjISEA.cpp` uses 10-digit Snyder constants (R1, DH, originXOff/YOff).
  - Where the vert0 value comes from: it was **set by hand**. It puts the north pole
    on the midpoint of an icosahedron edge (checked: N pole → face 0 (0.25, 0.433013),
    S pole → face 16 (0.25, 0.433013), both edge midpoints). Equivalently, the
    equatorial plane is an icosahedral mirror plane, so the grid is symmetric about the
    equator.
  - The exact value for that property: place the vertices at the cyclic permutations
    of (0, ±1, ±φ), with φ = (1+√5)/2 the golden ratio. The edge (0,1,φ)–(0,−1,φ) has
    its midpoint (0,0,φ) on the z-axis, so vertex (0,1,φ) has latitude
    atan(φ/1) = **atan(φ) = 58.28252558853899…°**. The hand-set value is 1.1e-9°
    from this.
- **The existing `TRANSFORM_POINTS` GEO→PROJTRI path quantizes to cell centres**
  (tested: at res 3 every point came out as a cell centre). It cannot compare raw
  projection output. The test environment needs a small driver that calls
  `DgIcosaProj` forward/inverse directly (step 2.4).
- Build lists: `src/lib/dglib/CMakeLists.txt` names every source and header
  (lines ~86-88 and ~220-222). `Makefile.noCMake` uses a wildcard, so it needs no edit.

### 0.4 The examples regression suite (as found)

- `examples/examples.lst` lists 47 examples. `examplesNoGDAL.lst` is the non-GDAL
  subset. The local build has `WITH_GDAL=ON` (Homebrew GDAL 3.13.2), so use the full
  list.
- `doexamples.sh` runs each `<ex>/<ex>.meta` with `../../build/src/apps/dggrid/dggrid`.
  It writes the log to `<ex>/outputfiles/<ex>.txt` and runs
  `diff -rwb outputfiles ../sampleOutput/<ex>`. `cleanexamples.sh` empties the
  `outputfiles/` directories. `copyexamples.sh` copies `outputfiles/*` into
  `sampleOutput/<ex>`.
- Output types in `sampleOutput` (32 MB):
  - text: 57 txt, 51 kml, 15 geojson, 4 nbr, 4 chd, 2 gen, ndxPrt/ndxChd, `.meta.000N`
  - binary: 7 shapefile sets (shp, shx, dbf, prj)

  `diff` can't compare the binary shapefiles meaningfully; they need `ogrinfo` dumps.
- Problems with the scripts as they stand:
  - **`dymaxionIcosa` is in `examples.lst` but has no `sampleOutput`**. It was added
    in commits 239aa0b..688940b. Canonical output for it has to be generated on ai00.
  - The first lines of every log depend on the environment:
    `** executing DGGRID version 9.0b with GDAL version 3130100 **` and
    `type sizes: big int: 64 bits / big double: 128 bits`. On this Mac the second
    line reads 64 bits, so **every log file will differ from `sampleOutput` on those
    lines**. The comparer must normalize exactly these known lines, and nothing else.
  - `doexamples.sh` doesn't check dggrid's exit status, and doesn't guard `cd $f`. A
    missing example directory would run everything after it from the wrong place.
  - Only `diff -rwb` is used. It can't tell harmless last-digit round-off from real
    changes.
- The local build has `CMAKE_BUILD_TYPE` empty, i.e. no optimization, with Apple clang
  (`/usr/bin/c++`). Clang on arm64 contracts a*b+c into fused multiply-adds by
  default, which is another source of last-digit round-off against ai00. Record the
  build type and compiler used on both machines.

---

## 1. High-level phases

| Phase | Goal | Depends on | Changes DGGRID code? |
|-------|------|------------|----------------------|
| **P0** | Example-regression tooling (compare with tolerance, validate output) and a validated baseline reference for this Mac | none | scripts only |
| **P1A** | Every geometric constant to its full closed form | P0 | **yes, and output changes** |
| **P1B** | Metafile parameters and an `IVEA` skeleton, a copy of ISEA wired end to end | P1A | yes (output identical) |
| **P2** | Test environment: PROJ and DGGAL built, plus a DGGRID direct-projection driver | none (P1B for the IVEA driver mode) | no |
| **P3** | Frame calibration: map every implementation into DGGRID's ProjTri frame; derive face numbering and orientation tables | P2 | no |
| **P4** | Cross-comparison: PROJ vs DGGAL vs DGGRID, for ISEA first, then IVEA | P3 | no |
| **P5** | Extract the kernel: a clean long-double slice-and-dice kernel and a 120-triangle locator, as standalone code | P2 (sources only); validated by P4 | no |
| **P6** | Put P5 inside `DgProjIVEA`; generate grids; regression and property tests | P1B, P5, P4 | **yes** (new examples) |
| **P7** | Ellipsoidal ISEA/IVEA via authalic latitude (the other agent) | P6 and the authalic work | yes |
| **P8** | Docs, examples, CHANGELOG, manual | P1B (stubs), P6 (final) | examples only |

### 1.1 Standing regression rule R (every step that changes DGGRID code)

A step is "changing DGGRID code" if it touches anything under `src/`, the CMake files,
the compiler or build flags, or example metafiles and inputs. Such a step is **not
done** until all of the following pass on this Mac:

1. **Build:** `cmake --build build -j`. Reconfigure from scratch if CMake files
   changed. From step 0.6 on, always use the Release build (§2 step 0.6).
2. **Run the suite:**

   ```
   cd examples && ./cleanexamples.sh && ./doexamples.sh > $RUNDIR/doexamples.log 2>&1
   ```

   `RUNDIR=/Users/sahrk/CODE/IVEA_TESTENV/example_runs/<yyyymmdd-hhmm>_<shortsha>/`.
   The built-in `diff -rwb` output stays for eyeballing. The next two steps are the
   actual gate.
3. **Validate:** `python3 checkexamples.py` must be PASS for every example. It checks
   the output is correct on its own terms, with no reference (§2 step 0.3).
4. **Compare:**
   - against the **local Mac reference**: `python3 cmpexamples.py --ref $MACREF`.
     Every file must be **IDENTICAL**, unless the plan declares the step
     output-changing (see below).
   - against the **canonical ai00 reference**: `python3 cmpexamples.py --ref
     sampleOutput`. **This is informational and never blocks.** Report the summary.
     Differences that come from steps still in the ai00 regeneration queue are
     expected until Kevin regenerates. Examples with no canonical output yet
     (`dymaxionIcosa`, new IVEA examples) are NOREF.
5. **Record:** put the one-line summary from `cmpexamples.py` in the commit message:
   file counts per class and the maximum numeric difference.

**Output-changing steps.** The plan says in advance which steps change output (P0.6, P1A,
P6, P8's new examples) and what change to expect. For those, step 4 against `$MACREF`
runs in `--expect-change` mode with the stated tolerance (see P1A.5). Every difference
outside the tolerance must be explained individually. Once accepted:

- regenerate `$MACREF` (§2 step 0.5), keeping the previous one as
  `$MACREF.<oldsha>`;
- add the step to the **ai00 regeneration queue** (§1.2). Then carry on. Work never
  waits for ai00.

`$MACREF` is `/Users/sahrk/CODE/IVEA_TESTENV/sampleOutput.mac-arm64/`. It lives
**outside the repo**, so every worktree and agent shares it and git never sees it.

### 1.2 ai00 canonical regeneration (Kevin, on ai00)

The canonical `examples/sampleOutput` is only ever generated on ai00, by Kevin, when
he chooses. **No step blocks on it.** All testing uses the local Mac output and
`$MACREF` until Kevin updates ai00. Keep a running list of queued output-changing
steps in `IVEA_TESTENV/AI00_QUEUE.md`, with the commit, the expected change and the
tolerance for each. Kevin can then batch them. Queue entries so far: the Release
default (P0.6), P1A, and the new IVEA examples (P6/P8). Checklist:

1. On ai00, `git pull` the branch.
2. `cmake --build build -j`. ai00's cache currently has an empty build type, the same
   as the Mac. Once P0.6 has landed, reconfigure from scratch (`cmake -S . -B build`)
   so it picks up the new Release default.
3. `cd examples && ./cleanexamples.sh && ./doexamples.sh`, then run
   `checkexamples.py`. It must be all PASS.
4. `cmpexamples.py --ref sampleOutput --expect-change <tolerance>`. Review the
   differences against the expectation written in the step.
5. `./copyexamples.sh`, then commit `examples/sampleOutput`. This includes the first
   `dymaxionIcosa` output.
6. Back on the Mac: pull, then `cmpexamples.py --ref sampleOutput`. It must be
   IDENTICAL or ROUNDOFF only. That confirms `$MACREF` is still a faithful stand-in for
   ai00. Any STRUCTURAL difference becomes an investigation item, reported to Kevin.
   Mac-side work continues against `$MACREF` in the meantime.
7. Clear the handled entries from `AI00_QUEUE.md`.

### 1.3 Parallelization (workstreams for multiple agents)

```
          ┌── WS-A: P0 tooling+baseline → P1A constants → P1B params+skeleton ──┐
          │         (serial: all three touch examples/ and SubOpDGG.cpp)          │
start ────┼──── WS-B: PROJ build + adapter (P2.2) ──┐                             │
          │                                         ├─ WS-E: P3 → P4 ─────────────┤
          ├──── WS-C: DGGAL build + adapter (P2.3) ─┤  (calibration,              │
          │                                         │   comparisons)              ├─ WS-F: P6 integrate
          ├──── WS-D: P2.4 DGGRID direct driver ────┘                             │        │
          │                                                                       │        ▼
          └──── WS-G: P5 kernel extraction (reads sources only) ──────────────────┘   WS-H: P8 docs
                                                                                     P7 (other agent)
```

- **Can start immediately, in parallel:**
  - WS-A (P0 → P1A → P1B, serial)
  - WS-B (PROJ)
  - WS-C (DGGAL)
  - WS-D (DGGRID driver). Build it against current HEAD first, then rebuild after P1A
    lands. The driver itself doesn't change.
  - WS-G (kernel extraction from the paper, PROJ `snyder.h` and DGGAL
    `icoVertexGreatCircle.ec`)
- **WS-E** (calibration and comparison) starts once at least two adapters exist. The
  pilot is DGGRID-ISEA vs PROJ-ISEA. Its headline numbers (C1, C2, C6) should be
  measured **after P1A**. Measuring once before P1A as well documents what P1A gained.
- **WS-F** (integration) needs WS-A's skeleton, WS-G's kernel, and WS-E showing that
  the kernel matches both references.
- **WS-H** (docs) can stub parameter docs as soon as P1B lands.
- **Shared-file conflicts:** only WS-A and WS-F edit the DGGRID repo. Everything else
  lives in the external test environment. Give WS-A, WS-D and WS-F their own worktrees
  and merge WS-A first. Each worktree has its own `build/` and `examples/outputfiles`.
  All of them compare against the shared `$MACREF`.
- **Example runs** are CPU-bound. Don't run the suite in two worktrees at once if you
  want the timings to mean anything.

**For an orchestrator (Project or lead agent):**
- The workstreams A–H and their dependencies above are the intended task breakdown.
- Hard ordering constraints:
  - WS-A runs serially: P0 → P1A → P1B.
  - Rule R gates every repo-changing task.
  - Only WS-A and WS-F write to the DGGRID repo, each in its own worktree.
- Keep this file under a single editor. Other workers report results in
  `IVEA_TESTENV/results/` and do not edit this plan directly.

---

## 2. Phase 0 (detailed): example-regression tooling and the Mac baseline

Goal: before touching any DGGRID code, have a trustworthy, **validated** reference
for this Mac, and tools that separate round-off from real change.

### 0.1 Build the baseline

- Start from a clean configure of the current `ivea` HEAD:

  ```
  cmake -S . -B build && cmake --build build -j
  ```

- Record the commit hash, compiler and version, `CMAKE_BUILD_TYPE`, CXX flags, GDAL
  version, the dggrid "type sizes" line, and the macOS version. Write these into
  `$MACREF/PROVENANCE.txt` in step 0.5.
- **Current build type.** The top-level `CMakeLists.txt` never sets
  `CMAKE_BUILD_TYPE`. `INSTALL.md` says to pass `-DCMAKE_BUILD_TYPE=Release`, but a
  plain `cmake ..` gives an **unoptimized build**.
  - The current Mac `build/` is one: its `flags.make` shows only
    `-arch arm64 -Wall -pedantic`, with no `-O`.
  - ai00 has the same empty build type (confirmed by Kevin), so today's canonical
    `sampleOutput` is also from an unoptimized build.
  - The baseline (step 0.5) therefore uses the empty build type on purpose, to match
    how canonical was made. Step 0.6 then switches the default to Release.

### 0.2 Harden the example scripts

Keep the diffs small, and keep the default behaviour identical to today.

- `doexamples.sh` and `doexamplesNoGDAL.sh`:
  - Take an optional reference directory argument, defaulting to `sampleOutput`.
  - Guard with `cd "$f" || { echo "MISSING EXAMPLE $f"; continue; }`.
  - After the dggrid run, print `dggrid exit status N` to the script's stdout, not to
    the log file, so the logs stay comparable.
  - Print a note instead of a diff wall when the reference directory is missing, as
    for `dymaxionIcosa`.
- `copyexamples.sh`: take an optional destination directory, defaulting to
  `sampleOutput`, and create `<dest>/<ex>` if it's missing.
- Mirror these changes in the `NoCMake` variants.
- Update `examples/README.txt` to describe the new arguments and the two Python tools.

### 0.3 `examples/checkexamples.py`: validity without a reference

For each example in the list, check:

- **Run health:**
  - The log exists and ends with dggrid's normal completion message.
  - No `ERROR`, `Fatal`, `Unable to continue` or `is located on another polygon` lines.
  - The exit status (from the doexamples stdout) is 0.
- **Expected files:**
  - Every output file named in the metafile exists and is non-empty. This covers the
    `*_output_file_name` parameters, the neighbour/children files and the
    per-placement `.000N` files.
  - The metafile is parsed with the same `key value` rules dggrid uses.
- **Syntax:**
  - KML: `xmllint --noout`.
  - GeoJSON: `json.load`, and every geometry is a valid Polygon or Point.
  - Shapefiles: `ogrinfo -al -so` opens them and reports the feature count.
  - Text tables: a consistent column count per file.
- **Numeric sanity** in every text or dumped output:
  - no `nan` or `inf`
  - lon in [−180, 180], lat in [−90, 90]
  - polygon rings closed, with at least 4 points
- **Grid invariants** where they apply:
  - Whole-earth `GENERATE_GRID` cell counts: hexagon aperture a at res r gives
    10·a^r + 2 cells; 4T gives 20·4^r; 4D gives 10·4^r. Mixed and sequence apertures
    use the product of their apertures, and should match the counts dggrid prints.
  - Exactly 12 pentagons in whole-earth hexagon grids.
  - Neighbour and children files reference only cells that exist.
- **Equal-area check** (ISEA now, and reused for IVEA in P6): for whole-earth hexagon
  outputs, compute the spherical polygon area of every cell.
  - Hexagons must all be equal within the tolerance that printed precision allows,
    derived from `precision` in the metafile.
  - The areas must sum to 4πR² within that tolerance.
  - Report the worst relative deviation.
- Output one PASS/FAIL line per example, with reasons for any failure. Exit non-zero on
  any FAIL.

### 0.4 `examples/cmpexamples.py`: comparison that tolerates round-off

- Inputs: `--ref DIR` (the default is `sampleOutput`), plus optional
  `--expect-change ABS_TOL` and `--allow FILE`.
- **File sets** must match exactly per example. A missing or extra file is
  STRUCTURAL. An example with no reference directory is listed as NOREF, which is not
  a failure.
- **Text files** (txt, kml, geojson, gen, nbr, chd, ndx*, meta.000N, prj):
  - Tokenize into numbers and other text.
  - Non-number tokens must match exactly, with whitespace normalized as in `diff -wb`.
    The only exception is a fixed, documented list of environment lines, which get
    normalized: the DGGRID/GDAL version line and the `type sizes:` line (§0.4 of the
    background).
  - Integers such as cell ids, sequence numbers and counts must match exactly.
  - Line counts must match.
  - Floats are equal when |a − b| ≤ one unit in the last printed place. With d
    decimals, that is 10^−d. Treat −0 as 0.
  - In `--expect-change` mode, the tolerance is instead max(10^−d, ABS_TOL).
- **Shapefiles:** compare `ogrinfo -al -q` text dumps under the same rules. The `.shx`
  file is an index, so compare it only through record count once `.shp`/`.dbf` pass.
- **Classes per file:**
  - IDENTICAL: byte-identical after the environment-line normalization.
  - ROUNDOFF: only floats differ, all within tolerance.
  - STRUCTURAL: anything else, including an integer that changed or a point that
    binned into a different cell.
- A STRUCTURAL difference can be accepted only through the `--allow` file. Each entry
  there needs a written justification, for example "point 1234 lies 3e-13° from a cell
  boundary; round-off flips its bin". The tool prints any allow entry that no longer
  matches, so stale ones get removed.
- **Report:** per-example and per-file class, the maximum |Δ| with its location, and a
  one-line total, e.g. `IDENTICAL 212 / ROUNDOFF 9 / STRUCTURAL 0 / NOREF 1`. Exit
  non-zero on any unallowed STRUCTURAL.

### 0.5 Run the baseline and create `$MACREF`

1. Run `./cleanexamples.sh`, then `./doexamples.sh`, into
   `$RUNDIR=IVEA_TESTENV/example_runs/<date>_<sha>_baseline/`.
2. Run `checkexamples.py`: it must be all PASS. That includes `dymaxionIcosa`, which
   is validated only.
3. Run `cmpexamples.py --ref sampleOutput`.
   - **If every file is IDENTICAL or ROUNDOFF** (NOREF is allowed for
     `dymaxionIcosa`), this Mac differs from ai00 only by round-off. Run
     `./copyexamples.sh $MACREF`, then write `$MACREF/PROVENANCE.txt` with the step 0.1
     facts, the commit and the `cmpexamples` summary against canonical.
   - **If anything is STRUCTURAL**, that is a platform difference that already exists,
     not ours. **Don't block on it.** Write it up in `$RUNDIR/REPORT.md` for Kevin, with
     each file, the first differing lines and a diagnosis. Then create `$MACREF`
     anyway, provided `checkexamples.py` passed. The Mac reference only has to be
     valid and self-consistent. Kevin can resolve the platform differences later
     through the allow file or an ai00 regeneration.
4. **Self-test the tools:**
   - `cmpexamples.py --ref $MACREF` right after creation must be all IDENTICAL.
   - Corrupt one digit in a scratch copy: it must be ROUNDOFF when inside tolerance
     and STRUCTURAL when outside.
   - Delete a file: it must be STRUCTURAL.

### 0.6 Make Release the default build type (decided; its own commit)

- In `CMakeLists.txt`, use the standard idiom:

  ```cmake
  if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "No build type specified; defaulting to Release")
  endif()
  ```

  It sits before any targets. An explicit `-DCMAKE_BUILD_TYPE=...` still wins, and
  multi-config generators (Xcode, MSVC) are unaffected. Update `INSTALL.md` to say
  Release is now the default.
- Reconfigure the Mac `build/` from scratch (`rm -rf build && cmake -S . -B build`).
  Confirm `flags.make` now contains `-O3 -DNDEBUG`.
- **Rule R, output-changing.** Compare with `cmpexamples.py --ref $MACREF` (the
  unoptimized baseline).
  - Expected: IDENTICAL or ROUNDOFF only, for example from FMA or reordering
    differences on arm64. Any STRUCTURAL difference needs a per-item justification.
  - Also check `checkexamples.py`, which must be all PASS.
  - Also record the wall-clock time before and after, to show the speed-up.
- Keep the unoptimized reference as `$MACREF.unoptimized`, and regenerate `$MACREF`
  from the Release build. From here on, every later rule R run uses the Release build.
- Add this commit to `AI00_QUEUE.md`. A fresh configure on ai00 picks up Release.
  Expect little or no effect on x86-64 long double: x87 arithmetic is 80-bit whether or
  not values are spilled, and the default x86-64 target has no FMA.

### 0.7 Commit P0

- Commit 1: "examples: tolerant compare, validity checks, script hardening". The DGGRID
  source is untouched. As a sanity check, confirm `doexamples.sh` with no argument
  behaves as before.
- Commit 2: "cmake: default to Release build type" (step 0.6).

---

## 3. Phase 1A (detailed): full-precision constants

Goal: every geometric constant in the icosahedron and projection path becomes its
exact closed form, rounded correctly to long double.

- This removes the ~1e-10 accuracy limit.
- It makes ai00's 80-bit results as precise as that hardware allows.
- It lets DGGRID-ISEA be compared with PROJ and DGGAL at ~1e-15, which is what makes
  C1, C2 and C6 real oracles.
- **This step changes output on purpose** (rule R, output-changing).

### 1A.1 Inventory (found by `grep -rnoE '[0-9]*\.[0-9]{7,}'` over `src/lib/dglib` and `src/apps`)

| Constant | Where | Current | Closed form | Value (50 digits via `bc -l`) |
|---|---|---|---|---|
| vertex-ring latitude | `DgProjTriRF.cpp:182,186` (`ico12verts`) | `26.565051177 * M_PI / 180.0` | atan(1/2), already `M_ATAN_HALF` (rad) in `DgConstants.h` | 26.565051177077989351572193720453294671204214299645° |
| default vert0 lat | `SubOpDGG.cpp:227,284`; `DgProjTriRF.h:44` (and comments at :111,:148); `appex.cpp:48` | `58.28252559` | atan(φ): pole at an edge midpoint (§0.3) | 58.282525588538994675786096860226647335602107149823° = 1.0172219678978513677227889615504829220635608769868 rad |
| Snyder g (`DH`) | `DgProjISEA.cpp:121` | `37.37736814` | atan(3 − √5) = atan(2/φ²) | 37.377368140649695642857138494937517570095047381872° |
| Snyder R′/R (`R1`) | `DgProjISEA.cpp:118` | `0.9103832815` | √(4π/(15√3)) / (3 − √5) | 0.91038328150950356822347395337355245137353373771808 |
| `originXOff` | `DgProjISEA.cpp:129` | `0.6022955029` | (√3/2)·√(4π/(15√3)) (half the planar edge) | 0.60229550292762735363937944891744584664375358180458 |
| `originYOff` | `DgProjISEA.cpp:130` | `0.3477354707` | ½·√(4π/(15√3)) (the planar inradius) | 0.34773547074696668535852744828413924043723662369569 |
| WGS84 mean radius (used only when `proj_datum WGS84_MEAN_SPHERE`) | `DgConstants.h:67` (`WGS84_MEAN_RADIUS_KM`, defined but **never referenced**); `SubOpDGG.cpp:407` repeats the literal; it also reaches the shapefile `.prj` (`DgOutShapefile.cpp:125`, printed in metres at 7 decimals) | `6371.0087714` | (2a + b)/3, with a = 6378.137 km, b = a(1 − 1/298.257223563) | 6371.0087714150598325213221998778850522660571036952 km |
| WGS84 authalic radius (the default `proj_datum`) | `DgConstants.h:69` (`WGS84_AUTHALIC_RADIUS_KM`); `SubOpDGG.cpp:405` repeats the literal | `6371.007180918475` (correct to double precision, 1.1e-12 km high; not enough for ai00's 80-bit long double) | a·√(q_p/2), where q_p = 1 + ((1−e²)/(2e))·ln((1+e)/(1−e)) and e² = f(2−f) | 6371.0071809184738979763378457319610626905184106258 km |

Why these closed forms hold:

- **Ring latitude:** the arc between adjacent vertices is atan(2), so with a vertex at
  the pole the ring sits at 90° − atan 2 = atan(1/2).
- **g:** the arc from a face centre to a vertex.
- **Scale constants:** the planar face area must equal the spherical face area
  4πR²/20. That gives the planar circumradius r_c = R·√(4π/(15√3)). Snyder's
  ρ(vertex) = R′·tan g = r_c then gives R′/R.
- **Mean radius:** WGS84 is defined by a = 6378137 m and 1/f = 298.257223563 (plus GM
  and ω). Everything else is derived from those. NGA's tables publish the
  "mean radius of semi-axes" R₁ = (2a + b)/3 rounded to 0.1 mm, as 6371008.7714 m.
  Computed exactly from a and f it is 6371008.771415059832… m. The two differ by
  0.015 mm (2.4e-12 relative). Only `WGS84_MEAN_SPHERE` uses it, and no example does.
  **Decided: use the exact value.** It also changes the `.prj` text written for
  shapefiles with this datum: 7 decimals in metres gives `6371008.7714151`.
- **Authalic radius:** the radius of the sphere with the same surface area as the
  WGS84 ellipsoid. The current literal is right to about double precision.
  **Decided: use the 40-digit value**, which gives ai00 its last bits. It is the
  default datum, so on ai00 this may change output in the final digits of distances
  and areas. On the Mac it should change nothing, since the literal rounds to the same
  double. Rule R there expects IDENTICAL for everything this row affects.
- Record how the two radii were derived: in a comment next to each constant, give the
  formula and the WGS84 defining parameters a = 6378137 m and 1/f = 298.257223563.

Already at full precision, no change: `DgConstants.h` (`M_PHI`, `M_ATAN2`,
`M_ATAN2_2`, `M_ATAN_HALF`, the square roots, `M_PI_180`, `M_AP7_ROT_*`), and all
Fuller constants in `DgProjFuller.cpp`.

**Explicitly out of scope:** tolerances and epsilons such as
`M_EPSILON` and the `0.00000005` "located on another polygon" checks in
`DgProjISEA.cpp`. They are not geometric constants, and changing them changes
behaviour, not precision. Revisit them in P6 if the kernel needs it.

### 1A.2 Single source of truth

- Add the new constants to `DgConstants.h`, in the existing style: 40+ digit `L`
  literals with the closed form in a comment. For example:
  - `M_ATAN_PHI` (rad) and `M_ICOSA_VERT0_LAT_DEG` = atan(φ) in degrees
  - `M_ISEA_G` (rad)
  - `M_ISEA_R1`
  - `M_ISEA_ORIGIN_X_OFF` and `M_ISEA_ORIGIN_Y_OFF`
- Use literals, not runtime `atanl`/`sqrtl` expressions. That way they are correctly
  rounded on every platform, independent of libm, and usable in `constexpr`.
- Replace every use with the named constant:
  - `ico12verts` uses `M_ATAN_HALF` directly (it is already in radians).
  - `DgProjISEA.cpp` uses the new names. `DH` = `M_ISEA_G`, with no `* M_PI_180`.
  - `DgProjTriRF.h`'s default argument and `appex.cpp` use `M_ICOSA_VERT0_LAT_DEG`.
- `SubOpDGG.cpp`: the `dggs_vert0_lat` default becomes `M_ICOSA_VERT0_LAT_DEG`. The
  **preset string** `setPresetParam("dggs_vert0_lat", …)` must be generated from the
  constant, at enough digits to round-trip an 80-bit long double (21 significant
  digits), for example with `dgg::util::to_string` and `std::setprecision(21)`. No
  second hand-typed literal.
- `SubOpDGG.cpp:405,407` repeat the authalic and mean radius literals instead of using
  `WGS84_AUTHALIC_RADIUS_KM` / `WGS84_MEAN_RADIUS_KM` from `DgConstants.h`. Point them
  at the named constants, so each radius is written down exactly once.
- **Long-double hygiene in the same path.** `ico12verts` and `DgProjISEA.cpp` use the
  double-precision `M_PI` macro and unsuffixed `180.0` about 31 times. On ai00 each of
  these rounds to 53 bits. Replace them with long-double constants: add `M_PI_L` to
  `DgConstants.h`, or use `M_PI_180` and `M_2PI`. Limit this to the icosahedron and
  ISEA path (`DgProjTriRF.cpp`, `DgProjISEA.cpp`, `DgGeoSphRF.cpp`). A codebase-wide
  sweep is a separate task.
- A unit check that runs in the P2.4 driver: every new literal equals its closed form
  recomputed with `atanl`/`sqrtl` to within 2 ulp.

### 1A.3 Non-code references to update

- The manual (`dggrid_man_V841.md` lines ~191, ~441, ~510): show the default as
  58.282525588538995 (atan φ), and add a sentence on why.
- The comment in `examples/dymaxionIcosa/dymaxionIcosa.meta:20`.
- Example metafiles that set `dggs_vert0_lat` explicitly to 58.28252559: none found
  except in comments. Re-grep.

### 1A.4 Build and run the examples (rule R, output-changing)

Run rule R steps 1–3 as usual.

### 1A.5 Expected change and how to judge it

The expected size of the change:

- Vertex positions move by ≲1.1e-9°.
- The Snyder scale and offset constants change by ≲5e-11 relative.
- Together, projected geographic coordinates should move by **≲1e-8°**, and planar
  coordinates by ≲1e-10 of an edge.

What to check:

- Run `cmpexamples.py --ref $MACREF --expect-change 1e-8`.
- Examples printed at 5–7 decimals should be mostly IDENTICAL, with a few
  last-digit ROUNDOFF flips.
- `hiRes` (precision 12) will change in digits 8–12. That's expected, and must stay
  within 1e-8.
- Logs change only if a parameter echo prints the new vert0 at more digits. The echo
  currently prints 6 significant digits, so they should be unchanged.
- Any STRUCTURAL difference, i.e. a binned point changing cell, has to be shown to be a
  point within about 1e-8° of a cell boundary. Justify it in the allow file, or treat
  it as a bug.
- Also run `cmpexamples.py --ref sampleOutput --expect-change 1e-8` against canonical.
  It is informational only and does not block.
- `checkexamples.py`: all PASS. The equal-area worst deviation should improve or stay
  the same. Record it before and after.

### 1A.6 Accept and commit

- Regenerate `$MACREF` (keeping the old one as `$MACREF.<oldsha>`).
- Queue an ai00 regeneration (§1.2).
- Commit: "use full-precision icosahedron and ISEA constants". The message should give
  the closed forms and the `cmpexamples` summary.
- Add a CHANGELOG entry noting that ISEA/Fuller geographic output changes at the
  ≤1e-8° level.

---

## 4. Phase 1B (detailed): metafile parameters and the IVEA skeleton

Goal: `dggs_proj IVEA` and `dggs_type IVEA3H|IVEA4H|IVEA7H|IVEA43H|IVEA4T|IVEA4D` work
end to end. They produce output **identical to ISEA** because the skeleton delegates to
the Snyder code. That proves the plumbing, and gives the kernel a place to drop in
later without touching any other code.

### 1B.1 Metafile parameters (`src/apps/dggrid/SubOpDGG.cpp`)

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

### 1B.2 Projection dispatch (`src/lib/dglib/lib/DgIDGGBase.cpp:170-180`)

- Add `#include <dglib/DgProjIVEA.h>` and an `else if (projType() == "IVEA")` branch.
- *Recommended small refactor:* move the string→projection dispatch into one factory,
  e.g. `DgIcosaProj* DgIcosaProj::makeIcosaProj(const std::string& projType, geoRF,
  projTriRF)` in a new `DgIcosaProj.cpp`, and have `createConverters()` call it. This
  keeps "the list of projections" in one library location. Optional; keep it if it
  stays tiny.

### 1B.3 Skeleton classes

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

### 1B.4 Other projType-string consumers

- `grep` showed that the only `projType() ==` comparison is in `DgIDGGBase.cpp`.
  Re-check after the change: `grep -rn '"ISEA"\|"FULLER"' src | grep -v proj4lib`.
- Default `projType = "ISEA"` arguments in the IDGGS headers stay as they are.
- `DgIDGGutil.cpp` and `DgIDGG.cpp` include the projection headers but don't dispatch.
  No change is needed.

### 1B.5 Build, smoke test, and run the examples (rule R, output must be IDENTICAL)

- Run the full rule R. `cmpexamples.py --ref $MACREF` must be **all IDENTICAL**:
  P1B must not change any existing output.
- Also run these IVEA-vs-ISEA pair checks, in `IVEA_TESTENV/p1b_smoke/`. Each pair is
  identical except for `dggs_type` or `dggs_proj`, and is compared with
  `cmpexamples.py`-style rules. The only allowed difference is the DGGS name string.
  - `GENERATE_GRID` with `ISEA3H` vs `IVEA3H` at res 3, with KML and GeoJSON output.
  - The same for 4H, 7H, 43H, 4T and 4D.
  - A `CUSTOM` metafile with `dggs_proj IVEA`.
  - `TRANSFORM_POINTS` GEO→SEQNUM and GEO→Q2DI on
    `examples/transform/inputfiles/20k.txt`.
- Negative test: `dggs_type IVEX3H` must fail at parameter validation, and the preset
  parser's Fatal branch must be unreachable from valid choices.

### 1B.6 Pin down the ProjTri face frame (needed by P3 and P5)

This can be a throwaway driver run under WS-D. For each face f and each j ∈ {0,1,2},
forward-project a point at 1 − 1e-9 of the way from the face centre to `icotri[f][j]`,
using `DgProjISEA`, and record (x, y).

- Expected: j=0 → apex (0.5, √3/2), j=1 → (0,0), j=2 → (1,0), for all 20 faces.
- Also record which sub-triangle convention falls out: the (V, M, C) index order
  around the centroid.
- Write the result into this file (§0.3), replacing "derivation".

### 1B.7 Commit

Commit P1B as one small commit ("add IVEA projection parameter and skeleton"). Include
the rule-R summary. Add a CHANGELOG line, marked as work in progress.

---

## 5. Phase 2 (detailed): test environment

Location: **outside the repo**, at `/Users/sahrk/CODE/IVEA_TESTENV/`. It is not tracked
in DGGRID and can become its own git repo.

```
IVEA_TESTENV/
  README.md              # pinned commits, build commands, how to run
  sampleOutput.mac-arm64/  # $MACREF: validated Mac example reference (P0) + PROVENANCE.txt
  example_runs/          # one dir per rule-R run: logs, check/cmp reports
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
- Output uses the same record order, `%.17g` for every float (`%.21Lg` for long double
  on ai00), and `nan nan` on failure (never skip a line).
- Sphere with R = 1 everywhere. PROJ: `+R=1`. DGGRID: pass `DgGeoCoord` directly, since
  the projection is on the unit sphere.
- **Explicit, identical orientation in all three:**
  - vertex lat = atan(φ) in full precision (58.282525588538994676°). After P1A this
    is DGGRID's default too.
  - lon = 11.25°
  - azimuth = 0
  - Pass these explicitly anyway, so a changed default never goes unnoticed.
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
- It also runs the P1A constant self-check (step 1A.2).
- Model it on `src/apps/appex/appex.cpp`.
- Keep it outside the repo for now. It may later become `src/apps/projtest` if we want
  it in CI (open question Q6).

### 2.5 Point sets (`gen_points.py`, seeded and deterministic)

1. N = 10⁶ uniform random points on the sphere, using a fixed seed.
2. Special loci, computed from the *same* icosahedron as DGGRID, in closed form. Also
   compute them with the pre-P1A truncated constants, to measure P1A's effect.
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

## 6. Phase 3: frame calibration (WS-E), mapping everything into one frame

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

## 7. Phase 4: cross-comparison (WS-E)

Everything is compared in the hub frame. Report the max, p99.9 and RMS error, plus the
worst 20 points with their loci. "Before P1A" numbers are recorded once, for the
record. The "after P1A" numbers are what count.

| # | Comparison | Expected (after P1A) | What it proves |
|---|-----------|----------|----------------|
| C1 | DGGRID-ISEA vs PROJ-ISEA (fwd & inv) | ≈1e-15 (≈1e-10 before P1A) | frame calibration; baseline |
| C2 | DGGRID-ISEA vs DGGAL-ISEA | ≈1e-15 (≈1e-10 before P1A) | same, for DGGAL |
| C3 | PROJ-ISEA vs DGGAL-ISEA | ≈1e-15 | ports agree |
| C4 | PROJ-IVEA vs DGGAL-IVEA (fwd & inv) | ≈1e-15 | **do they agree?** |
| C5 | PROJ `dsea` (non-dual) composed with the per-sub-triangle affine map vs PROJ-IVEA | ≈1e-15 | the uniqueness argument in §0.1; the dual unfolding |
| C6 | P5 extracted kernel (ISEA mode) vs DGGRID-ISEA | ≈1e-15 | **independent oracle** for the kernel |
| C7 | P5 kernel (IVEA mode) vs PROJ-IVEA & DGGAL-IVEA | ≈1e-15 | the extracted code is correct |
| C8 | Round trip inv(fwd(p)) for every implementation | ≈1e-15 rad | internal consistency |

Run the comparisons on this Mac, where everything is double. Repeat C1, C6 and C8 on
ai00 (80-bit) for DGGRID and the kernel, to confirm the long-double path gains
precision there.

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

## 8. Phase 5: kernel extraction (WS-G, can start now)

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
   index convention** in both directions (fixed in step 1B.6).
3. **The radial vertex is a parameter**: {FaceCentre = ISEA, Vertex = IVEA,
   EdgeMidpoint = RTEA}. The permutation of (V, M, C) into kernel roles (A radial, B,
   C) lives in one table. There is no per-projection code path.
4. All constants come from the P1A `DgConstants.h` set: φ, atan(1/φ), acos(√((φ+1)/3)),
   atan(2/φ²), and the 6° sub-triangle area (4π/120 sr). No new hand-typed literals.
5. Cross-read DGGAL's trigonometric derivation against the paper. Document the
   equations in comments, and cite van Leeuwen & Strebe 2006, Snyder 1992 and Recht
   2021.
6. Validate with C6 and C7 in P4 before integration.

## 9. Phase 6: integration into DGGRID (WS-F)

1. Add `DgIcosaSliceDice` (the kernel and the 120-triangle tables) to dglib. The
   precomputed geometry lives with `DgSphIcosa`, so ISEA, IVEA and later RTEA share it.
2. `DgProjIVEAFwd/Inv::convertTypedAddress`: replace the skeleton bodies with the
   kernel in Vertex mode. The public class shape from P1B stays the same.
3. Leave `DgProjISEA` (legacy Snyder, now with P1A constants) **unchanged**, so ISEA
   output stays byte-stable. Keep a kernel-in-ISEA-mode path as a test-only check
   (C6). Whether to retire the legacy code is open question Q2.
4. **Rule R.** Every existing example must be IDENTICAL against `$MACREF`, since P6
   must not change ISEA or Fuller output. Then add the new IVEA examples (P8). Those
   are validated with `checkexamples.py` (equal area included), added to `$MACREF`,
   and queued for ai00.
5. Grid-level regression. These invariants follow from the design:
   - **Every non-GEO output is identical between ISEA and IVEA** for the same grid
     parameters: SEQNUM, Q2DI, Q2DD, PROJTRI, PLANE, neighbours, children, and
     hierarchical indices. Only the projection between PROJTRI and GEO changes. Diff
     these automatically.
   - GEO outputs differ from ISEA, and match PROJ/DGGAL IVEA at the point level (C7
     through the full DGGRID pipeline, using the driver).
   - **Equal area at the cell level:** at res n, every hexagon has the same spherical
     area, and each of the 12 pentagons has the expected fraction. Check with the
     `checkexamples.py` area routine over GENERATE_GRID polygon output, for aperture
     3, 4 and 7 and for triangle and diamond grids.
   - Cell-set match with DGGAL's IVEA3H/IVEA7H: nearest-neighbour matching of cell
     centroids and vertex sets. No zone-ID mapping is needed.

## 10. Phase 7: ellipsoidal versions (after the authalic-latitude work lands)

- Keep projections **purely spherical**. Handle the ellipsoid as a separate
  geodetic↔authalic converter in the RF network, placed in front of the
  `DgGeoSphRF` → ProjTri step. It then applies to ISEA, IVEA and Fuller in the same way,
  with no per-projection ellipsoid code.
- Orientation semantics must be decided with the other agent: is `dggs_vert0_lat`
  authalic or geodetic? PROJ uses about 58.40° geodetic, which is atan(φ) authalic, and
  11.20° lon on the ellipsoid. Compare against PROJ `+proj=ivea +ellps=WGS84` and
  DGGAL's authalic path, reusing the P2–P4 harness unchanged.
- **Rule R** applies. This phase adds ellipsoidal examples, and any change to the
  spherical path must leave existing examples IDENTICAL.

## 11. Phase 8: documentation (WS-H)

- Manual (`documentation/source/dggrid_man_V841.md`):
  - the `dggs_proj` description (§3 "Specifying the projection")
  - the parameter table (line ~432)
  - the `dggs_type` table and Appendix B presets
  - the IVEA description and citations
  - the P1A constants
- `CHANGELOG.md`, and README if it lists projections.
- Examples: `examples/ivea3hGen` (or similar), added to `examples.lst` and
  `examplesNoGDAL.lst`.
  - **Rule R**, plus a validity PASS, including equal area.
  - Their canonical `sampleOutput` comes from ai00 (§1.2). Until then they are NOREF
    against canonical and present in `$MACREF`.
- Doxygen comments on the new classes.

---

## 12. Open questions / decisions for Kevin

- **Q1.** *Decided (rev 2):* all constants go to full closed forms, as the early step
  P1A.
- **Q2.** Once C6 shows the kernel matches, replace the legacy Snyder ISEA with the
  kernel in ISEA mode? That gives one code path, but ISEA output changes at the
  round-off level. It would be another output-changing step under rule R.
- **Q3.** Expose RTEA too? It is almost free, since it only needs a different radial
  vertex, but it needs its own validation.
- **Q4.** Should the IVEA default orientation be the same as ISEA's? This plan assumes
  yes, matching PROJ and DGGAL.
- **Q5.** Add an IVEA-based analogue of the IGEO7 preset (DGGAL has IVEA7H_Z7)?
- **Q6.** Should the direct-projection driver (§2.4) move into the repo as a test app
  and CI check?
- **Q7.** *Decided:* make Release the default (P0.6). ai00 currently has the same
  empty build type as the Mac, and picks up Release on reconfigure after P0.6. Never
  block on ai00 output: test against the local `$MACREF` until Kevin updates ai00.
- **Q8.** *Decided:* use the exact (2a + b)/3 for `WGS84_MEAN_RADIUS_KM`, and the
  40-digit authalic radius for `WGS84_AUTHALIC_RADIUS_KM` (P1A).
