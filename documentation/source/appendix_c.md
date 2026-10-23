```{raw} latex
\begin{landscape}
\footnotesize
\setlength{\tabcolsep}{3.5pt}
```

## Appendix C. Default Values for Preset DGG Types

````{container} landscape-appendix

A preset grid type can be specified using the choice parameter dggs_type. All preset grid types share the following default parameter values:

```
dggs_base_poly: ICOSAHEDRON
dggs_orient_specify_type: SPECIFIED
dggs_num_placements: 1
sphere_radius_type: AUTHALIC_SPHERE
dggs_res_specify_type: SPECIFIED
```

The datum and orientation parameters are set by the preset according to its kind. All presets other than the ellipsoidal presets (names ending in `L` or `LZ`), IGEO7v2, IGEO7, H3 and the FULLER presets set:

```
input_datum: AUTHALIC_SPHERE
output_datum: AUTHALIC_SPHERE
dggs_orient_preset: ISEA
```

The ellipsoidal presets (names ending in `L` or `LZ`), IGEO7v2 and IGEO7 set:

```
input_datum: WGS84
output_datum: WGS84
dggs_orient_preset: ISEAL
```

The FULLER presets and H3 (but not SUPERFUND, which uses the ISEA orientation above) set:

```
input_datum: AUTHALIC_SPHERE
output_datum: AUTHALIC_SPHERE
dggs_orient_preset: DYMAXION
```

The orientation preset in turn sets the following (see **Section 4**):

| **dggs_orient_preset** | **dggs_vert0_lon** | **dggs_vert0_lat**              | **dggs_vert0_azimuth** | **orientation_datum** |
|------------------------|--------------------|---------------------------------|------------------------|-----------------------|
| **ISEA**               | 11.25              | 58.282525588538995 (atan φ)     | 0.0                    | AUTHALIC_SPHERE       |
| **ISEAL**              | 11.20              | 58.282525588538995 (atan φ)     | 0.0                    | AUTHALIC_SPHERE       |
| **DYMAXION**           | 10.5361990754676369448 | 64.7000001279348804496    | 125.687650893546277353 | AUTHALIC_SPHERE       |

The table below gives the values of other parameters that are set by each preset DGG type that is not one of the ISEA, IVEA, or FULLER base grids. Note that any preset parameter value can be overridden by explicitly specifying a different value for that parameter in the metafile (the position of the parameter relative to dggs_type does not matter).

| **dggs_type** | **dggs_topology** | **dggs_proj** | **dggs_res_spec** | **dggs_aperture_type** | **dggs_aperture** | **dggs_num_aperture_4_res** | **dggs_aperture_sequence** |
|---------------|-------------------|---------------|-------------------|------------------------|-------------------|-----------------------------|-----------------------------|
| **CUSTOM**    | HEXAGON           | ISEA          | 9                 | PURE                   | 4                 | N/A                         | N/A                         |
| **SUPERFUND** | HEXAGON           | FULLER        | 9                 | MIXED43                | N/A               | 2                           | N/A                         |
| **PLANETRISK**| HEXAGON           | ISEA          | 11                | SEQUENCE               | N/A               | N/A                         | 43334777777777777777777     |
| **H3**        | HEXAGON           | GNOMONIC      | 9 (H3 resolution) | SEQUENCE               | N/A               | N/A                         | 437777777777777777          |
| **IGEO7v1**   | HEXAGON           | ISEA          | 9                 | PURE                   | 7                 | N/A                         | N/A                         |
| **IGEO7v2**   | HEXAGON           | ISEA          | 9                 | PURE                   | 7                 | N/A                         | N/A                         |
| **IGEO7**     | HEXAGON           | ISEA          | 9                 | PURE                   | 7                 | N/A                         | N/A                         |

> **Note:** In addition to the listed parameters, the preset type **SUPERFUND** also sets the value of the parameter `output_cell_label_type` to `SUPERFUND`, and the preset type **H3** sets `h3_boundary_def` to `H3` and `densification` and `clip_cell_densification` to `0`.

