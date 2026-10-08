# DGGRID v9.03b: code bugs found during the manual review (to debug later)

Found while checking the manual against the v9.03b source (tag `v9.03b`). "Reproduced" means run on a v9.03b build with GDAL 3.13.3 (macOS). Repro files are in the session scratchpad and are inlined below. File/line references are to v9.03b.

## Crashes (reproduced)

### 1. Segfault: TRANSFORM_POINTS reading a plain CSV through GDAL
- Exit code 139, no output.
- `pts.csv`:
```
lon,lat
-75,40
10,40
```
- Metafile:
```
dggrid_operation TRANSFORM_POINTS
dggs_type ISEA4H
dggs_res_spec 3
input_address_type GEO
output_address_type SEQNUM
point_input_file_type GDAL
input_files pts.csv
output_file_name out1.txt
```
- Same input as GeoJSON points works. A non-GDAL text file given to GDAL gives a clean "Invalid GDAL data model" error.
- Suspected cause: the CSV layer has no geometry; `DgInGdalFile` (via `SubOpIn::getNextLoc`, `SubOpIn.cpp` ~285) probably dereferences a null geometry. Not yet debugged.

### 2. Segfault: COARSE_CELLS with a space-delimited multi-part address
- Exit code 139, 0-byte output.
- Metafile (default `input_delimiter`):
```
dggrid_operation GENERATE_GRID
dggs_type ISEA4H
dggs_res_spec 4
clip_subset_type COARSE_CELLS
clip_cell_res 2
input_address_type Q2DI
clip_cell_addresses 1 0 3
cell_output_type AIGEN
cell_output_file_name bug2
```
- Works with `input_delimiter ","` and `clip_cell_addresses 1,0,3` (26 cells, same as SEQNUM 5). With `1,0,3` and the default delimiter there is a clean "DgQ2DIRF::fromString() invalid input" error.
- Cause (from reading): `SubOpGenHelper.cpp:1269` splits `clip_cell_addresses` on white space, so `1 0 3` becomes three one-token addresses; `fromString` (line ~1288) fails on each and the failure is not handled.
- Fix idea: validate the parse result / emit a clean fatal error; or split on white space only when `input_delimiter` is not a space.

## Wrong results (reproduced)

### 3. Multiple placements: later placements bin no points
- `BIN_POINT_VALS`, `dggs_num_placements 2`, `dggs_orient_specify_type RANDOM`, two input points: `valsout.txt.0001` has 2 lines, `.0002` has 0.
- Cause (from reading): the input file is opened once in `inOp.execute` and is at EOF after the first placement; `SubOpBasicMulti::execute` loops over grids without resetting it. Probably also affects GENERATE_GRID_FROM_POINTS, BIN_POINT_PRESENCE and TRANSFORM_POINTS.

### 4. `output_cell_label_type ENUMERATION` gives every cell label 0 outside GENERATE_GRID
- `BIN_POINT_VALS` with ENUMERATION: text file lines `0 2.5` and `0 1.5`; GeoJSON names `"0"`, `"0"`.
- Cause (from reading): the label uses `nCellsAccepted`, which only `SubOpGen` increments (`SubOpOut.cpp` ~260).
- Either maintain the counter in the other operations or reject ENUMERATION for them.

## Likely bugs (from reading, not reproduced)

