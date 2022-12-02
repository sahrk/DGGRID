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

Two bash shell scripts are provided for running the 
examples: 

  doexamples.sh - executes all examples and performs a
    diff of the output files with output that has been
    verified. Assumes that dggrid is in it's original
    build location. Note that rounding errors may result 
    in small differences in the output generated, 
    depending on your system.

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
    into sampleOutput
