```{raw} latex
\begin{landscape}
\footnotesize
\setlength{\tabcolsep}{3.5pt}
```

## Appendix D. DGGRID Examples

````{container} landscape-appendix

The table below lists the example metafiles distributed with **DGGRID** in the examples directory. Each example is a directory that contains a metafile of the same name, any input files it needs in subdirectory inputfiles, and a place for its output (subdirectory outputfiles). The output produced by a reference run is in the corresponding sampleOutput directory. The examples are grouped by the value of the **choice** parameter dggrid_operation and listed alphabetically within each group. The "Requires GDAL?" column is "Yes" if the example can only be run by a build of **DGGRID** that includes GDAL. The DGG column gives the value of dggs_type and the Res column the value of dggs_res_spec.

```{table} Examples using GENERATE_GRID
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| aigenerate | No | ISEA3H | 6 | Resolution 6 grid clipped to Oregon plus a 100 mile buffer (Shapefile clip); cells and points in ArcInfo Generate format. |
| determineRes | No | ISEA43H | from cell area | Chooses the resolution from a target cell area (120,000 km^2); whole earth, KML. |
| dymaxionIcosa | No | FULLER4T | 0 | Res 0 triangles (the icosahedron) with the Dymaxion orientation; KML. |
| gdalCollection | Yes | ISEA3H | 2 | Cells, points, neighbors and children for cells listed by SEQNUM, combined in one GeoJSON GDAL collection file. |
| gdalExample | Yes | ISEA7H | 9 | GDAL-readable clip file; cells and points output as GeoJSON via GDAL. |
| gridgenCellClip | No | ISEA3H | 5 | Clips using coarser resolution cells (given by SEQNUM) as the clipping polygons; KML. |
| gridgenDiamond | No | ISEA4D | 3 | Res 3 diamond grid, whole earth; KML. |
| gridgenGeoJSON | No | ISEA43H | 16 | High resolution mixed aperture grid clipped to a small area of Corvallis, Oregon; GeoJSON. |
| gridgenMixedSHP | No | FULLER43H | 10 | Mixed aperture grid with the Fuller projection clipped to Benton County, Oregon (AIGEN clip); Shapefile. |
| gridgenPureKML | No | ISEA3H | 5 | Res 5 aperture 3 hexagon grid, whole earth; KML. |
| hiRes | No | ISEA43H | 27 | Very high resolution grid (cell area about 0.02 m^2) clipped to a small triangle in Ashland, Oregon; Shapefile. |
| holes | Yes | ISEA3H | 17 | Clips with a polygon that has holes (clip_using_holes); KML. |
| icosaISEA | No | ISEA4T | 0 | Res 0 triangles (the icosahedron) with the ISEA orientation; KML. |
| icosaISEAL | No | ISEA4TL | 0 | Res 0 triangles with the ISEAL orientation and WGS84 input/output datums; KML. |
| isea4d | No | ISEA4D | 7 | Res 7 diamond grid clipped to Oregon plus a 100 mile buffer; KML. |
| isea4t | No | ISEA4T | 7 | Res 7 triangle grid clipped to Oregon plus a 100 mile buffer; KML. |
| isea7hGen | No | ISEA7H | 3 | Res 3 aperture 7 hexagon grid, whole earth; KML. |
| ivea3hGen | No | IVEA3HS | 4 | Res 4 IVEA aperture 3 grid, whole earth, with neighbors and children; GeoJSON. |
| ivea7hGen | No | IVEA7HS | 3 | Res 3 IVEA aperture 7 grid, whole earth; KML cells and GeoJSON points. |
| mixedAperture | No | CUSTOM (ISEA hexagons) | 5 | Custom hexagon grid with the mixed aperture sequence 434747, whole earth, with neighbors and children; KML points. |
| multipleOrientations | No | ISEA3H | 2 | Four random orientations of a res 2 grid, whole earth, at most 50 cells per KML file. |
| planetRiskClipHiRes | Yes | PLANETRISK | 19 | Res 19 grid clipped to a polygon over an office building; KML. |
| planetRiskClipMulti | Yes | PLANETRISK | 14 | Res 14 grid clipped to several polygons in the Washington, DC area, with neighbors and children; KML. |
| planetRiskGridGen | Yes | PLANETRISK | 5 | Res 5 grid, whole earth, with neighbors and children; GeoJSON cells and points. |
| planetRiskGridNoWrap | Yes | PLANETRISK | 2 | Res 2 grid, whole earth, with cells and points unwrapped east of the anti-meridian; GeoJSON. |
| quads | No | FULLER4D | 0 | Res 0 diamonds (quadrilaterals) with the Dymaxion orientation; KML. |
| seqnums | No | ISEA7H | 5 | Generates the cells whose SEQNUMs are listed in text files; KML. |
| superfundGrid | No | SUPERFUND | 5 | Res 5 grid clipped to Oregon plus a 100 mile buffer; Shapefile output split into multiple files. |
| wholeEarthIGEO7v1 | No | IGEO7v1 | 3 | Res 3 grid, whole earth, with Z7 indexes; KML. |
| wholeEarthIGEO7v2 | No | IGEO7v2 | 3 | Res 3 ellipsoidal grid, whole earth, with Z7 indexes; KML. |
| z3CellClip | No | ISEA3H | 5 | Clips using coarser resolution cells given as Z3 indexes; Z3 labels; KML. |
| z3Collection | Yes | ISEA3H | 2 | Cells for SEQNUMs in a GDAL collection file, labeled with Z3 indexes; GeoJSON. |
| z3Nums | No | ISEA3H | 9 | Generates the cells whose Z3 indexes are listed in text files; KML. |
| z3WholeEarth | No | ISEA3H | 4 | Res 4 grid, whole earth, labeled with Z3 indexes; KML. |
| z7CellClip | No | ISEA7H | 5 | Clips using coarser resolution cells given as Z7 indexes; also outputs index parents and children; KML. |
| z7Collection | Yes | ISEA7H | 2 | Cells for SEQNUMs in a GDAL collection file, labeled with Z7 indexes, with index parents and children; GeoJSON. |
| z7Nums | No | ISEA7H | 4 | Generates the cells whose Z7 indexes are listed in text files; KML. |
| zCellClip | No | ISEA3H | 5 | Clips using coarser resolution cells given as ZORDER indexes; ZORDER labels; KML. |
| zCollection | Yes | ISEA3H | 2 | Cells for SEQNUMs in a GDAL collection file, labeled with ZORDER indexes; GeoJSON. |
| zNums | No | ISEA3H | 4 | Generates the cells whose ZORDER indexes are listed in text files; KML. |
```

