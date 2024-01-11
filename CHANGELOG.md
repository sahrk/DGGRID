# Change Log
All notable changes to this project will be documented in this file.

All changes are by Kevin Sahr, unless otherwise noted.

## [8.1b] - 2024-01-10
### Added
- pure aperture 7 hexagon grids (ISEA7H/FULLER7H) are now first class DGGS that support all operations (just like aperture 3 and 4 hexagon DGGS)
### Changed
- removed altitude component from built-in GeoJSON and KML output files

## [8.0b] - 2023-10-24
### Added
- dgaplib glue library for dglib application development
- example multipleOrientations to demonstrate/test creating multiple random grid placements
- new version 8 examples binpresV8, binvalsV8, genPtsV8, and transformV8.
### Changed
- complete re-architecture of DGGRID application to use dgaplib approach
- all operations now use complete cell output infrastructure previously used by GENERATE_GRID.
- added operation GENERATE_GRID_FROM_POINTS
- operations GENERATE_GRID_FROM_POINTS, BIN_POINT_VALS, and BIN_POINT_PRESENCE can use GDAL file input
- see new V8 examples and the manual for information on the new parameters 

## [7.81] - 2023-05-06
### Added
- command-line -v and -h flags

## [7.8] - 2023-04-21
### Fixed
- several compiler warnings
### Added
- cmake switch WITH_EXT_SHAPELIB to build with an external version of the 
shapelib library; see details in INSTALL.md (thanks to @allixender)
- (all changes below are not yet documented in the manual; but see the
new examples listed below)
- added choice OUTPUT_ADDRESS_TYPE to parameter output_cell_label_type
- added new hierarchical index address types ZORDER, ZORDER_STRING, Z3,
and Z3_STRING to parameters input_address_type and output_address_type
- operations BIN_POINT_VALS and BIN_POINT_PRESENCE now output addresses
using output_address_type
- replaced parameter clip_cell_seqnums with clip_cell_addresses, which uses
cell addresses as specified by input_address_type.
- added new examples isea4t, isea4d, z3CellClip, z3Nums, z3WholeEarth,
zCollection, zTransform, z3Collection, z3Transform, zCellClip, and zNums

## [7.72] - 2023-04-19
### Added
- additional debugging statements to help in tracking down some issues
### Changed
- removed DestroyFeature from DgOutGdalFile.cpp

## [7.71] - 2023-03-21
### Added
- added DGGRID dockerfile image (thanks to @allixender)
### Fixed
- clipping with input polygons now works (again) in diamond and triangle grids
### Changed
- updated README.md

## [7.7] - 2022-12-08
### Added
- handles holes in GDAL input clipping polygons. Added new parameter clip_using_holes and example named holes
### Fixed
- replaced deprecated sprintf with snprintf throughout
- suppressed unused parameter compiler warnings
### Changed
- updated manual to version 7.7

## [7.521] - 2022-03-03
### Fixed
- refactor DgHexIDGG class hierarchy to avoid potential memory errors

## [7.52] - 2022-02-19
### Added
- support for dggridR output
### Changed
- minor refactor of macro FALLTHROUGH to suppress clang compiler warning

## [7.51] - 2022-02-12
### Added
- macro FALLTHROUGH to suppress compiler warning on implict switch case fallthrough

## [7.5] - 2022-02-8
### Added
- GDAL collection output, including parameters collection_output_gdal_format
and collection_output_file_name, and GDAL_COLLECTION as a choice for output
type parameters
- example `gdalCollection`
- debugging parameters pause_on_startup and pause_before_exit
### Changed
- updated manual to version 7.5

## [7.41] - 2022-01-19
### Added
- choice COARSE_CELLS to parameter clip_subset_type so grid generation
clipping region can be specified using lower resolution cells
- parameters clip_cell_res, clip_cell_seqnums, and clip_cell_densification
- example gridgenCellClip to demonstrate new parameters
### Changed
- updated manual to version 7.41

## [7.4] - 2022-01-11
### Added
- parameters longitude_wrap_mode and unwrap_points to output cells and points
without longitude wrapping (to facilitate 2D display)
- example planetRiskGridNoWrap to demonstrate new parameters
### Changed
- updated manual to version 7.4

## [7.33] - 2022-01-07
### Changed
- commented-out possible overflow warning messages pending a better implementation
### Added
- DGGRIDR compiler flag to allow for dggridR-specific code. Used in the DgBase::report() method.
### Fixed
- suppress compiler warnings (thanks to r-barnes)

