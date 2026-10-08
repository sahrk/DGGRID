# Plan: updating the DGGRID manual from v9.03b to v9.1

Written 2026-10-07 on branch `v91docs`. This is a plan only. No manual file has been changed. When executed, every edit is confined to `documentation/source/` (`dggrid_man_V91.md`, `appendix_a.md`, `_static/custom.css`, `conf.py`) unless Kevin gives explicit permission for anything else. Code issues found along the way are recorded in `documentation/V903_CODE_BUGS_PLAN.md` (addendum at the end of that file); nothing in the code is fixed.

## 1. What was compared and how

- **Baseline:** tag `v9.03b` (identical to `origin/master` for `src/`; local `master` is a stale older commit and was not used). The manual in `v91docs` is byte-identical to `origin/master`'s `dggrid_man_V903.md`.
- **v9.1 code:** `v91docs` HEAD (`DGGRID_VERSION "9.1"`, `DGGRID_RELEASE_DATE "October 10, 2026"`). The `src/` diff touches 55 files; I read all the parts that affect user-visible behavior (`SubOpDGG`, `SubOpIn`, `SubOpOut`, `SubOpTransform`, `SubOpGenHelper`, `SubOpBasicMulti`, `dggrid.cpp`, `DgApParamList`, `DgConstants`, `DgWGS84RF`, `DgAuthalicConverter`, `DgOut*File`, `DgIDGGutil`, projection files).
- **Parameters:** I extracted every `insertParam` name on both trees. The only differences are **added** `custom_sphere_radius`, `dggs_orient_preset`, `input_datum`, `orientation_datum`, `output_datum`, `sphere_radius_type` and **removed** `proj_datum`, `proj_datum_radius`. (Source-only names remaining from v9.03b were `dggs_base_poly`, `input_address_field_type`, `bin_method`, `input_address_field_name` and `point_input_gdal_format`. The last three were only in commented-out code; `input_address_field_type` is now commented out too, and `dggs_base_poly` is documented in Appendix A.) Defaults and choice lists changed only for `dggs_type`, `dggs_proj`, `dggs_vert0_lat`.
- **Verification:** I built the v9.1 binary and the v9.03b binary (GDAL 3.13.3, macOS arm64, in the scratchpad, not in the repo) and ran the cases cited below. In this plan **[V]** means I ran it on the 9.1 binary, **[C]** means read from the code only, **[U]** means I could not verify it.

## 2. What changed in 9.1 that the manual must reflect

1. **IVEA projection** (`dggs_proj IVEA`); IVEA equal-area construction (slice-and-dice), same planar cells/addresses as ISEA. [V] IVEA4HS gives the same cell numbering as ISEA4HS with different coordinates; the 272 cells of a whole-earth IVEA3HS resolution-3 grid have total area 510,065,621.72 km² = 4πR², and their hexagon areas (1,889,266 to 1,889,368 km² as measured from 5-point-densified boundaries) agree with the nominal ISEA3H value 1,889,131.93 km² to about 7e-5.
2. **Datum system replaces `proj_datum`.** `sphere_radius_type`, `custom_sphere_radius`, `input_datum`, `output_datum`, `orientation_datum`, each `WGS84 | AUTHALIC_SPHERE | CUSTOM_SPHERE`. `proj_datum` / `proj_datum_radius` / `WGS84_MEAN_SPHERE` are gone: an old metafile fails with "unknown parameter proj_datum" [V]. `WGS84` for a boundary datum requires `sphere_radius_type` WGS84 or AUTHALIC_SPHERE, otherwise a fatal error [V]. `WGS84` as `sphere_radius_type` means the WGS 84 authalic sphere (radius 6371.0071809… km) [V].
3. **Orientation:** `dggs_orient_preset NONE|ISEA|ISEAL`; default `dggs_vert0_lat` is now atan(φ) = 58.28252558853899… [V]; ISEAL is vertex 0 at lon 11.20, same latitude [V]. Explicit parameters override the preset, `NONE` can override a preset-selected orientation, and the value is case-insensitive [V]. `orientation_datum WGS84` converts only user-set vert0 / region-center values, never preset or built-in ones [V].
4. **Presets:** the `dggs_type` list grows from 16 to 42 values [V, all 42 names accepted by the binary; `IVEA3H`, `IVEA43H`, `FULLER3HL` rejected]:
   - `CUSTOM, SUPERFUND, PLANETRISK` (unchanged);
   - `IGEO7v1` (= old `IGEO7`: ISEA7HS + Z7), `IGEO7v2` (= IVEA7HL + Z7), `IGEO7` (alias of `IGEO7v1`);
   - `ISEA{3H,4H,7H,43H,4T,4D}S`, `IVEA{...}S` : "S" = spherical (input/output datum AUTHALIC_SPHERE, `dggs_orient_preset ISEA`);
   - `ISEA{...}L`, `IVEA{...}L` : "L" = ellipsoidal (input/output datum WGS84, `dggs_orient_preset ISEAL`);
   - legacy `ISEA3H … ISEA4D` = aliases of the `S` presets; **no** unsuffixed IVEA names; `FULLER*` and SUPERFUND/PLANETRISK have no S/L variants.
   - All presets use `sphere_radius_type AUTHALIC_SPHERE`; `orientation_datum` is AUTHALIC_SPHERE even for the L presets (the preset orientation is already on the grid sphere).
