# WGS 84 Authalic Latitude: Input and Output Plan

## Scope and governing rule

Add independent metafile controls for the interpretation of geographic input
and geographic output. DGGRID continues to run its existing spherical
projections, grids, and indexing on the sphere selected by `proj_datum`.
Selecting WGS 84 at either boundary inserts an authalic latitude conversion
between that boundary and the spherical projection. It does not replace ISEA,
Fuller, or any future projection. The conversion must be available to every
projection through the same reference-frame graph.

The other agent's IVEA plan covers the Slice-and-Dice projection itself. Once
IVEA uses DGGRID's spherical geographic backing frame, these input and output
controls should work with it without projection-specific authalic code.

Use these direction names consistently:

- **Geodetic to authalic (input):** WGS 84 ellipsoid longitude/geodetic latitude
  `λ, φ` to the WGS 84 authalic sphere longitude/authalic latitude `λ, β`.
- **Authalic to geodetic (output):** inverse-projected sphere `λ, β` to WGS 84
  ellipsoid `λ, φ` before serialization.

Longitude does not change. The authalic mapping preserves area between the
ellipsoid and its authalic sphere. The complete projection is equal-area on the
ellipsoid only if its spherical projection is equal-area. Thus ISEA qualifies;
Fuller still does not.

## Metafile contract

Add two independent choice parameters, each defaulting to `SPHERE`:

| Parameter | `SPHERE` | `WGS84` |
|---|---|---|
| `input_geographic_mode` | Interpret geographic input latitude on the projection sphere, as today. | Interpret geographic input as WGS 84 geodetic and convert `φ → β` before spherical projection. |
| `output_geographic_mode` | Emit geographic output latitude on the projection sphere, as today. | Convert inverse-projected sphere latitude `β → φ` before geographic output. |

The modes are independent. Support and test all four combinations:

| Input | Output | Meaning |
|---|---|---|
| `SPHERE` | `SPHERE` | Existing behavior; default. |
| `WGS84` | `SPHERE` | Accept WGS 84 geodetic input; emit spherical coordinates. |
| `SPHERE` | `WGS84` | Accept spherical input; emit WGS 84 geodetic coordinates. |
| `WGS84` | `WGS84` | Accept and emit WGS 84 geodetic coordinates. |

Keep `proj_datum` and `proj_datum_radius` as the existing *internal projection
sphere* controls. Do not add `WGS84_ELLIPSOID` to `proj_datum`. The selected
spherical projection remains unchanged. Because this task specifically maps
the WGS 84 ellipsoid to the sphere with the WGS 84 authalic radius, require
`proj_datum WGS84_AUTHALIC_SPHERE` whenever either new mode is `WGS84`.
Reject `WGS84_MEAN_SPHERE` and `CUSTOM_SPHERE` in that case with a clear setup
error. Both remain valid when the input and output modes are `SPHERE`.

Apply the input mode only to longitude/latitude values: geographic point or
polygon files; `GEO` addresses; geographic values supplied directly in the
metafile, including `dggs_vert0_lon/lat` and `region_center_lon/lat`; and
geographic clipping inputs. Preserve internally generated or preset spherical
orientation values. Define and test how a partially specified longitude/latitude
pair combines with its default before conversion. Do not transform grid
indexes, projected coordinates, radii, or an azimuth merely because they
appear in a metafile.

Apply the output mode to every geographic coordinate produced from the grid or
an inverse projection: transformed `GEO` addresses, centers, vertices,
boundaries, random points, parent/child geometries, and geographic coordinates
written to text, AIGen, shapefile, GDAL, GeoJSON, KML, and orientation outputs.
The boundary inventory below must verify this list and identify exceptions.

## Existing DGGRID hooks to use

- `DgEllipsoidRF` already holds ellipsoid axes and derived flattening and
  eccentricity; `DgGeoSphRF` derives from it for the spherical backing frame.
  Use a thin concrete WGS 84 `DgEllipsoidRF` derivative, with `a = 6,378,137 m`
  and inverse flattening `298.257223563`, rather than creating a second
  ellipsoid model.
