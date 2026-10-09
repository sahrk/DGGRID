```{raw} latex
\begin{landscape}
\footnotesize
\setlength{\tabcolsep}{3.5pt}
```

## Appendix A. DGGRID Metafile Parameters

````{container} landscape-appendix

```{table} DGGRID metafile parameters
:class: longtable
:widths: 16 22 18 12 16 16
:align: left

| **Parameter Name** | **Description** | **Allowed Values** | **Default** | **Notes** | **Used When** |
|-------------------|----------------|-------------------|------------|-----------|---------------|
| **bin_coverage** *(choice)* | Are values distributed over most of the globe or only a relatively small portion? | GLOBAL, PARTIAL | GLOBAL | Allows **DGGRID** to determine how to trade-off speed vs. memory usage | dggrid_operation is GENERATE_GRID_FROM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **cell_output_control** *(choice)* | Designates which cells to output | OUTPUT_ALL, OUTPUT_OCCUPIED | OUTPUT_ALL | OUTPUT_ALL - output all cells, even if no input values were associated with them; OUTPUT_OCCUPIED - output only cells with associated input values | dggrid_operation is BIN_POINT_VALS or BIN_POINT_PRESENCE |
| **cell_output_file_name** *(string)* | Cell boundary output file name prefix | any | "cells" | | cell_output_type is AIGEN, SHAPEFILE, or KML |
| **cell_output_gdal_format** *(string)* | Cell boundary output file format | GDAL-compatible vector file format (see gdal.org) | GeoJSON | | cell_output_type is GDAL |
| **cell_output_type** *(choice)* | Cell boundary output file format | NONE, AIGEN, SHAPEFILE, KML, GEOJSON, GDAL, GDAL_COLLECTION | AIGEN if dggrid_operation is GENERATE_GRID, otherwise NONE | GDAL and GDAL_COLLECTION require a build with GDAL | |
| **children_output_file_name** *(string)* | Spatial children output file name | any | "chld" | | children_output_type is TEXT |
| **children_output_type** *(choice)* | Output cell spatial children? | NONE, TEXT, GDAL_COLLECTION | NONE | |  |
| **clip_cell_addresses** *(string)* | Addresses of coarse clipping cells in input_address_type | addNum1 addNum2 ... addNumN | | Cell resolution given by clip_cell_res | dggrid_operation is GENERATE_GRID and clip_subset_type is COARSE_CELLS |
| **clip_cell_densification** *(integer)* | Number of points-per-edge densification for clipping cell boundaries | 0 ≤ v ≤ 500 | 1 | v of 0 indicates no densification | dggrid_operation is GENERATE_GRID and clip_subset_type is COARSE_CELLS |
| **clip_cell_res** *(integer)* | Resolution of clipping cells | 0 < v < r, where r is the currently specified DGG resolution | 1 | | dggrid_operation is GENERATE_GRID and clip_subset_type is COARSE_CELLS |
| **clip_region_files** *(string)* | Space delimited list of files that specify grid clipping | any | "test.gen" | | dggrid_operation is GENERATE_GRID and clip_subset_type is AIGEN, GDAL, SHAPEFILE, or ADDRESS_FILES |
| **clip_subset_type** *(choice)* | Specifies how portion of DGG to generate will be determined | WHOLE_EARTH, AIGEN, SHAPEFILE, GDAL, ADDRESS_FILES, COARSE_CELLS | WHOLE_EARTH | COARSE_CELLS is only supported for hexagon grids | dggrid_operation is GENERATE_GRID |
| **clip_type** *(choice)* | Method for determining whether a cell is included by a clipping polygon | POLY_INTERSECT | POLY_INTERSECT | | dggrid_operation is GENERATE_GRID |
| **clip_using_holes** *(boolean)* | Handle holes in input polygons? | TRUE, FALSE | FALSE | Requires a build with GDAL | dggrid_operation is GENERATE_GRID and clip_subset_type is GDAL |
| **clipper_scale_factor** *(integer)* | Scale factor of the integer grid used by the polygon intersection library | 1 ≤ v | 1000000 | Increase if clipping gives incorrect results; a larger value may limit the extent of the region that can be clipped | dggrid_operation is GENERATE_GRID |
| **collection_output_file_name** *(string)* | Collection output file name prefix | any | "cells" | See the last paragraph of **Section 5** | |
| **collection_output_gdal_format** *(string)* | Collection output file format | GDAL-compatible vector file format (see gdal.org) | GeoJSON | See the last paragraph of **Section 5** | |
| **custom_sphere_radius** *(double)* | Radius (km) of the CUSTOM_SPHERE datum | 1.0 ≤ v ≤ 10,000.0 | 6371.007180918474 (WGS 84 authalic radius) | One radius per run, used by every parameter set to CUSTOM_SPHERE; see **Section 4.5** | sphere_radius_type, input_datum, output_datum, or orientation_datum is CUSTOM_SPHERE |
| **densification** *(integer)* | Number of points-per-edge densification to use when generating cell boundaries | 0 ≤ v ≤ 500 | 0 | v of 0 indicates no densification | |
| **dggrid_operation** *(choice)* | Specifies the operation to be performed by this run of **DGGRID** | GENERATE_GRID, BIN_POINT_VALS, BIN_POINT_PRESENCE, TRANSFORM_POINTS, OUTPUT_STATS | GENERATE_GRID | | always |
| **dggs_aperture** *(integer)* | Desired DGGS aperture | 3, 4, 7 | 4 | | dggs_aperture_type is PURE |
| **dggs_aperture_sequence** *(string)* | The DGGS aperture sequence | string of 3's, 4's, and 7's in any order | "333333333333" | | dggs_aperture_type is SEQUENCE |
| **dggs_aperture_type** *(choice)* | Is the aperture sequence pure or mixed? | PURE, MIXED43, SEQUENCE | PURE | | dggs_topology is HEXAGON |
| **dggs_base_poly** *(choice)* | Base polyhedron of the DGGS | ICOSAHEDRON | ICOSAHEDRON | ICOSAHEDRON is the only supported value | |
| **dggs_num_aperture_4_res** *(integer)* | Number of aperture 4 resolutions in a mixed aperture sequence | 0 ≤ v ≤ 35 | 0 | | dggs_aperture_type is MIXED43 |
| **dggs_num_placements** *(integer)* | Number of grid placements to use | 1 ≤ v | 1 | If dggs_orient_specify_type is not RANDOM all placements will be the same | |
| **dggs_orient_output_file_name** *(string)* | Name of file for output of multiple DGGS placement parameter values | any | "grid.meta" | Each file is a metafile that can be used as input to reproduce that placement; see **Section 4.1** | dggs_orient_specify_type is RANDOM or dggs_num_placements > 1 |
| **dggs_orient_preset** *(choice)* | Named orientation; sets dggs_vert0_lon, dggs_vert0_lat, dggs_vert0_azimuth and orientation_datum | NONE, ISEA, ISEAL | NONE (ISEA for dggs_type presets, ISEAL for the L presets and IGEO7v2) | Explicitly set parameters take precedence; see **Section 4.1** | dggs_orient_specify_type is SPECIFIED |
| **dggs_orient_rand_seed** *(integer)* | Seed for orientation random number generator | 0 ≤ v | 77316727 | | dggs_orient_specify_type is RANDOM |
| **dggs_orient_specify_type** *(choice)* | How is the DGG orientation specified? | RANDOM, SPECIFIED, REGION_CENTER | SPECIFIED | |  |
| **dggs_proj** *(choice)* | Projection used by the DGGS | ISEA, IVEA, FULLER | ISEA | |  |
| **dggs_res_spec** *(integer)* | Specified DGG resolution | 0 ≤ v ≤ 35 | 9 | If dggs_type is SUPERFUND then 0 ≤ v ≤ 9; if dggs_aperture_type is SEQUENCE then 0 ≤ v ≤ n, where n is the length of dggs_aperture_sequence | dggs_res_specify_type is SPECIFIED |
| **dggs_res_specify_area** *(double)* | Desired cell area | 0.0 < v ≤ 4π(6500)² | 100 | | dggs_res_specify_type is CELL_AREA |
| **dggs_res_specify_intercell_distance** *(double)* | Desired intercell distance (measured on the plane) | 0.0 < v ≤ 2π(6500) | 100 | | dggs_res_specify_type is INTERCELL_DISTANCE |
| **dggs_res_specify_rnd_down** *(boolean)* | Should the desired cell area or intercell distance be rounded down (or up) to the nearest DGGS resolution? | TRUE, FALSE | TRUE | | dggs_res_specify_type is CELL_AREA or INTERCELL_DISTANCE |
| **dggs_res_specify_type** *(choice)* | How is the DGGS resolution specified? | SPECIFIED, CELL_AREA, INTERCELL_DISTANCE | SPECIFIED | |  |
| **dggs_topology** *(choice)* | Desired cell shape | HEXAGON, TRIANGLE, DIAMOND | HEXAGON | |  |
| **dggs_type** *(choice)* | Specify a preset DGG type | CUSTOM, SUPERFUND, PLANETRISK, IGEO7v1, IGEO7v2, IGEO7, ISEA3HS, ISEA4HS, ISEA7HS, ISEA43HS, ISEA4TS, ISEA4DS, ISEA3HL, ISEA4HL, ISEA7HL, ISEA43HL, ISEA4TL, ISEA4DL, IVEA3HS, IVEA4HS, IVEA7HS, IVEA43HS, IVEA4TS, IVEA4DS, IVEA3HL, IVEA4HL, IVEA7HL, IVEA43HL, IVEA4TL, IVEA4DL, ISEA3H, ISEA4H, ISEA7H, ISEA43H, ISEA4T, ISEA4D, FULLER3H, FULLER4H, FULLER7H, FULLER43H, FULLER4T, FULLER4D | CUSTOM | IGEO7 is an alias for IGEO7v1; ISEA3H … ISEA4D are aliases for ISEA3HS … ISEA4DS; S presets use the sphere, L presets use WGS84 input and output; see **Section 4** and [**Appendix C**](#appendix-c-default-values-for-preset-dgg-types) for preset parameter value details | |
| **dggs_vert0_azimuth** *(double)* | Azimuth from icosahedron vertex 0 to vertex 1 (degrees) | 0.0 ≤ v ≤ 360.0 | 0 | | dggs_orient_specify_type is SPECIFIED |
| **dggs_vert0_lat** *(double)* | Latitude of icosahedron vertex 0 (degrees) | -90.0 ≤ v ≤ 90.0 | 58.282525588538995 (atan φ) | | dggs_orient_specify_type is SPECIFIED |
| **dggs_vert0_lon** *(double)* | Longitude of icosahedron vertex 0 (degrees) | -180.0 ≤ v ≤ 180.0 | 11.25 | | dggs_orient_specify_type is SPECIFIED |
| **geodetic_densify** *(double)* | Maximum degrees of arc for a clipping polygon line segment | 0.0 ≤ v ≤ 360.0 | 0 | 0.0 indicates no densification | dggrid_operation is GENERATE_GRID |
| **hier_indexing_system_type** *(choice)* | Hierarchical indexing system used for indexing parents/children | ZORDER, Z3, Z7, NONE | NONE | See [**Appendix D**](#appendix-d-dgg-address-forms) | |
| **indexing_children_output_file_name** *(string)* | Hierarchical indexing children output file name | any | "ndxChld" | | indexing_children_output_type is TEXT |
| **indexing_children_output_type** *(choice)* | Output cell hierarchical indexing children? | NONE, TEXT, GDAL_COLLECTION | NONE | |  |
| **indexing_parent_output_file_name** *(string)* | Hierarchical indexing parents output file name | any | "ndxPrt" | | indexing_parent_output_type is TEXT |
| **indexing_parent_output_type** *(choice)* | Output cell hierarchical indexing parent? | NONE, TEXT, GDAL_COLLECTION | NONE | |  |
| **input_address_type** *(choice)* | Cell address form in input file(s) | GEO, Q2DI, SEQNUM, Q2DD, PROJTRI, VERTEX2DD, HIERNDX | GEO | See [**Appendix D**](#appendix-d-dgg-address-forms); SEQNUM is not allowed if dggs_aperture_type is SEQUENCE; BIN_POINT_VALS and BIN_POINT_PRESENCE require GEO | dggrid_operation is TRANSFORM_POINTS, or dggrid_operation is GENERATE_GRID and clip_subset_type is COARSE_CELLS or ADDRESS_FILES |
| **input_datum** *(choice)* | Datum of geographic input | WGS84, AUTHALIC_SPHERE, CUSTOM_SPHERE | AUTHALIC_SPHERE (WGS84 for the L presets and IGEO7v2) | WGS84 requires sphere_radius_type AUTHALIC_SPHERE; see **Section 4.6** | Geographic input files, clipping files, and GEO addresses |
| **input_delimiter** *(string)* | Character that delimits address components and additional data in the input files | v is any single character in double quotes | " " (a single space) | | dggrid_operation is GENERATE_GRID_FROM_POINTS, TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE; or clip_subset_type is COARSE_CELLS or ADDRESS_FILES |
| **input_file_name** *(string)* | Name of file containing input points or addresses | fileName | valsin.txt | Overrides input_files if set | dggrid_operation is GENERATE_GRID_FROM_POINTS, TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **input_files** *(string)* | Name(s) of files containing lon/lat locations with associated values | fileName1 fileName2 ... fileNameN | vals.txt | | dggrid_operation is GENERATE_GRID_FROM_POINTS, TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **input_hier_ndx_form** *(choice)* | Index representation used in input file(s) | INT64, DIGIT_STRING | INT64 | See [**Appendix D**](#appendix-d-dgg-address-forms) | input_address_type is HIERNDX |
| **input_hier_ndx_system** *(choice)* | Hierarchical indexing system used in input file(s) | ZORDER, Z3, Z7 | Z3 | See [**Appendix D**](#appendix-d-dgg-address-forms) | input_address_type is HIERNDX |
| **input_value_field_name** *(string)* | Field name containing value to bin | | value | | dggrid_operation is BIN_POINT_VALS |
| **kml_default_color** *(string)* | Color of cell boundaries in KML output | any valid KML color | ffffffff | | cell_output_type is KML |
| **kml_default_width** *(integer)* | Width of cell boundaries in KML output | 1 ≤ v ≤ 100 | 4 | | cell_output_type is KML |
| **kml_description** *(string)* | Description tag value in KML output file | | Generated by DGGRID *version* | | cell_output_type is KML |
| **kml_name** *(string)* | Name tag value in KML output file | | "" (empty) | If empty, the output file name (with extension) is used | cell_output_type is KML |
| **longitude_wrap_mode** *(choice)* | How handle vertex longitude for cells that straddle the anti-meridian? | WRAP, UNWRAP_EAST, UNWRAP_WEST | WRAP | |  |
| **max_cells_per_output_file** *(integer)* | Maximum number of cells output to a single output file | 0 ≤ v | 0 | 0 indicates no maximum | |
| **neighbor_output_file_name** *(string)* | Neighbors output file name | any | "nbr" | Triangle grids not supported | neighbor_output_type is TEXT |
| **neighbor_output_type** *(choice)* | Output cell neighbors? | NONE, TEXT, GDAL_COLLECTION | NONE | Triangle grids not supported | |
| **orientation_datum** *(choice)* | Datum of user supplied orientation positions | WGS84, AUTHALIC_SPHERE, CUSTOM_SPHERE | AUTHALIC_SPHERE | WGS84 requires sphere_radius_type AUTHALIC_SPHERE; see **Section 4.6** | dggs_vert0_lon, dggs_vert0_lat, region_center_lon, or region_center_lat is set |
| **output_address_type** *(choice)* | Address form to use in output | GEO, Q2DI, SEQNUM, PLANE, Q2DD, PROJTRI, VERTEX2DD, HIERNDX | SEQNUM | See [**Appendix D**](#appendix-d-dgg-address-forms); only has an effect when output_cell_label_type is OUTPUT_ADDRESS_TYPE | dggrid_operation is TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE, or output_cell_label_type is OUTPUT_ADDRESS_TYPE |
| **output_cell_label_type** *(choice)* | Output form for generated cell indexes | GLOBAL_SEQUENCE, ENUMERATION, OUTPUT_ADDRESS_TYPE, SUPERFUND | GLOBAL_SEQUENCE | Default is OUTPUT_ADDRESS_TYPE if dggrid_operation is TRANSFORM_POINTS or dggs_type is IGEO7, IGEO7v1, or IGEO7v2; SUPERFUND (required) if dggs_type is SUPERFUND | |
| **output_count** *(boolean)* | Output the count of input points contained in each cell? | TRUE, FALSE | FALSE | | dggrid_operation is GENERATE_GRID_FROM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **output_count_field_name** *(string)* | Field name containing count of contained points | | count | | dggrid_operation is GENERATE_GRID_FROM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **output_datum** *(choice)* | Datum of geographic output, including output metadata | WGS84, AUTHALIC_SPHERE, CUSTOM_SPHERE | AUTHALIC_SPHERE (WGS84 for the L presets and IGEO7v2) | WGS84 requires sphere_radius_type AUTHALIC_SPHERE; see **Section 4.6** | GEO addresses and geographic geometry |
| **output_delimiter** *(string)* | Character that delimits address components and additional data in the output file | v is any single character in double quotes | " " (a single space) | | dggrid_operation is TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **output_file_name** *(string)* | Name of file to use for output | | valsout.txt | | dggrid_operation is TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **output_file_type** *(choice)* | Operation-specific text output? | NONE, TEXT | TEXT | | dggrid_operation is TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **output_first_seqnum** *(integer)* | Begin generating with this cell ID | 1 ≤ v | 1 | Cell numbering starts at 1; ignored if dggs_type is SUPERFUND | dggrid_operation is GENERATE_GRID and clip_subset_type is WHOLE_EARTH |
| **output_hier_ndx_form** *(choice)* | Index representation used in output file(s) | INT64, DIGIT_STRING | INT64 | See [**Appendix D**](#appendix-d-dgg-address-forms) | output_address_type is HIERNDX |
| **output_hier_ndx_system** *(choice)* | Hierarchical indexing system used in output file(s) | ZORDER, Z3, Z7 | Z3 | See [**Appendix D**](#appendix-d-dgg-address-forms) | output_address_type is HIERNDX |
| **output_last_seqnum** *(integer)* | Last cell ID to generate | 1 ≤ v | ULONG_MAX (no limit) | Ignored if dggs_type is SUPERFUND | dggrid_operation is GENERATE_GRID and clip_subset_type is WHOLE_EARTH |
| **output_mean** *(boolean)* | Output the mean of contained point values? | TRUE, FALSE | TRUE | | dggrid_operation is BIN_POINT_VALS |
| **output_mean_field_name** *(string)* | Field name containing mean of contained point values | | mean | | dggrid_operation is BIN_POINT_VALS |
| **output_num_classes** *(boolean)* | Output the number of classes present in cell? | TRUE, FALSE | FALSE | | dggrid_operation is BIN_POINT_PRESENCE |
| **output_num_classes_field_name** *(string)* | Field name containing the number of classes present in cell | | numClass | | dggrid_operation is BIN_POINT_PRESENCE |
| **output_presence_vector** *(boolean)* | Output the cell presence vector? | TRUE, FALSE | TRUE | | dggrid_operation is BIN_POINT_PRESENCE |
| **output_presence_vector_field_name** *(string)* | Field name containing the cell presence vector | | presVec | | dggrid_operation is BIN_POINT_PRESENCE |
| **output_total** *(boolean)* | Output the total of contained point values? | TRUE, FALSE | FALSE | | dggrid_operation is BIN_POINT_VALS |
| **output_total_field_name** *(string)* | Field name containing total of contained point values | | total | | dggrid_operation is BIN_POINT_VALS |
| **pause_before_exit** *(boolean)* | Pause program execution before exiting | TRUE, FALSE | FALSE | |  |
| **pause_on_startup** *(boolean)* | Pause program execution at program start | TRUE, FALSE | FALSE | |  |
| **point_input_file_type** *(choice)* | Point input file type | NONE, TEXT, GDAL | NONE if dggrid_operation is GENERATE_GRID, otherwise TEXT | GDAL requires a build with GDAL | dggrid_operation is GENERATE_GRID_FROM_POINTS, TRANSFORM_POINTS, BIN_POINT_VALS, or BIN_POINT_PRESENCE |
| **point_output_file_name** *(string)* | Cell point output file name prefix | any | "centers" | | point_output_type is AIGEN, SHAPEFILE, KML, or TEXT |
| **point_output_gdal_format** *(string)* | Point output file format | GDAL-compatible vector file format (see gdal.org) | GeoJSON | | point_output_type is GDAL |
| **point_output_type** *(choice)* | Cell point output file format | NONE, AIGEN, KML, SHAPEFILE, TEXT, GEOJSON, GDAL, GDAL_COLLECTION | NONE | |  |
| **precision** *(integer)* | Number of digits to right of decimal point when outputting floating point numbers | 0 ≤ v | 7 | |  |
| **proj_datum** *(choice)* | **Deprecated**; use sphere_radius_type | WGS84_AUTHALIC_SPHERE, WGS84_MEAN_SPHERE, CUSTOM_SPHERE | | Still accepted with a warning, and will be removed in a future version. WGS84_AUTHALIC_SPHERE and CUSTOM_SPHERE map to sphere_radius_type AUTHALIC_SPHERE and CUSTOM_SPHERE; WGS84_MEAN_SPHERE maps to CUSTOM_SPHERE with radius 6371.00877141506 km | |
| **proj_datum_radius** *(double)* | **Deprecated**; use custom_sphere_radius | 1.0 ≤ v ≤ 10,000.0 | | Still accepted with a warning, and will be removed in a future version | proj_datum is CUSTOM_SPHERE |
| **randpts_concatenate_output** *(boolean)* | Put random points for multiple DGG placements in a single file? | TRUE, FALSE | TRUE | | randpts_output_type is AIGEN, KML, SHAPEFILE, or TEXT |
| **randpts_num_per_cell** *(integer)* | Number of random points to generate per cell | 0 ≤ v | 0 | | randpts_output_type is AIGEN, KML, SHAPEFILE, or TEXT |
| **randpts_output_file_name** *(string)* | Random points-in-cell output file name prefix | any | "randPts" | | randpts_output_type is AIGEN, KML, SHAPEFILE, or TEXT and randpts_num_per_cell > 0 |
| **randpts_output_type** *(choice)* | Random points-in-cell output file format | NONE, AIGEN, KML, SHAPEFILE, TEXT, GEOJSON | NONE | |  |
| **randpts_seed** *(integer)* | Seed for cell points random number generator | 0 ≤ v | 77316727 | | randpts_output_type is not NONE and randpts_num_per_cell > 0 |
| **region_center_lat** *(double)* | Latitude of study region (degrees) | -90.0 ≤ v ≤ 90.0 | 0 | | dggs_orient_specify_type is REGION_CENTER |
| **region_center_lon** *(double)* | Longitude of study region (degrees) | -180.0 ≤ v ≤ 180.0 | 0 | | dggs_orient_specify_type is REGION_CENTER |
| **rng_type** *(choice)* | Specifies the random number generator to use | RAND, MOTHER | RAND | RAND: C standard library rand; MOTHER: George Marsaglia's multiply-with-carry "Mother" function | |
| **shapefile_id_field_length** *(integer)* | Number of digits in Shapefile output cell index strings | 1 ≤ v ≤ 50 | 11 | | cell_output_type, point_output_type, or randpts_output_type is SHAPEFILE |
| **sphere_radius_type** *(choice)* | Sphere the grid is built on | AUTHALIC_SPHERE, CUSTOM_SPHERE | AUTHALIC_SPHERE | AUTHALIC_SPHERE is the sphere with the WGS 84 authalic radius; see **Section 4.5** | |
| **unwrap_points** *(boolean)* | Output cell center points unwrapped to follow their unwrapped cell? | TRUE, FALSE | TRUE | Only has an effect when longitude_wrap_mode is UNWRAP_EAST or UNWRAP_WEST | |
| **update_frequency** *(integer)* | Number of cell inclusion tests to perform between outputting status updates | 0 ≤ v | 100000 | | dggrid_operation is GENERATE_GRID |
| **verbosity** *(integer)* | Amount of debugging output to display | 0 ≤ v ≤ 3 | 0 | |  |
| **z3_invalid_digit** *(choice)* | Padding digit for unused resolutions in Z3 INT64 indexes | 0, 1, 2, 3 | 3 | See [**Appendix D**](#appendix-d-dgg-address-forms) | |

```

````

```{raw} latex
\end{landscape}
```