5. **Exact constants:** icosahedron/ISEA constants and WGS 84 radii are computed exactly. The authalic radius is 6371.00718091847389797… km (printed 6371.007180918474; v9.03b used …475). The WGS 84 mean radius (6371.00877141506) is no longer a selectable datum.
6. **Output:** shapefile `.prj` and GDAL-written layers now carry a spatial reference: WGS84 output → EPSG:4326 (`GCS_WGS_1984` in `.prj`, `urn:ogc:def:crs:OGC:1.3:CRS84` in GDAL GeoJSON); spherical output → a named sphere (`AuthalicSphereWGS84radius` or `CustomSphere`, radius in the `.prj`) [V, all of shapefile/GPKG/GeoJSON-via-GDAL]. The non-GDAL GeoJSON and KML writers add no CRS, spherical GDAL GeoJSON adds none [V].
7. **TRANSFORM_POINTS now allows `output_address_type GEO`** (the cell centre, in `output_datum`). v9.03b stopped with "output address type must be non-GEO"; 9.1 writes it [V both]. Works for SEQNUM→GEO and GEO→GEO, with trailing text preserved [V].
8. **Orientation/metafile output:** the file named by `dggs_orient_output_file_name` is now a valid metafile (no "(user set)" annotations), written with 17+ significant digits, listing every used parameter, with `orientation_datum` echoed and vertex latitude written in that datum. Feeding it back as a metafile reproduces the grid exactly [V: byte-identical GeoJSON with RANDOM orientation and `orientation_datum WGS84`].
9. **Command line:** `-h` and the run banner now say `built with GDAL` with no version; `-v` still prints `built with GDAL version 3130300` [V]. The manual currently says both print the version.
10. **Bug fixes that need no manual text** (ISEA/Fuller values change at ≤1e-8°; the Class III vertex fix; sector reduction; atan2l). I found no sentence in the manual they contradict. Appendix D/E/F statistics are unchanged at the printed precision except Appendix D/F PlanetRisk (item 25 in the bug file, a v9.03b problem).
11. **New examples** (`ivea3hGen`, `ivea7hGen`, `icosaISEA`, `icosaISEAL`, `wholeEarthIGEO7v1`, `wholeEarthIGEO7v2`) — the manual only points to the examples folder, so no text is required.

## 3. Corrections needed to the parked text (`V91_MANUAL_PLAN.md`)

The parked text is a good starting draft but must not be restored verbatim:

1. **IVEA preset names are wrong.** Its Appendix A / Appendix B / Section 4 lists use `IVEA3H, IVEA4H, IVEA7H, IVEA43H, IVEA4T, IVEA4D`; the code rejects them [V]. They are `IVEA…S` and `IVEA…L`. (`CHANGELOG.md` has the same error; reported in the bug file, item 17.)
2. **"Every dggs_type preset other than CUSTOM selects dggs_orient_preset ISEA"** is wrong for the `L` presets and `IGEO7v2`, which select ISEAL [V].
3. **Stale vocabulary:** "In SPHERE output mode", "according to the corresponding geographic mode" (Appendix C sentence). There is no such mode in 9.1; reword in terms of `input_datum` / `output_datum`.
4. **"matches PROJ ivea and DGGAL to ~1e-14 rad"** [U]. I could not check: the installed PROJ (9.8.1) has no `ivea`, DGGAL is not installed, and on this machine `long double` is 64 bits. Needs Kevin's decision (Decision D2).
5. **Radius printed to 50 digits** (6371.0071809184738979763378457319610626905184106258). The source constant holds 48 digits and the exact value is defined by a and 1/f. Use "6371.007180918474 km (computed exactly from the WGS 84 defining parameters)".
6. **Release line** says October 15, 2026; the code (`DgBase.h`) and root `README.md` say **October 10, 2026**.
7. **Formatting convention:** the draft backticks parameter names (`orientation_datum`, `output_datum`); the manual's convention is plain text for parameter names and backticks for values.
8. **"see Section 1 above"** in the slice-and-dice paragraph is ambiguous; it means item 1 of Section 4 (use "**Subsection 1**", as Section 4 already says "**Subsection 5**").
9. **Appendix B shared defaults** must also list the datum parameters and `dggs_orient_preset` (see Step 12); the draft only changed one line.
10. The draft does not cover: S/L presets, IGEO7v1/v2, `dggs_orient_preset` Appendix B semantics, TRANSFORM_POINTS GEO output, the `-v`/`-h` text, the metafile-replay description of the orientation file, the `proj_datum` migration, third-party credits. These are added below.

## 4. Decisions (answered by Kevin, 2026-10-07)

- **D1. Release date:** "9.1", October 10, 2026. Applied in Steps 1 and 2.
- **D2. PROJ/DGGAL agreement:** drop the numeric claim; say only that the vector form of Recht (2021) is used (Step 7).
- **D3. `proj_datum` migration note:** superseded: a code alias was added (deprecated, warns, maps to the new parameters) and the manual documents the deprecation in Section 4 item 5 and Appendix A. Originally: deferred; stays on the open list. Step 8 will not add the note, and Appendix A simply deletes the two rows. The removal is recorded in the bug file (item 16).
- **D4. Appendix B:** group the names. Layout: (a) one table with a row for CUSTOM, SUPERFUND, PLANETRISK, IGEO7v1, IGEO7v2, IGEO7 and for each base grid 3H, 4H, 7H, 43H, 4T, 4D (topology, aperture type, aperture, num aperture 4, sequence, resolution); (b) a "name construction" table with a row per projection (ISEA, IVEA, FULLER) giving `dggs_proj`, the names formed from each base grid with suffix S, suffix L, and no suffix (ISEA only legacy alias; FULLER has no suffix), and the datum/orientation settings of S versus L. If this proves unclear when drafted I will fall back to one row per name (42 rows).
- **D5. PlanetRisk statistics:** add a footnote stating the radius used (WGS 84 mean radius 6371.0087714 km) to the Appendix D PlanetRisk table and Appendix F Table 1; numbers unchanged (Step 14).
- **D6. Credits:** add the PROJ/DGGAL/A5 bullet (Step 2).
- **D7. Appendix D and IVEA:** put both projections in the table headings (for example "dggs_type ISEA3H / IVEA3H"). Caveat to settle when I get there: the number-of-cells and hex-area columns are identical for IVEA (verified), but the intercell-spacing columns were measured empirically on ISEA only. **Final:** share the heading and add a note that the spacing columns are ISEA values; IVEA spacing is not measured for now.
- **D8. `conf.py` copyright year:** change to 2026 (Step 1). `examples._rst` is left alone.

## 5. Steps

Steps are ordered so each leaves the manual building. After each step I will report what I changed and what I verified, as you asked.

### Step 1. Title, release line, CSS id (`dggrid_man_V91.md` L1, L80; `_static/custom.css`)

