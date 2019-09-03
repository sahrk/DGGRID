# Change Log
All notable changes to this project will be documented in this file.

All changes are by Kevin Sahr, unless otherwise noted.

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
- added SEQNUMS choice to parameter clip_subset_type (thanks to R. Barnes)
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
- gcc v6 compiler errors (from R. Barnes)
- llvm compile error in DgLocVector.h
### Changed
- released under an AGPL license
- using clipper for polygon intersection (thanks to R. Barnes)
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
- added GeoJSON output format (Matt Gregory)
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
- added KML format for output (Jesse Williamson)
- added ability to break-up large output files by setting
  the max number of cells to write to a single file
- added operation that outputs DGGS characteristics table
- improved feedback during execution
- improved Makefiles
- builds cleanly on MacOS 10.8.2 using gcc 4.2.1 and on 
  Ubuntu 11.04 with gcc 4.5.2 (Jesse Williamson)

## [4.3b] - 2003-06-21
- first public release

## [3.1b] - 2001-11-11

## [0.9b] - 2001-10-01
- initial port of Kevin Sahr's tclib to dglib
- Lian Song wrote many of the spherical trigonometry routines and the 
  original implementation of the ISEA projection.
