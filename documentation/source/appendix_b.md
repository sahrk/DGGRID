```{raw} latex
\begin{landscape}
\footnotesize
\setlength{\tabcolsep}{3.5pt}
```

## Appendix B. Parameters Used by Each Operation

````{container} landscape-appendix

The table below shows which parameters have an effect under each operation (**choice** parameter dggrid_operation); a blank means that the parameter has no effect for that operation. A blank parameter is either rejected for that operation with an "unknown parameter" error or accepted and ignored. In particular, OUTPUT_STATS accepts only the general parameters and the parameters that specify the DGG, and its table of statistics depends only on the grid structure, the resolution and the sphere radius; the projection, orientation, datum and indexing parameters do not change its output.

```{table} Parameters used by each operation
:class: longtable
:widths: 22 11 19 14 18 15 10
:align: left

| **Parameters** | **GENERATE_GRID** | **GENERATE_GRID_FROM_POINTS** | **BIN_POINT_VALS** | **BIN_POINT_PRESENCE** | **TRANSFORM_POINTS** | **OUTPUT_STATS** |
|---|---|---|---|---|---|---|
| General parameters (**Section 3**): dggrid_operation, precision, verbosity, pause_on_startup, pause_before_exit | x | x | x | x | x | x |
| update_frequency | x |  |  |  |  |  |
| Grid structure, resolution and sphere (**Section 4**): dggs_type, dggs_base_poly, dggs_topology, dggs_aperture\*, dggs_num_aperture_4_res, dggs_res_\*, sphere_radius_type, custom_sphere_radius, and the deprecated proj_datum, proj_datum_radius | x | x | x | x | x | x |
| Projection, orientation, datums and indexing (**Section 4**): dggs_proj, dggs_orient_specify_type, dggs_orient_rand_seed, dggs_orient_preset, dggs_vert0_\*, region_center_\*, dggs_num_placements, rng_type, orientation_datum, output_datum, hier_indexing_system_type, z3_invalid_digit | x | x | x | x | x |  |
| input_datum | x (a) | x | x | x | x (b) |  |
| clip_\*, clipper_scale_factor, geodetic_densify | x |  |  |  |  |  |
| Point input files: input_files, input_file_name, point_input_file_type |  | x | x | x | x |  |
| Input address form: input_address_type, input_hier_ndx_system, input_hier_ndx_form | x (c) |  |  |  | x |  |
| input_delimiter | x (c) | x | x | x | x |  |
| bin_coverage, output_count, output_count_field_name, cell_output_control |  | x | x | x |  |  |
| input_value_field_name, output_total\*, output_mean\* |  |  | x |  |  |  |
| output_presence_vector\*, output_num_classes\* |  |  |  | x |  |  |
| output_first_seqnum, output_last_seqnum | x (d) |  |  |  |  |  |
| Text file output and cell labels (**Section 5**): output_file_name, output_file_type, output_cell_label_type, output_address_type, output_hier_ndx_\*, output_delimiter | x | x | x | x | x |  |
| Per-cell output (**Section 5**): cell_output_type, cell_output_file_name, cell_output_gdal_format, point_output_\*, collection_output_\*, neighbor_\*, children_\*, indexing_\*, randpts_\*, kml_\*, shapefile_id_field_length, densification, longitude_wrap_mode, unwrap_points, max_cells_per_output_file, and dggs_orient_output_file_name (see (e)) | x | x | x | x | x |  |

```

Notes:

(a) For GENERATE_GRID, input_datum applies to clipping files, that is, clip_subset_type values AIGEN, SHAPEFILE, and GDAL.

(b) For TRANSFORM_POINTS, input_datum applies only when input_address_type is GEO.

(c) For GENERATE_GRID, the input address parameters and input_delimiter apply only to clip_subset_type values ADDRESS_FILES, COARSE_CELLS, and COARSE_CELL_FILES, where they describe the cell addresses. For GENERATE_GRID_FROM_POINTS, BIN_POINT_VALS, and BIN_POINT_PRESENCE the input address type must be GEO, so the input address form parameters have no effect.

(d) Only for clip_subset_type WHOLE_EARTH.

(e) The file named by dggs_orient_output_file_name is written only when dggs_orient_specify_type is RANDOM or dggs_num_placements is greater than 1. Some per-cell output parameters apply only to particular output types or cell label types; see **Section 5** and [**Appendix A**](#appendix-a-dggrid-metafile-parameters).

````

```{raw} latex
\end{landscape}
```