```{table} Examples using GENERATE_GRID_FROM_POINTS
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| genPtsV8 | Yes | ISEA3H | 10 | Outputs the cells containing input points (Oregon airports), with a count per cell; GDAL input, GeoJSON output. |
```

```{table} Examples using BIN_POINT_VALS
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| binvals | No | ISEA3H | 9 | Average population of large Oregon cities per cell; text input and output. |
| binvalsV8 | Yes | ISEA3H | 10 | Count, total and mean length of Oregon bridges per cell; Shapefile input and output via GDAL. |
```

```{table} Examples using BIN_POINT_PRESENCE
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| binpres | No | ISEA3H | 7 | Presence/absence of Oregon cities of various population classes per cell; text input and output. |
| binpresV8 | Yes | ISEA3H | 7 | Presence/absence of Oregon cities of various population classes per cell; GeoJSON input and output via GDAL. |
```

```{table} Examples using TRANSFORM_POINTS
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| transform | No | ISEA3H | 9 | Converts the locations of some Oregon cities to SEQNUM cell addresses; text. |
| transformV8 | Yes | ISEA3H | 9 | Converts the locations of some Oregon cities to SEQNUM cell addresses; text plus GeoJSON GDAL collection output. |
| z3Transform | No | ISEA3H | 9 | Converts SEQNUM addresses to Z3 indexes; text. |
| z7Transform | No | ISEA7H | 9 | Converts SEQNUM addresses to Z7 DIGIT_STRING indexes; text. |
| zTransform | No | ISEA3H | 9 | Converts GEO locations to ZORDER indexes; text. |
```

```{table} Examples using OUTPUT_STATS
:class: longtable
:widths: 18 9 14 8 51
:align: left

| **Name** | **Requires GDAL?** | **DGG** | **Res** | **Description** |
|---|---|---|---|---|
| planetRiskTable | No | PLANETRISK | 20 | Table of grid statistics for resolutions 0-20. |
| table | No | ISEA43H | 15 | Table of grid statistics for resolutions 0-15. |
```

````

```{raw} latex
\end{landscape}
```