- `DgConverter<DgGeoCoord, long double, DgGeoCoord, long double>`,
  `Dg2WayConverter`, and `DgRFNetwork` already provide the two directional
  coordinate edges and path composition. Register one WGS 84↔authalic-sphere
  pair and select its endpoints independently for input and output.
- `DgDegRadConverter` already accepts generic geographic reference frames.
  Generalize the degree adapter and only the I/O interfaces that currently
  insist on `DgGeoSphDegRF`; preserve their spherical callers.
- Keep `DgIDGGSBase` backed by `DgGeoSphRF`. `DgGeoProjConverter` handles
  geographic↔planar conversion; its geocentric latitude switch is unrelated
  to authalic latitude and is not the right hook for this transform.

`SubOpDGG::addressTypeToRF` currently uses one `GEO` degree frame for both
directions. Refactor the caller contract so `SubOpIn` requests the input
geographic frame and `SubOpOut` requests the output geographic frame. Do not
hide the mode in a global `deg()` accessor that cannot represent asymmetric
settings. Create the WGS 84 frame if either boundary needs it, and retain the
existing spherical frame and its projection converters in all cases.

### Shared interface contract for implementing agents

The integration owner should publish a small typed API before input/output
agents edit their callers. It must expose separate input and output geographic
degree frames, direction-aware `GEO` selection in `addressTypeToRF` (including
child and parent frames), and the unchanged internal spherical frame. Avoid
making a shared `deg()` accessor mean different things at different call sites.
Both selectors may point to the same WGS 84 frame when both modes are `WGS84`.

`SubOpDGG::executeOp` currently creates the sphere first, as the network's
ground frame. Register the optional WGS 84 frame and both authalic converter
edges before constructing any composed paths or the DGGS. Verify graph paths
from WGS 84 input to the selected grid and from the grid to WGS 84 output.
`DgSeriesConverter` may cache composed paths, so registration order matters.

## Detailed first steps

### 1. Establish a behavioral baseline and map the boundaries

Build DGGRID with GDAL on and off where available. Record representative
outputs for ISEA and Fuller with existing `proj_datum` choices and `GEO`
transform operations. Identify all uses of geographic frames in
`SubOpDGG`, `SubOpIn`, `SubOpOut`, `SubOpGenHelper`, and the format readers and
writers. For each source or sink, record whether it reads user geographic
coordinates, operates internally on the sphere, or emits geographic output.

Pay particular attention to orientation values, region-center placement,
random placement, clipping and densification, antimeridian wrapping, output
cell construction, child/parent output, and any values written into generated
metafiles. Record whether an edge is currently treated as a spherical great
circle; setting `input_geographic_mode WGS84` must not silently imply that
edge densification now follows an ellipsoidal geodesic.

Begin at these known entry points:

- `SubOpIn::executeOp` calls `addressTypeToRF` for point input. Several
  clipping paths in `SubOpGenHelper.cpp` separately construct AIGen,
  shapefile, and GDAL readers against `dgg.geoRF()` or `op.dggOp.deg()`.
- `SubOpDGG::setupOp` reads `dggs_vert0_lon/lat`; `orientGrid` reads
  `region_center_lon/lat`. Separate user-supplied geographic positions from
  built-in spherical defaults and generated positions.
- `SubOpOut::executeOp` calls `addressTypeToRF` for address output and passes
  `op.dggOp.deg()` directly to cell, point, collection, random-point, and GDAL
  writers. `makeCell` and `genRandPts` handle geometry outside the address
  selector.
- `DgGeoSphDegRF`, `DgInGdalFile`, `DgInShapefile`,
  `DgOutLocFile::makeOutLocFile`, and `DgOutShapefile` have sphere-specific
  types or checks that may block an ellipsoidal geographic frame.

Inventory CRS declarations in shapefile `.prj`, GDAL layers, GeoJSON, KML,
and textual reports. Set numeric acceptance tolerances for conversion,
round trips, and end-to-end results. Capture the existing default output as a
regression baseline.

### 2. Add both metafile parameters before changing coordinates

Register `input_geographic_mode` and `output_geographic_mode` as choice
parameters with values `SPHERE` and `WGS84`, both defaulting to `SPHERE`.
Parse them into a small shared enum/typed configuration used by the
application boundary selectors. Validate the WGS 84 authalic sphere radius
requirement above during setup, including each asymmetric combination. Do
not alter `dggs_proj`, the current `proj_datum` choices, or their defaults.

