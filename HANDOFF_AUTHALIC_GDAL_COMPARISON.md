# DGGRID Authalic Latitude and GDAL ISEA Handoff

## Current state

The WGS 84 authalic latitude feature and its direct GDAL ISEA comparison are implemented in the working tree. No commit has been made. Both final builds and CTest runs completed successfully after the comparison probe was added:

- GDAL enabled: `/tmp/dggrid-authalic-gdal`; 3/3 CTest tests passed.
- GDAL disabled: `/tmp/dggrid-authalic-nogdal`; 3/3 CTest tests passed.
- `git diff --check` passed before the final builds.

## What the comparison does

DGGRID's `TRANSFORM_POINTS` operation snaps input to cell addresses, so it cannot measure the continuous ISEA projection. `tests/reference/dggrid_isea_probe.cpp` calls DGGRID's `DgProjISEA` converters directly and accepts:

```text
dggrid_isea_probe forward SPHERE|WGS84 lon lat
dggrid_isea_probe inverse SPHERE|WGS84 face x y
```

DGGRID returns face-local coordinates on a dimensionless unit-edge triangle. GDAL outputs coordinates in meters on an unfolded net. `tests/reference/compare_dggrid_gdal_isea.py` fits each face's rotation and translation from two interior anchors and reports differences at five separate points on each of the 20 faces. The fitted scale, about 7,674,457.948 meters per unit edge, agrees with the equal-area triangular edge scale implied by the authalic sphere within 0.001 meters.

## Measured GDAL results

Runtime versions: GDAL 3.12.2 and PROJ 9.7.1. The full output is in `/tmp/dggrid-gdal-isea-comparison.json`.

| Comparison over 100 face-interior points | Maximum difference |
|---|---:|
| DGGRID spherical ISEA vs GDAL spherical ISEA, forward | 0.00083 m |
| DGGRID spherical ISEA vs GDAL spherical ISEA, inverse | 0.00102 m |
| DGGRID WGS 84 authalic ISEA vs GDAL spherical ISEA fed authalic latitude, forward | 0.00083 m |
| DGGRID WGS 84 authalic ISEA vs GDAL spherical ISEA fed authalic latitude, inverse | 0.00102 m |
| DGGRID WGS 84 authalic ISEA vs GDAL ellipsoidal ISEA, forward | 15,838 m |
| DGGRID WGS 84 authalic ISEA vs GDAL ellipsoidal ISEA, inverse | 25,114 m |
| GDAL ellipsoidal ISEA's own forward/inverse round trip | 25,586 m |

The ellipsoidal GDAL result is not a sound correctness oracle on this installed runtime. `gdaltransform` independently reproduced the self-round-trip error for `(30°, 20°)`: its inverse returned about `(30.03304897°, 20.05156932°)`. One tested point also failed inverse projection as outside the projection domain. The spherical GDAL implementation, with independently calculated authalic latitude, agrees with DGGRID to about a millimeter. Do not change DGGRID's authalic conversion to chase the installed GDAL ellipsoidal result.

PROJ 9.7.1 also silently ignores `+orient_lon` and `+orient_lat` for ISEA. The scripts test and report this. Current PROJ documentation describes these orientation options, but they are not effective in the installed version. See [PROJ ISEA documentation](https://proj.org/en/stable/operations/projections/isea.html).

## Reproduce the GDAL comparison

From the repository root:

```sh
cmake -S . -B /tmp/dggrid-authalic-gdal -DWITH_GDAL=ON
cmake --build /tmp/dggrid-authalic-gdal --target dggrid_isea_probe
python3 tests/reference/compare_gdal_isea.py > /tmp/gdal-isea-raw.json
python3 tests/reference/compare_dggrid_gdal_isea.py \
  /tmp/dggrid-authalic-gdal/dggrid_isea_probe \
  > /tmp/dggrid-gdal-isea-comparison.json
```

The first script calls GDAL's Python `osgeo.osr.CoordinateTransformation` API for forward and inverse transforms for spherical and WGS 84 ISEA. The second compares the continuous DGGRID converter against those GDAL transformations. Python GDAL bindings are required for these reference scripts; ordinary DGGRID builds do not need them.

## Main implementation files

- `src/lib/dglib/lib/DgAuthalicConverter.cpp`, `src/lib/dglib/include/dglib/DgAuthalicConverter.h`: stable forward and inverse authalic latitude math and graph converters.
- `src/lib/dglib/include/dglib/DgWGS84RF.h`, `src/lib/dglib/lib/DgWGS84RF.cpp`: WGS 84 ellipsoid frame and canonical historical authalic radius.
- `src/apps/dggrid/SubOpDGG.cpp` and `.h`: independent input/output modes, validation, WGS 84 frame registration, and orientation handling.
- `src/apps/dggrid/SubOpIn.cpp`, `SubOpGenHelper.cpp`, `SubOpOut.cpp`, `SubOpTransform.cpp`: geographic boundary routing for inputs, clipping, outputs, and GEO transforms.
- `src/lib/dglib/lib/DgOutGdalFile.cpp`, `DgOutShapefile.cpp`, `DgOutGeoJSONFile.cpp`, `DgOutKMLfile.cpp` and corresponding headers: geographic writer frames and CRS metadata.
- `src/lib/dgaplib/include/dgaplib/DgApParamList.h`, `src/lib/dgaplib/lib/DgApParamList.cpp`, `src/apps/dggrid/SubOpBasicMulti.cpp`: parseable generated orientation metafiles with numeric precision sufficient for exact replay.
- `tests/core/test_authalic_converter.cpp`, `tests/integration/test_geographic_modes.py`, `tests/output/test_geographic_output.py`: numerical, mode-matrix, clipping, area, orientation replay, and writer checks.
- `documentation/source/dggrid_man_V841.md`: user-facing parameter and CRS documentation.

## Test commands

```sh
cmake -S . -B /tmp/dggrid-authalic-gdal -DWITH_GDAL=ON
cmake --build /tmp/dggrid-authalic-gdal -j4
ctest --test-dir /tmp/dggrid-authalic-gdal --output-on-failure

cmake -S . -B /tmp/dggrid-authalic-nogdal -DWITH_GDAL=OFF
cmake --build /tmp/dggrid-authalic-nogdal -j4
ctest --test-dir /tmp/dggrid-authalic-nogdal --output-on-failure
```

Both CTest invocations passed all three tests after the latest CMake target and reference scripts were added.
