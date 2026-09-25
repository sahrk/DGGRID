#!/bin/bash
#
# doexamples.sh: executes the examples and diff's the output 
#     against sample output
#
# Kevin Sahr, 4/9/13
#
# usage: doexamples.sh [referenceDir]
#   referenceDir defaults to sampleOutput (relative to this directory,
#   or absolute). The dggrid exit status of each example is printed to
#   stdout (not to the example log).
#

dggridExe=../../build/src/apps/dggrid/dggrid
refDir=${1:-sampleOutput}
case "$refDir" in
   /*) refPath="$refDir" ;;
   *)  refPath="../$refDir" ;;
esac
examples=( `cat examples.lst` )

for f in ${examples[@]}
do
   echo "############################################"
   echo "#### running example $f " 
   cd "$f" || { echo "MISSING EXAMPLE $f"; continue; }
   $dggridExe ${f}.meta >& outputfiles/${f}.txt
   echo "dggrid exit status $?"
   echo diffs\?
   if [ -d "$refPath/$f" ]; then
      diff -rwb outputfiles "$refPath/$f"
   else
      echo "no reference output $refPath/$f; diff skipped"
   fi
   echo end diffs for example $f
   cd ..
done