Add parser/setup tests for all four mode combinations, the default, both
invalid values, and the radius compatibility rule. The repository currently
has no formal CTest suite; introduce the smallest maintainable CTest target
needed for these tests. Keep the new WGS 84 paths at identity while the
skeleton milestone is under review, and do not present that interim build as
a completed WGS 84 coordinate conversion.

Handoff result: a written contract in the shared API/comments and tests
covering parameter names, defaults, validation, coordinate scope, and the
two geographic-frame selectors.

### 3. Add no-op ellipsoid and converter skeletons

Add a thin WGS 84 geographic frame subclass of `DgEllipsoidRF`. Centralize
WGS 84 axes and the derived authalic sphere radius. Add one geodetic→authalic
and one authalic→geodetic `DgConverter` implementation plus the usual
`Dg2WayConverter` registration facade. Initially both copy latitude and
longitude unchanged. Add them to `src/lib/dglib/CMakeLists.txt`; verify the
non-CMake build, whose dglib Makefile discovers `*.cpp` automatically.

Test the converter edge directly in both directions, with degrees/radians
handled by the existing degree conversion mechanism. Test equator, poles,
ordinary latitudes, longitude preservation, and graph composition. Explicitly
define handling for non-finite and out-of-range latitude; avoid accidental
normalization that changes valid coordinates.

Handoff result: compiling frame and converter skeletons, registered graph
edges, and identity tests. Fix the converter interface at this stage; its
numeric bodies can be replaced after the reference comparison.

`DgEllipsoidRF::dist()` currently returns a placeholder. Keep distance and
grid statistics on the internal sphere unless this work also supplies a real,
documented ellipsoidal distance method. No WGS 84 mode may report a distance
computed through that placeholder.

### 4. Wire input and output selection independently with no-op converters

Create the spherical frame as before. If either mode is `WGS84`, create the
WGS 84 frame and register its converter pair with the sphere. Bind geographic
input to the spherical degree adapter or WGS 84 degree adapter according to
`input_geographic_mode`; bind geographic output independently according to
`output_geographic_mode`. Keep the selected `dggs_proj` operating on the
spherical frame in every case.

Feed user-supplied metafile latitude/longitude inputs through the same
input-side choice. Convert them once before they affect the internal
icosahedron orientation or clipping; keep built-in spherical orientation
defaults in the internal frame. Refactor sphere-specific I/O signatures only
where required by the new external frame; keep old paths source-compatible.
Test every combination with both ISEA and Fuller while converters are
identity, including `GEO`
transform operations and at least one generated geometry format. Confirm no
unintended latitude change or change in legacy mode.

Handoff result: the four mode combinations each select the intended input
and output frames. Identity tests should exercise a direct `GEO` transform,
one clipping input, and generated polygon and point output.

### 5. Build an independent authalic comparison environment

Pin DGGAL, PROJ, and/or `rhealpixdggs-py` versions outside the DGGRID source
tree. Record exact commits/releases, build commands, constants, units, axis
order, and licenses. Extract the forward and inverse formulas and compare
their assumptions: exact meridional-area formula versus series, inverse
iteration/series, pole handling, clamping, and stopping criteria. DGGRID's
bundled old PROJ.4 helpers can inform history but are not an independent
oracle.

Run a common deterministic corpus through every available implementation:
equator, exact and near poles, both hemispheres, a global latitude sweep,
representative longitudes, maximum-difference latitudes, and randomized
samples. Compare forward values, inverse values, and round trips in angular
error and physical distance. Report whether the projects agree, where they
disagree, and by how much. Check the results against the defining authalic
area equation and a high-precision oracle, rather than choosing by majority
vote. Preserve attribution and compatible licenses for any adapted code.

## Remaining steps

### 6. Implement the verified mathematics

Replace the identity converter bodies. Compute authalic latitude from the
ellipsoidal meridional-area function `q(φ)` and `q_p`; implement the inverse
with the method selected by the comparison study. Use `long double` where
the project already does, without assuming it has more precision on every
platform. Centralize constants, domain checks, stable evaluation near zero
and the poles, clamping, and bounded inverse convergence.