## [7.32] - 2021-10-05
### Changed
- all makeRF methods now return a const pointer

## [7.312] - 2021-09-28
### Added
- modify find_package(GDAL) calls to allow user to toggle on/off (thanks to Crghilardi)
### Fixed
- cmake USE_GDAL flag in dggrid CMakeLists.txt
- initialized instance variables in DgResAdd template

## [7.311] - 2021-09-27
### Fixed
- code improvements (thanks to @r-barnes)
- set C STANDARD target property in shapelib CMakeLists.txt (thanks to Crghilardi)
- incorrect nulib include in DgRF.hpp

## [7.31] - 2021-06-04
### Fixed
- fixed shapelib include search for cmake (thanks to @r-barnes)

## [7.3] - 2021-04-29
### Fixed
- all build warnings
### Added
- cmake build system (thanks to @r-barnes)

## [7.2] - 2021-04-06
### Fixed
- lots of memory leaks
### Added
- doexamplesNoGDAL.sh shell script in examples for users who built DGGRID without GDAL
- added doexamples shell scripts for users who built DGGRID using the legace Makefile build system
### Changed
- all RF's can now only be created using factory methods name makeRF
- reduced precision on several examples so that the output would match on a wider range of platforms

## [7.101] - 2021-03-20
### Fixed
- type conversion compiler warnings
- minor documentation errors
### Changed
- removed unnecessary application source code files from the shapelib directory to avoid Xcode linker errors
- removed main function from shputils.c to avoid Xcode linker errors

## [7.1] - 2021-03-15
### Fixed
- ensure C++ version of streampos is used (thanks to r-barnes)
- outdated tags in Doxyfile
### Added
- BUILD_WITH_GDAL build flag in MakeIncludes to allow building DGGRID without gdal installed
- USE_NUCELL build flag in MakeIncludes to maintain compatibility with Kevin's nuit fork
- WHERE macro in DgBase.h for quick code location output when debugging
- gridgenDiamond example
### Changed
- integrated DgRF infrastructure from Kevin's nuit fork
- minor updates to documentation; note neighbors are broken for triangle grids
- renamed MakeIncludes and Makefile to add a ".noCMake" extension so that they won't conflict with a cmake build system

## [7.07] - 2020-06-24
### Fixed
- invalid gridStats in hexagon DGGS when not created using the factory
method makeRF(); hex DGGS are now all sub-classes of DgHexIDGGS
- fix inappropriately escaped bracket (thanks to r-barnes)
- remove unused instance variable curLayer_ from DgInGDALFile

## [7.06] - 2020-04-10
### Added
- a warning in MakeIncludes about potential gdal building issues

## [7.05] - 2020-01-27
### Fixed
- GDAL output files now correctly repeat the first/last polygon vertex

## [7.04] - 2019-12-21
### Fixed
- added newlines after features in GeoJSON output files and updated examples accordingly
### Changed
- deleted some extraneous files

## [7.03] - 2019-10-01
### Added
- point binning grid generation; currently undocumented

## [7.02] - 2019-09-30
### Fixed
- fixed a bug in dggrid evalCell that caused quad overages to fail
- removed (for now) my failed attempt at optimizing the gnomonic quad bounds code; put back PROJ4

## [7.01] - 2019-08-05
### Added
- added DgInterleaveToQ2DIConverter, though not yet wired-up to handle INTERLEAVE input
### Changed
- refactored quadclip gnomonic bounds check

## [7.0] - 2019-08-01
### Added
- added PLANETRISK choice to parameter dggs_type
- added ISEA7H and FULLER7H choices to parameter dggs_type
- added aperture 7 choice to parameter dggs_aperture
- added SEQUENCE choice to parameter dggs_aperture_type
- added new parameter dggs_aperture_sequence
- added GDAL choice to parameter clip_subset_type
- added GDAL choice to parameters cell_output_type and point_output_type
- added new parameter cell_output_gdal_format
- added new parameter point_output_gdal_format
- added cell neighbor output, including parameters neighbor_output_type and neighbor_output_file_name
- added cell children output, including parameters children_output_type and children_output_file_name
- added parameters output_first_seqnum and output_last_seqnum
- restored INTERLEAVE choice to parameter output_address_type with corrected error messages
### Changed
- released DGGRID under the AGPL license
- gnomonic projection now calculated manually, allowing the removal of the PROJ4 library dependency
- made DgSerialConverter conversion trace code more informative/legible
- updated documentation and examples to reflect all changes

## [6.51] - 2019-07-01
### Changed
- simplified the point-on-icosahedron-face function