5. **Indexing parent/children output files miss the placement/split suffixes.** `SubOpOut::executeOp` (`SubOpOut.cpp` ~925-935) appends `.NNNN` / `_N` to the data, cell, point, collection, randpts, neighbor and children names, but not `indexing_parent_output_file_name` / `indexing_children_output_file_name`. Multi-placement or `max_cells_per_output_file` runs would overwrite them.
6. **Random points:** (a) the random-points file is created inside the branch for `point_output_type != GDAL_COLLECTION`, so random points are silently skipped when `point_output_type GDAL_COLLECTION`; (b) `randPtsOutFileName` seems to get its suffix twice when `randpts_concatenate_output FALSE` (`SubOpOut.cpp` ~941 and ~1002).
7. **Wrong error text:** the indexing *children* branch reports "indexing parents require a hierarchical indexing system." (`SubOpOut.cpp` ~398).
8. **`input_address_type PLANE` is accepted by the parameter but fails at run time** with "DgRFNetwork::getConverter() frames not connected: IDGGS03plane -> IDGGS03". Either drop PLANE from the input choices or give a clean error (`SubOpIn.cpp` ~93).
9. **SEQNUM with aperture SEQUENCE is only rejected for point input.** The check in `SubOpDGG.cpp` (~134) is commented out; only `SubOpIn::executeOp` (~361) enforces it. GENERATE_GRID with `clip_subset_type COARSE_CELLS`/`ADDRESS_FILES` and `input_address_type SEQNUM` on PLANETRISK ran without error; check whether the seqnum interpretation is valid there.
10. **`clip_subset_type COARSE_CELL_FILES`:** handled in `SubOpGen.cpp` (~206) but absent from the parameter's choice list, so it can never be selected. Dead code or missing choice.
11. **`input_address_field_type`** was accepted but `SubOpIn::setupOp` hard-codes GEO_POINT; the value was never used. (v9.1 working tree: the `insertParam` is now commented out, so the parameter is rejected.)
12. **GENERATE_GRID ignores `input_files` but opens one if `point_input_file_type TEXT`:** setting it opens `vals.txt` and fails if absent.
13. **Version strings:** `DgBase.h` still has `DGGRID_VERSION "9.0b"` and `DGGRID_RELEASE_DATE "April 2, 2026"` at the v9.03b tag; `CHANGELOG.md` says 9.03b, 2026-07-19. Also the default `kml_description` embeds the version.
14. **`precision` has no upper bound** (`DgIntParam(..., 0, INT_MAX)`); `precision 500` did not crash in a quick GENERATE_GRID test, but fixed-size buffers (`maxFmtStr`, `maxBuffSize = 200` in `DgOutRandPtsText.cpp`) have not been checked.
15. **OUTPUT_STATS with `dggs_num_placements` > 1 (or RANDOM orientation):** the statistics table is printed once per placement, and the orientation-metafile write is attempted with an empty file name because `outOp` is inactive for this operation (`SubOpBasicMulti.cpp` ~205). Not run; check whether it silently writes nothing or fails.

## Behaviors that may be surprising (not necessarily bugs)

- `GENERATE_GRID_FROM_POINTS` defaults `cell_output_control` to OUTPUT_ALL (all cells) and `cell_output_type` to NONE, so by default it writes nothing; documented in Section 8.
- `output_address_type` is used only when `output_cell_label_type` is OUTPUT_ADDRESS_TYPE (documented).
- `output_first_seqnum` / `output_last_seqnum` work only for GENERATE_GRID WHOLE_EARTH (documented).
- TRANSFORM_POINTS runs the per-cell output once per input point, so cell, point, neighbor and children outputs contain one record per input point (duplicates when points share a cell).
- `update_frequency` is accepted by every operation but only used by GENERATE_GRID.

---

# Addendum: status in v9.1 and new findings (from the v9.1 manual comparison, 2026-10-07)

Checked on a Release build of branch `v91docs` (`DGGRID_VERSION "9.1"`) with GDAL 3.13.3, macOS arm64, and on a build of the `v9.03b` tag for comparison. Repro files are the ones inlined above unless stated.

## Status of the v9.03b items on v9.1

| # | Item | v9.1 status |
|---|---|---|
| 1 | TRANSFORM_POINTS + plain CSV through GDAL segfaults | **Still present** (exit 139, reproduced) |
| 2 | COARSE_CELLS with space-delimited multi-part address segfaults | **Still present** (exit 139, reproduced) |
| 3 | Multiple placements: later placements bin no points | **Still present** (reproduced: BIN_POINT_VALS, RANDOM, 2 placements, `OUTPUT_OCCUPIED`: `.0001` has 1 line, `.0002` has 0) |
| 4 | ENUMERATION labels every cell 0 outside GENERATE_GRID | **Still present** (reproduced) |
| 5-12, 14, 15 | Likely bugs from reading | Code unchanged in 9.1 (`SubOpOut.cpp`, `SubOpIn.cpp`, `SubOpGen*.cpp` diffs touch only datum plumbing); not re-run |
| 13 | Version strings stale in `DgBase.h` | **Fixed** in 9.1: `DGGRID_VERSION "9.1"`, `DGGRID_RELEASE_DATE "October 10, 2026"` |

## New findings in v9.1

16. **RESOLVED in the v9.1 working tree: `proj_datum` / `proj_datum_radius` now have a deprecation alias** (`DgApParamList::setDeprecatedParam`: accepted with a warning, mapped to `sphere_radius_type` / `custom_sphere_radius`). Original finding: they were removed with no compatibility path. An old metafile now stops with `FATAL ERROR: DgApParamList::setParam() unknown parameter proj_datum`. `WGS84_MEAN_SPHERE` no longer exists; the nearest equivalent is `sphere_radius_type CUSTOM_SPHERE` with `custom_sphere_radius 6371.0087714`. Reproduced. Consider accepting the old names (or at least a clearer message). `CHANGELOG.md` does not mention the removal.