- L1: `# User Documentation for DGGRID v9.03b` → `v9.1`.
- L80: `**DGGRID** version 9.03b was released July 19, 2026` → `version 9.1 was released October 10, 2026` (per D1).
- `custom.css`: the selector `section#user-documentation-for-dggrid-v9-03b > h1` must follow the new title id. I will read the actual id from a build of `_build/html/dggrid_man_V91.html` (expected `user-documentation-for-dggrid-v9-1`) and update the selector, otherwise the centered title is lost on the site.
- `conf.py`: copyright year (D8).
- Evidence: HTML page `<title>` already reads "DGGRID 9.1" from `CMakeLists.txt`.

### Step 2. Credits (L56-73)

- Add the PROJ/DGGAL/A5 bullet to the third-party list (D6), and the Recht (2021) / van Leeuwen–Strebe (2006) acknowledgment is handled in Appendix G (Step 14).

### Step 3. Section 1: command-line flags (L137)

- Current text says `-v` prints "whether DGGRID was built with GDAL (and, if so, the GDAL version)" and `-h` "prints that information along with brief usage and license information". Change to: `-v` prints version, release date, and whether built with GDAL and the GDAL version number; `-h` prints version, release date, whether built with GDAL (no version number), brief usage and license information. [V]
- Section 1 also still describes the grid as "icosahedral discrete global grids" — no change.

### Step 4. Section 2: metafile format and the parameter table (L141-179)

- No new rules apply. Re-verify the acceptance table: all six new parameters are registered by `SubOpDGG`, so they are accepted by every operation (including OUTPUT_STATS) [V: `input_datum`/`custom_sphere_radius` accepted by OUTPUT_STATS], so they belong in the "DGG specification" row, which already covers them.
- Add one sentence under the table that the datum parameters of Section 4 are accepted by every operation (the WGS84-needs-authalic check is applied even for OUTPUT_STATS [V]); no new table row is needed.
- Note for Kevin: the "invalid value" change (bug file, item 19) is not a documented behavior; I will not add it to the manual.

### Step 5. Section 4 Background and Preset DGG Types (L206-242)

- L208-215: add IVEA to the supported-projection sentence (parked wording, with "Icosahedral Vertex-oriented great-circle Equal Area" and the van Leeuwen and Strebe [2006] citation).
- L217: "...a discussion on specifying the spherical earth radius" → "...and the datums and the sphere on which the grid is built".
- L219-238: replace the preset list. New structure (names verified [V]):
  - `CUSTOM`, `SUPERFUND`, `PLANETRISK` unchanged;
  - `IGEO7v1`, `IGEO7v2`, and `IGEO7` (alias of `IGEO7v1`);
  - a short explanation of the **S** (spherical) and **L** (ellipsoidal) suffix rule, then lists of the ISEA, IVEA preset bases (`3H, 4H, 7H, 43H, 4T, 4D`), with the unsuffixed ISEA names described as aliases of the S names and a statement that IVEA and FULLER have no unsuffixed names;
  - FULLER presets (no S/L).
- L240: the sentence on override precedence stays. Add that the presets set the datum and orientation parameters (Appendix B) and the L presets set `input_datum`/`output_datum` WGS84 and the ISEAL orientation.
- L242 "Appendix D gives some statistics on … ISEA presets": add the IVEA sentence if D7.

### Step 6. Section 4 item 1, orientation (L248-262)

- L254: default code block `dggs_vert0_lat 58.28252559` → `58.282525588538995`.
- Add the golden-ratio paragraph (parked text is correct [V]: poles fall on icosahedron edge midpoints; earlier versions used 58.28252559).
- Add the `dggs_orient_preset` paragraph, corrected: values NONE (default), ISEA, ISEAL; sets `dggs_vert0_lon` (11.25 / 11.20), `dggs_vert0_lat` (atan φ), `dggs_vert0_azimuth` 0 and `orientation_datum` AUTHALIC_SPHERE [V]; explicit parameters take precedence [V]; CUSTOM defaults to NONE, every other preset except the L presets and `IGEO7v2` selects ISEA, and the L presets and `IGEO7v2` select ISEAL [V]; ISEAL is the orientation PROJ and DGGAL use for ellipsoidal ISEA/IVEA and keeps vertex 0 in the ocean on WGS 84 (source comment [C], keep the claim modest).
- L258 (RANDOM paragraph): add the replay paragraph. Corrections to the parked wording: the file lists *all* parameters used in the run (not only the orientation), is a plain metafile, sets `orientation_datum`, and vertex latitude is written in that datum; relative input files must remain available [V].
- Add one paragraph describing `orientation_datum` here (or a forward reference to item 6): it applies only to *user-supplied* `dggs_vert0_lon/lat` and `region_center_lon/lat` and, if one component of a pair is set, the pair is interpreted in that datum [V]; presets and RANDOM are not converted [V]. (The surprising partial-pair case is bug-file item 21.)

