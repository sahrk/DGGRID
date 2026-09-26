#!/bin/bash
#
# copyexamples.sh: replace the sampleOutput output files with the
#      outputs from the current runs of the examples
#
# Kevin Sahr, 10/20/14
#
# usage: copyexamples.sh [destinationDir]
#   destinationDir defaults to sampleOutput; <destinationDir>/<example>
#   is created if it is missing.
#

destDir=${1:-sampleOutput}
examples=( `cat examples.lst` )

for f in ${examples[@]}
do
   echo \*\* copy example $f
   mkdir -p "${destDir}/${f}"
   rm -rf "${destDir}/${f}"/*
   cp -r ${f}/outputfiles/* "${destDir}/${f}"
   touch "${destDir}/${f}/.keep"
done