17. **`CHANGELOG.md` `[Unreleased]` is out of date relative to the code.** It says the IVEA presets are `IVEA3H, IVEA4H, IVEA7H, IVEA43H, IVEA4T, IVEA4D`; the code rejects those names ("Invalid parameter data"). Only the suffixed names exist (`IVEA3HS`/`IVEA3HL`, and so on). It does not mention: the `S`/`L` preset families (26 new `dggs_type` values in all, counting `IGEO7v1` and `IGEO7v2`), `IGEO7v1`/`IGEO7v2`, the datum parameters (`sphere_radius_type`, `custom_sphere_radius`, `input_datum`, `output_datum`, `orientation_datum`), `dggs_orient_preset`, removal of `proj_datum`/`proj_datum_radius`, TRANSFORM_POINTS now allowing `output_address_type GEO`, spatial-reference output (shapefile `.prj`, GDAL layer SRS), the orientation/metafile output now being a replayable metafile, the `-v`/`-h` GDAL-version change, or the `invalid` change (item 19). It says "manual updated to v9.1" and "link in README to readthedocs", which is not yet true.

18. **GEO text addresses ignore `precision`.** TRANSFORM_POINTS with `output_address_type GEO` writes the cell centre with six decimals in the text file and in the cell label (`-168.750000,66.049550`) even with `precision 10`; the GeoJSON point geometry honors `precision` (`66.0495496878`). Reproduced. The cause is the geographic RF's `formatStr()` used by `DgEllipsoidRF::add2str`.

19. **A parameter value of `invalid` is no longer silently ignored.** `DgApParamList::setParam` used to return early for any value equal (case-insensitive) to `invalid`; that line was removed in 9.1. `dggs_type invalid` was accepted as CUSTOM in v9.03b and is now `FATAL ERROR: Invalid parameter data`. This is probably intended (the old behavior hid errors) but is a behavior change that is not in the changelog.

20. **`-h` and `-v` disagree about GDAL.** `-v` prints `built with GDAL version 3130300` but `-h` and the run banner now print only `built with GDAL` (the version lines were commented out in `dggrid.cpp`). Also the `-h` branch condition `hFlag || (hFlag && vFlag)` is redundant.

21. **A partially specified orientation pair is reinterpreted when `orientation_datum` is `WGS84`.** If only `dggs_vert0_lon` (or only `dggs_vert0_lat`) is set explicitly, the other component keeps its default/preset value, but the pair is then converted as a geodetic position. Example (reproduced): `dggs_vert0_lon 11.25` plus `orientation_datum WGS84` moves the two northern vertices at longitudes 11.25 and -168.75 to latitudes 58.1677 and 58.3974 instead of both being at 58.2825 (the vertices are no longer related by the preset's symmetry; the geodetic-to-authalic conversion was applied to a latitude that was already authalic). Setting both components, or using a preset, is unaffected. Probably by design, but surprising.

22. **`dggs_orient_preset` is applied even when the orientation is RANDOM or REGION_CENTER.** The preset still sets `dggs_vert0_*` and `orientation_datum`; the vertex values are then overwritten, but `orientation_datum` stays AUTHALIC_SPHERE unless set explicitly. Harmless but easy to misread in the echoed parameter list.

23. **Preset name asymmetry.** The unsuffixed legacy names exist for ISEA (`ISEA3H` is an alias of `ISEA3HS`) but not for IVEA (`IVEA3H` is invalid), and FULLER has no `S`/`L` variants (`FULLER3HL` is invalid). Fine if intended; the changelog and the parked manual text assumed otherwise.

24. **Platform precision.** On this macOS arm64 build `long double` is 64 bits (the banner prints `big double: 64 bits`), so the "long double" exact constants and IVEA kernel are only double precision there. Manual statements about numerical agreement (for example "1e-14 rad with PROJ/DGGAL" in `CHANGELOG.md`) are platform dependent. I could not check the PROJ/DGGAL agreement: the installed PROJ (9.8.1) has no `ivea` projection (added in 9.9) and DGGAL is not installed.

## Documentation inconsistencies found (pre-existing, not code bugs)

25. **Manual Appendix D "PlanetRisk Grid" table and Appendix F Table 1 do not use the stated radius.** Their areas (for example resolution 1: 12,751,646.91 km2) are what the WGS 84 mean sphere gives (6371.0087714 km). With the default authalic radius (the radius Appendix D states it assumes) 9.1 and 9.03b both give 12,751,640.54 km2 for the same grid (reproduced with `OUTPUT_STATS dggs_type PLANETRISK`). The ISEA3H/ISEA4H/ISEA43H tables and Appendix E match the authalic radius.