### 7. Verify all four modes through each projection

Promote reference vectors to versioned numerical tests. Test symmetry,
monotonicity, exact equator/poles, longitude invariance, dense round trips,
and coordinate bounds. Run the four input/output combinations for ISEA and
Fuller in transform, grid generation, clipping, and representative input and
output formats. Use a shared test pattern that can be applied to future
projections. Verify that only the input mode changes incoming geographic
latitude and only the output mode changes emitted geographic latitude.

Check two independent end-to-end invariants: the same physical location
entered as WGS 84 latitude `φ` or its corresponding spherical latitude `β`
must select the same cell when placed away from cell boundaries; and the same
cell emitted in `SPHERE` or `WGS84`
mode must have corresponding output latitudes `β` and `φ`. For a fixed cell
index, changing only the input mode must not change its output geometry. For
a fixed input point, changing only the output mode must not change its cell
index.

For ISEA, compare ellipsoidal cell/region areas with corresponding sphere
areas at several latitudes and resolutions. Separate transformation,
projection, densification, and polygon-area errors. For Fuller, verify
conversion and round trips without making an equal-area claim.

### 8. Make output metadata reflect the output mode

When `output_geographic_mode WGS84`, geographic results use WGS 84 geodetic
latitude. EPSG:4326 identifies the 2D geographic CRS, although its formal
axis order is latitude, longitude. DGGRID coordinate sequences are generally
longitude, latitude, so test the axis mapping in every writer:

- Shapefile `.prj`: emit WGS 84 ellipsoidal WKT rather than the spherical WKT.
- GDAL layers: supply a WGS 84 spatial reference instead of the current null
  spatial reference, with an explicit traditional GIS axis mapping as needed.
- GeoJSON: use RFC 7946 longitude/latitude WGS 84 semantics (often described
  as OGC:CRS84 axis order); do not add an obsolete `crs` member.
- KML: confirm its WGS 84 longitude/latitude contract and geographic values.
- Text/AIGen, generated metafiles, and reports: identify the output
  coordinate model where they declare one.

When `output_geographic_mode SPHERE`, keep spherical metadata regardless of
the input mode. Review GeoJSON and KML format conventions explicitly: both
normally imply WGS 84, while the legacy DGGRID output can contain spherical
latitude. Preserve the existing default behavior and document that limitation
without falsely declaring the sphere to be EPSG:4326.

### 9. Document and release

Update the metafile parameter reference with the four mode combinations,
the `proj_datum` compatibility rule, orientation and clipping semantics,
conditional equal-area guarantee, and examples showing asymmetric modes.
Run legacy examples plus the new tests with and without GDAL. Inspect
generated metadata with an independent reader. Profile bulk conversion only
after numeric correctness is established.

Use isolated build directories for verification, for example
`cmake -S . -B /tmp/dggrid-authalic-nogdal -DWITH_GDAL=OFF`, then
`cmake --build /tmp/dggrid-authalic-nogdal` and
`ctest --test-dir /tmp/dggrid-authalic-nogdal --output-on-failure` once the
test target exists. Repeat with `WITH_GDAL=ON` where GDAL is installed.
Record the DGGRID commit, reference-library versions, and the exact build
commands with the final test report.

## Parallel agent assignments and handoff gates

The work below is intentionally split by file ownership. An agent receiving
one assignment should deliver the named artifact plus a short note listing
changed files, tests run, any uncovered geographic path, and any decision
needed from the integration owner. Do not let two agents edit the same
`SubOpDGG` or format-writer file in the shared workspace at the same time.

