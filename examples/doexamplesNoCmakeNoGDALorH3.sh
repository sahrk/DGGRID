#!/bin/bash
#
# doexamplesNoCmakeNoGDALorH3.sh: executes the examples and diff's the output 
#     against sample output, assuming DGGRID was built with the
#     legacy Makefile system without GDAL or H3: only the examples in
#     examplesNoGDALorH3.lst are run
#
# Kevin Sahr, 4/19/21
#
# usage: doexamplesNoCmakeNoGDALorH3.sh [referenceDir]
#   referenceDir defaults to sampleOutput (relative to this directory,
#   or absolute). The dggrid exit status of each example is printed to
#   stdout (not to the example log).
#

#dggridExe=../../build/src/apps/dggrid/dggrid
dggridExe=../../src/apps/dggrid/dggrid
refDir=${1:-sampleOutput}
case "$refDir" in
   /*) refPath="$refDir" ;;
   *)  refPath="../$refDir" ;;
esac
examples=( `cat examplesNoGDALorH3.lst` )

for f in ${examples[@]}
do
   echo "############################################"
   echo "#### running example $f " 
   cd "$f" || { echo "MISSING EXAMPLE $f"; continue; }
   # every metafile in the example's directory whose name starts with the
   # example's name (usually just ${f}.meta)
   for meta in ${f}*.meta
   do
      m=${meta%.meta}
      $dggridExe ${meta} >& outputfiles/${m}.txt
      echo "dggrid exit status $?"
   done
   echo diffs\?
   if [ -d "$refPath/$f" ]; then
      diff -r outputfiles "$refPath/$f"
   else
      echo "no reference output $refPath/$f; diff skipped"
   fi
   echo end diffs for example $f
   cd ..
done
