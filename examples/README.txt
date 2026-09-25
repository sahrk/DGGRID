DGGRID Examples
===============

Kevin Sahr, August 30, 2015
www.discreteglobalgrids.org

The subdirectories in this examples directory each
contain an example DGGRID metafile and associated
input files. See the comments in each metafile for 
details. The subdirectory sampleOutput contains output 
created by DGGRID for each of the examples.

Assuming dggrid is in your command search path, each 
example can be run from within the example's
subdirectory using:

  dggrid theFileName.meta

The two bash shell scripts below are provided for running the 
examples. We recommend running cleanexamples.sh (see below)
before each run of these scripts to avoid spurious error
messages.

  doexamples.sh - executes all examples and performs a
    diff of the output files with output that has been
    verified. Assumes that dggrid is in it's original
    build location. Note that rounding errors may result 
    in small differences in the output generated, 
    depending on your system.

    doexamples.sh takes an optional argument, the reference
    directory to diff against (default sampleOutput; relative
    to this directory or absolute). Examples with no reference
    output are run but not diffed. After each example it prints
    "dggrid exit status N" to its standard output (not to the
    example's log file).

  doexamplesNoGDAL.sh - works exactly like doexamples.sh
    above except it does not execute the examples that
    use GDAL input or output files. Use this if you
    built DGGRID without GDAL.

The above assume that you built DGGRID using cmake. If
you built DGGRID using the legacy Makefiles, then use
doexamplesNoCMake.sh and doexamplesNoCMakeNoGDAL.sh
resectively.

The following bash shells scripts are used to help 
maintain the examples directory:

  cleanexamples.sh - removes output files from all
    examples directories, returning the examples to 
    their original state (directory sampleOutput is left
    unchanged)

  copyexamples.sh - copy the outputs generated in the
    examples directories by the last run of doexamples.sh 
    into sampleOutput. An optional argument names a different
    destination directory; <dest>/<example> is created if it
    is missing.

The following Python 3 tools (standard library only; they use
ogrinfo for shapefiles and xmllint for KML if available) check
the output of the last run of doexamples.sh:

  checkexamples.py - validates each example's output on its own
    terms, without any reference: the log ends normally with no
    error lines, dggrid exited with status 0, every output file
    named in the metafile exists and is non-empty, KML/GeoJSON/
    shapefile/text syntax, no nan/inf, lon/lat in range, closed
    rings, whole-earth cell counts, 12 pentagons, neighbour and
    children references, and cell areas of whole-earth hexagon
    grids (equal area). Prints PASS/FAIL per example; exits
    non-zero on any FAIL. Pass the saved standard output of
    doexamples.sh to check the exit status:

      ./doexamples.sh > run.log 2>&1
      python3 checkexamples.py --run-log run.log

  cmpexamples.py - compares the output with a reference tree
    (--ref DIR, default sampleOutput), classing every file as
    IDENTICAL, ROUNDOFF (only floats differ, each by at most one
    unit in the last printed place) or STRUCTURAL (anything else),
    and examples without reference output as NOREF. The version
    and type-size lines at the top of the logs are ignored.
    --expect-change ABS_TOL widens the float tolerance to ABS_TOL;
    --allow FILE accepts listed STRUCTURAL differences, each with
    a written justification. Prints a one-line summary; exits
    non-zero on any STRUCTURAL difference that is not allowed.

Run either tool with -h for details.