| Owner | Start condition | Files / concrete task | Completion evidence |
|---|---|---|---|
| G — integration contract | Start immediately | Own `SubOpDGG.h/.cpp`: register/parse the two parameters, validate `proj_datum`, define input/output selectors, create frames, register graph edges. Coordinate the degree-adapter interface with A. | Four-mode parser tests; graph path in both directions; unchanged default baseline. Publish API to C/D/E. |
| A — core converter | Start alongside G after agreeing on frame names | Own `src/lib/dglib/include/dglib/` and `src/lib/dglib/lib/` WGS 84 frame/converter and degree-adapter code plus `src/lib/dglib/CMakeLists.txt`; verify the non-CMake wildcard build. First deliver no-op pair; after B's report, fill numeric bodies. | Direct forward/inverse and graph tests; versioned numeric vectors after math phase. |
| B — reference comparison | Start immediately, independent of DGGRID code edits | Own a separate reference/test workspace and a written comparison report. Pin/build DGGAL, PROJ, and/or rHEALPix; run identical vectors and a high-precision oracle. | Reproducible commands, source versions/licenses, machine-readable results, discrepancies, selected algorithm and tolerance rationale. |
| C — geographic input | Inventory can start immediately; code wiring after G/A publish the selector/converter APIs | Own `SubOpIn.*`, input reader classes, and input-side paths in `SubOpGenHelper.cpp`; route point/polygon files, `GEO` addresses, and clipping to the selected input frame. Specify the metafile/orientation cases for G to implement in `SubOpDGG`. | Input-side tests with `SPHERE` and `WGS84`; demonstrate conversion occurs once for points, orientation, and a clipping region. |
| D — geographic output | Inventory can start immediately; code wiring after G/A publish APIs | Own `SubOpOut.*` and output geometry routing; cover `GEO` addresses, cell/point/collection/random-point calls, children/parents, generated orientation values, and helper calls that bypass `addressTypeToRF`. Coordinate any `SubOpDGG` change through G and writer API change through E. | Output-side tests in both modes; every geographic writer receives the selected output frame and no authalic latitude leaks in `WGS84` mode. |
| E — writer interfaces and CRS metadata | Audit can start immediately; implementation after output-mode API is fixed | Own output writer classes and factory signatures, including `DgOutLocFile`, `DgOutShapefile`, `DgOutGdalFile`, GeoJSON/KML writers, and metadata tests. Coordinate the new writer API with D. | Independent inspection of WGS 84 `.prj`/GDAL SRS; sphere metadata preserved; GeoJSON/KML convention documented. |
| F — regression and documentation | Baselines and test design can start immediately | Own `documentation/`, example metafiles, and end-to-end/CTest integration tests. Avoid A's core test files. | Default-output baseline, four-mode × ISEA/Fuller matrix, GDAL on/off runs, updated parameter reference and examples. |

### Parallel schedule

1. **Before shared-code edits:** G fixes the parameter names, mode semantics,
   radius rule, and selector signatures. In parallel, B starts the external
   comparison, F captures legacy outputs, and C/D/E perform read-only
   boundary audits. These activities have no code dependency on each other.
2. **Skeleton wave:** A builds the no-op frame/converter pair while G wires
   parameters and the graph; B continues the reference study. C and D can
   prepare file-local interface changes and tests against the published
   selector API. E can prepare metadata expectations. Merge G and A together
   and verify the identity path before switching geographic call sites.
3. **Boundary wave:** After the skeleton compiles, C, D, and E implement their
   separate input, output, and metadata files concurrently. B finalizes the
   algorithm comparison. F adds the four-mode integration matrix concurrently.
   G resolves any shared `SubOpDGG` or `SubOpGenHelper` edits in one place.
4. **Mathematics and final integration:** A replaces the two identity bodies
   using B's result. G integrates A/C/D/E/F, resolves graph and lifetime
   issues, and runs the acceptance matrix. Documentation and numeric fixtures
   are finalized against the implemented behavior.

The required gates are: **contract fixed → identity graph compiles → input and
output paths each pass in isolation → numerical converter passes independent
reference checks → all four modes pass end-to-end → metadata and docs match
the emitted coordinates.** A later IVEA implementation should need only the
same projection-independent end-to-end checks, not a second authalic
converter.

## Decisions to record

- The exact degree-adapter API that allows both `DgGeoSphRF` and the WGS 84
  `DgEllipsoidRF` derivative as geographic endpoints.
- Treatment of spherical great-circle versus WGS 84 geodesic edges in
  densification, clipping, and any length statistic.
- The inverse numerical method, tolerance, convergence policy, source
  provenance, and cross-platform precision expectations.
- Format-specific axis order and the behavior of GeoJSON/KML in legacy
  `SPHERE` output mode.

Resolve these in the shared frame contract and tests, rather than by adding
projection-specific latitude code.