### Step 7. Section 4 item 3, projection (L274)

- Parked sentence with IVEA in the `dggs_proj` list; adjust "equal area cells" to ISEA **and IVEA**.
- Add the slice-and-dice paragraph with these changes: "see Subsection 1 above"; drop the PROJ/DGGAL sentence unless D2 says otherwise; keep the Recht (2021) vector-form sentence; replace "in long double precision" with "in extended precision where the platform provides it" (on this arm64 machine `long double` is 64 bits [V]); keep "IVEA grids have the same planar cell geometry, cell addresses, neighbors, children and hierarchical indices as the ISEA grids" [V].
- State that IVEA, like ISEA, is a spherical projection; the datum parameters decide how longitude/latitude relate to the grid sphere.

### Step 8. Section 4 items 5 and 6, datums (L290-296) — the main rewrite

Replace baseline item 5 ("Specifying the earth radius", `proj_datum`) with the parked items 5 and 6, corrected:

- **Item 5, datums and the grid sphere.** Three datums: `WGS84`, `AUTHALIC_SPHERE` (radius 6371.007180918474 km), `CUSTOM_SPHERE` (`custom_sphere_radius`, range 1.0–10,000.0 km, default the authalic radius, one custom radius per run). `sphere_radius_type` (default AUTHALIC_SPHERE; WGS84 selects the authalic sphere) [V]. Keep the baseline sentence that the radius is not used in generating geometries, only in `dggs_res_specify_type` resolution selection and km statistics (still true: grid geometry is generated on a unit sphere) [C]. If D3 is accepted, add the `proj_datum` migration note here.
- **Item 6, input/output/orientation datums.** Parked paragraphs are accurate [V] after these edits: remove "SPHERE output mode"; avoid backticks on parameter names; keep the WGS84-requires-authalic-sphere rule and its fatal error [V]; keep "authalic conversion preserves area, so ISEA/IVEA stay equal area on the ellipsoid, Fuller does not" [C]; keep the statement that clipping and densification operate on the grid sphere [C]; keep CRS notes but correct them to what I observed: WGS84 → EPSG:4326 (shapefile `.prj` `GCS_WGS_1984`, GDAL layers EPSG:4326 / CRS84 in GDAL GeoJSON); spherical → named sphere with the radius in the `.prj` / GDAL layer; non-GDAL GeoJSON, KML, text and AIGEN carry no CRS [V].
- Cross references: Section 12 says "Section 4.5" (radius) — still correct with the new item 5; Section 5 will refer to "Section 4.6".

### Step 9. Section 5, per-cell output (L300-372)

- L302: "given in geodetic (longitude/latitude) coordinates in decimal degrees" → add "Their datum is selected by output_datum (see **Section 4.6**)".
- L337: "…or dggs_type is IGEO7" → "…or dggs_type is IGEO7, IGEO7v1, or IGEO7v2". [C: all three set OUTPUT_ADDRESS_TYPE]
- L331 (ENUMERATION caveat "for other operations every cell is labeled 0"): **keep**; bug still present in 9.1 [V].
- L344-346: no change. Optional one-sentence pointer that shapefile `.prj` and GDAL layers now declare the datum (Step 8).
- L372 random points: add "in output_datum coordinates" [C: `genRandPts` now converts via the output frame].

### Step 10. Sections 6, 7, 9, 10, 11, 12 (coordinates and TRANSFORM_POINTS)