## [6.5] - 2019-05-31
### Changed
- changed the rotation angle from backing frame to substrate grid backing frame for a DgHexC2Grid2D from 30.0 to -30.0, to correspond to the definition in the PlanetRisk code fork. Note that cell boundary polygons are generated beginning with a different vertex than in previous versions.
- changed examples/sampleOutput to reflect the rotation angle change
- minor changes to examples output formatting

## [6.42] - 2019-05-03
### Changed
- using shortest distance to face center point to calculate the face a point is on
- all trig functions and sqrt's now use long double versions
- precalculated more constants

## [6.41] - 2019-04-19
### Fixed
- error in coslon limiting in DgEllipsoidRF.cpp (thanks to @billyauhk who
found the error in H3)

## [6.4] - 2018-09-18
### Added
- added parameter clipper_scale_factor
- added SEQNUMS choice to parameter clip_subset_type (thanks to @r-barnes)
- example seqnums
### Fixed
- shapelib header file directory include in MakeIncludes
- overage in I and J intersection bug
### Changed
- added bounding box coarse filter before clipper polygon intersection

## [6.3] - 2018-09-01
### Added
- this file CHANGELOG.md
### Fixed
- gcc v6 compiler errors (thanks to @r-barnes)
- llvm compile error in DgLocVector.h
### Changed
- released under an AGPL license
- using clipper for polygon intersection (thanks to @r-barnes)
- cleaned-up examples
### Deleted
- unused hypot.c from proj4lib

## [6.2b] - 2018-08-31
### Added
- Created private github repo.

## [6.2] - 2015-08-30
- restored v4.3b TRANSFORM_POINTS operation and supporting parameters
- restored v4.3b BIN_POINT_VALS operation and supporting parameters
- restored v4.3b BIN_POINT_PRESENCE operation and supporting parameters
- added GeoJSON output format (thanks to Matt Gregory)
- added parameters kml_name and kml_description
- added new examples
- corrected/suppressed (as applicable) various compiler warnings
- added the metafile editor program MFeditor.jar (available
  separately from www.discreteglobalgrids.org); MFeditor was
  written by Michael Paradis and Benjamin Harris, and updated
  for DGGRID 6.2b by Jeremy Anders and Anthony Serna.

## [6.1] - 2013-05-28
- fixed vertex ordering in ArcInfo Generate file output
  polygons (bug introduced in version 6.02b)
- fixed bug that caused Superfund cells that straddle
  quad boundaries to not be appropriately clipped in some
  situations
- added parameters kml_default_color and kml_default_width

## [6.02b] - 2013-04-07
- corrected reversed winding order on output shapefile polygons
- added dglib code usage demo in src/apps/appex
- completed manual appendices A (DGGRID Metafile Parameters) and C
  (Characteristics of DGGRID Hexagonal DGGs)
- changed output_cell_label_type parameter value "LINEAR_INDEX"
  to the more descriptive value "GLOBAL_SEQUENCE"
- for naming consistency changed parameter concatenate_randpts_output
  to randpts_concatenate_output and num_randpts_per_cell to
  randpts_num_per_cell
- changed DgParamList.h to correctly handle strict c++ two-pass
  template compiling without -fpermissive
- fixed rare crashes caused by abstract classes with missing
  virtual destructors
- numerous small changes to allow or improve compilation on a
  variety of platforms

## [6.0b] - 2013-01-29

- increased precision throughout allowing for higher
  resolution DGGs
- added preset DGGS parameter types
- added mixed aperture 4 and 3 hexagon DGGSs, greatly
  increasing the number of grid choices
- can generate DGGSs using Fuller's icosahedral projection (projection
  was written in R by Denis White and then ported to C++ by James
  Scharmann)
- includes the Superfund_500m hierarchically indexed mixed
  aperture DGGS
- added ESRI Shapefile format for input and output
- added KML format for output (thanks to Jesse Williamson)
- added ability to break-up large output files by setting
  the max number of cells to write to a single file
- added operation that outputs DGGS characteristics table
- improved feedback during execution
- improved Makefiles
- builds cleanly on MacOS 10.8.2 using gcc 4.2.1 and on
  Ubuntu 11.04 with gcc 4.5.2 (thanks to Jesse Williamson)

## [4.3b] - 2003-06-21
- first public release

## [3.1b] - 2001-11-11

## [0.9b] - 2001-10-01
- initial port of Kevin Sahr's tclib to dglib
- Lian Song wrote many of the spherical trigonometry routines and the
  original implementation of the ISEA projection.