The ISEA, IVEA and FULLER presets are formed from the projection and a base grid. The base grid sets the following parameters:

| **Base grid** | **dggs_topology** | **dggs_res_spec** | **dggs_aperture_type** | **dggs_aperture** | **dggs_num_aperture_4_res** | **dggs_aperture_sequence** |
|---------------|-------------------|-------------------|------------------------|-------------------|-----------------------------|-----------------------------|
| **3H**        | HEXAGON           | 9                 | PURE                   | 3                 | N/A                         | N/A                         |
| **4H**        | HEXAGON           | 9                 | PURE                   | 4                 | N/A                         | N/A                         |
| **7H**        | HEXAGON           | 9                 | PURE                   | 7                 | N/A                         | N/A                         |
| **43H**       | HEXAGON           | 9                 | MIXED43                | N/A               | 0                           | N/A                         |
| **4T**        | TRIANGLE          | 9                 | PURE                   | 4                 | N/A                         | N/A                         |
| **4D**        | DIAMOND           | 9                 | PURE                   | 4                 | N/A                         | N/A                         |

The projection and the datum settings (`S` or `L` suffix) then give the preset names in the table below. In each row, *base* stands for any of the six base grids above; in the hierarchically indexed column, *base* stands for 3H, 4H, or 7H only.

| **Projection** | **dggs_proj** | **Spherical preset name** (AUTHALIC_SPHERE input and output, ISEA orientation) | **Ellipsoidal preset name** (WGS84 input and output, ISEAL orientation) | **Hierarchically indexed names** (spherical and ellipsoidal) | **Other names** |
|----------------|---------------|-------------------------------|-------------------------------|-------------------------------|-----------------|
| **ISEA**       | ISEA          | ISEA*base*S, for example ISEA3HS | ISEA*base*L, for example ISEA3HL | ISEA*base*SZ and ISEA*base*LZ, for example ISEA3HSZ and ISEA3HLZ | ISEA*base* (an alias for ISEA*base*S), for example ISEA3H; ISEA7HSZ is also IGEO7v1; ISEA7HLZ is also IGEO7v2 and IGEO7 |
| **IVEA**       | IVEA          | IVEA*base*S, for example IVEA3HS | IVEA*base*L, for example IVEA3HL | IVEA*base*SZ and IVEA*base*LZ, for example IVEA3HSZ and IVEA3HLZ | none |
| **FULLER**     | FULLER        | FULLER*base*, for example FULLER3H | none | none | none |

The FULLER presets use AUTHALIC_SPHERE input and output and the DYMAXION orientation.

The hierarchically indexed presets (names ending in `Z`, and IGEO7, IGEO7v1, IGEO7v2 and H3) set the additional parameters below, where *system* is the hierarchical indexing system for the aperture of the preset (H3 for the H3 preset, which sets shapefile_id_field_length to 15):

```
hier_indexing_system_type: system
input_address_type: HIERNDX
input_hier_ndx_system: system
input_hier_ndx_form: INT64
output_cell_label_type: OUTPUT_ADDRESS_TYPE
output_address_type: HIERNDX
output_hier_ndx_system: system
output_hier_ndx_form: INT64
shapefile_id_field_length: 16
```

| **Base grid** | **system** |
|---------------|------------|
| **3H**        | Z3         |
| **4H**        | ZORDER     |
| **7H**        | Z7         |

IGEO7v1 is the aperture 7 preset ISEA7HSZ, and IGEO7v2 and IGEO7 are ISEA7HLZ, so they use Z7. Because input_address_type is HIERNDX in these presets, a hierarchically indexed preset reads its input as indexes, for example the files of clip_region_files with clip_subset_type ADDRESS_FILES. Operations that read geographic points, such as BIN_POINT_VALS and BIN_POINT_PRESENCE, require input_address_type GEO, which must then be set explicitly. As for any preset, any of these parameters can be overridden by setting it explicitly.

````

```{raw} latex
\end{landscape}
```