- L392 (clip files), L430 (point files), L444, L464: "geodetic (latitude/longitude) coordinates" → add "interpreted in the datum given by input_datum (see **Section 4.6**)" [C: `DgInAIGenFile/DgInShapefile/DgInGdalFile` get `inputGeoRF/inputDeg`].
- L484: delete "The output_address_type GEO is not allowed for this operation." and replace with: GEO is allowed; the output is the longitude/latitude of the cell centre, in output_datum, and an input_address_type GEO input is therefore snapped to its cell centre [V]. Mention that GEO text addresses are written with six decimals regardless of precision? That is bug-file item 18; I will mention it to Kevin and keep it out of the manual.
- L488: the two-step "transform to GEO then to another DGG" sentence becomes directly valid [V]; keep it.
- L492 (Section 12): "radius of the grid sphere (see Section 4.5)" stays.
- L494: no change (`precision` default 7).

### Step 11. Appendix A (`appendix_a.md`)

Row-by-row (ASCII alphabetical order is preserved; new rows follow the table's six-column format):

- **Add** (corrected from the parked rows; allowed values / defaults verified [V]):
  - `custom_sphere_radius` *(double)*: 1.0 ≤ v ≤ 10,000.0; default 6371.007180918474 (WGS 84 authalic radius); used when `sphere_radius_type` or a datum is CUSTOM_SPHERE. After `collection_output_gdal_format`.
  - `dggs_orient_preset` *(choice)*: NONE, ISEA, ISEAL; default NONE (ISEA/ISEAL for dggs_type presets); explicitly set parameters take precedence. After `dggs_orient_output_file_name`.
  - `input_datum`, `output_datum`, `orientation_datum` *(choice)*: WGS84, AUTHALIC_SPHERE, CUSTOM_SPHERE; default AUTHALIC_SPHERE (preset: L presets use WGS84 for input/output). Notes: WGS84 requires `sphere_radius_type` WGS84 or AUTHALIC_SPHERE; see Section 4.6. Positions: `input_datum` after `input_address_type`, `orientation_datum` after `neighbor_output_type`, `output_datum` after `output_count_field_name`.
  - `sphere_radius_type` *(choice)*: WGS84, AUTHALIC_SPHERE, CUSTOM_SPHERE; default AUTHALIC_SPHERE. After `shapefile_id_field_length`.
- **Delete:** `proj_datum`, `proj_datum_radius` (L103-104) (plus the migration note if D3).
- **Change:**
  - `dggs_proj` (L45): ISEA, IVEA, FULLER.
  - `dggs_type` (L52): the 42 values (Step 5 list).
  - `dggs_vert0_lat` (L54): default 58.282525588538995 (atan φ).
  - `dggs_orient_output_file_name` (L42): add note "written as a metafile that can be used as input to reproduce the placement".
  - `output_address_type` (L77): remove "GEO is not allowed if dggrid_operation is TRANSFORM_POINTS".
  - `output_cell_label_type` (L78): "dggs_type is IGEO7" → IGEO7, IGEO7v1 or IGEO7v2.
  - `dggs_vert0_lon` (L55): no change (default 11.25; ISEAL 11.20 is described in the preset).
- Re-check the full row/parameter set again at the end (script comparing Appendix A names with `insertParam` names; expected remaining diff: the five long-standing source-only names).
- **Unicode for PDF:** the table already uses `≤ ≥ π`; new text must stay within `≤ ≥ φ π √` or add `\DeclareUnicodeCharacter` lines in `conf.py` (`°` is OK in pdflatex; confirm at build).

### Step 12. Appendix B (L499-547)

- Shared defaults block (L503-511): replace with the list valid for every preset: `dggs_base_poly ICOSAHEDRON`, `dggs_orient_specify_type SPECIFIED`, `dggs_num_placements 1`, `sphere_radius_type AUTHALIC_SPHERE`, `dggs_res_specify_type SPECIFIED`, and the orientation/datum settings by suffix: S, unsuffixed, FULLER, SUPERFUND, PLANETRISK, IGEO7v1 → `dggs_orient_preset ISEA` (`dggs_vert0_lon 11.25`, `dggs_vert0_lat 58.282525588538995`, `dggs_vert0_azimuth 0.0`, `orientation_datum AUTHALIC_SPHERE`), `input_datum` / `output_datum` AUTHALIC_SPHERE; L and IGEO7v2 → `dggs_orient_preset ISEAL` (`dggs_vert0_lon 11.20`, same latitude and azimuth, `orientation_datum AUTHALIC_SPHERE`), `input_datum` / `output_datum` WGS84. [V, whole table checked by running all 42 names]
- Table (L515-532): per D4. Rows to add: IGEO7v1, IGEO7v2 (IVEA, aperture 7, + Z7 settings), the IVEA family, the S/L families; keep the existing columns (`dggs_topology … dggs_aperture_sequence`) since those are identical across suffixes. Resolution default 9 everywhere except PLANETRISK 11 (table currently shows 11 — correct [C]).
- Z7 block (L536-547): "The preset type IGEO7 sets…" → applies to IGEO7, IGEO7v1 and IGEO7v2.

### Step 13. Appendix C (L549-589)

- L555: replace the baseline sentence with the datum-aware form (parked sentence, reworded to say the latitude is on the sphere or the WGS 84 ellipsoid according to input_datum / output_datum).
- `PLANE` and `SEQNUM` notes unchanged (bug-file items 8 and 9 still apply).
- Add under `GEO`: output allowed for TRANSFORM_POINTS (cell centre).

### Step 14. Appendices D, E, F, G

- Appendix D intro (L597): radius text → "6,371.007180918474 km (WGS 84 authalic sphere radius)"; ISEA table numbers do not change [V: OUTPUT_STATS ISEA4H res 1-3, ISEA43H and Appendix E reproduce]. Apply D5 and D7.
- Appendix E (L834): "authalic WGS84 radius (NAD 83 datum)" unchanged; Superfund still uses the authalic sphere in 9.1 [V].
- Appendix G: add Recht (2021) and van Leeuwen and Strebe (2006) (parked wording is correct). The list is sorted by first author's surname: Recht goes between Kimerling and Sahr; van Leeuwen goes between Snyder and White.

### Step 15. Build and verify

- `DGGRID_SKIP_DOXYGEN=1 make html` (full build once at the end) with `BUILDDIR` in the scratchpad; check: no new warnings (expected: none beyond the baseline), the id of the title `<h1>`, internal anchors (`#appendix-a-dggrid-metafile-parameters`), the front page centering, Section 4.5/4.6 references resolve in the text.
- `make latexpdf LATEXMKOPTS="-interaction=nonstopmode"` then `grep -n "^!" dggrid.log`; confirm Appendix A pages are `rot 90` (`pdfinfo -f 1 -l 70 … | grep rot`).
- Script check: every `input_datum`-style name used in the narrative exists in Appendix A and in `insertParam`; every preset name in Section 4 / Appendix A / Appendix B is accepted by the binary (reuse the 42-name loop).
- Re-run the bug repros that the manual text relies on: ENUMERATION caveat (still true), COARSE_CELLS delimiter workaround (still documented).

### Step 16. Report to Kevin

- List of changed files (all under `documentation/source/`), the untracked/uncommitted files that must be committed (`_static/custom.css` if changed, `documentation/Makefile`, `documentation/README.md`, `documentation/V91_*.md`, `V903_*.md` as he sees fit), and the open items below. I will not commit or push.

## 6. Observations outside `documentation/source/` (for Kevin, I will not touch them)

- Root `README.md` line 28 still points to `dggridManualV90b.pdf` for local docs; the third-party list there should get the PROJ/DGGAL bullet; the online-manual slug `v91docs` is a branch-name guess.
- `CHANGELOG.md` `[Unreleased]` needs the corrections listed in bug-file item 17 before the release, including the wrong IVEA preset names.
- The 9.1 code bugs and behaviors recorded in `V903_CODE_BUGS_PLAN.md` (items 16-25) and the v9.03b items that still reproduce on 9.1 (items 1-4).
- `documentation/V91_MANUAL_PLAN.md` and `V91_HANDOFF.md` can be archived once the manual is updated.
