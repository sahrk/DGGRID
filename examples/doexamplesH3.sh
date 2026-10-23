#!/bin/bash
#
# doexamplesH3.sh: executes the examples that require DGGRID to be built with
#     the H3 library and diff's the output against sample output. Like
#     doexamples.sh, but for the examples in examplesH3.lst; every metafile
#     in an example's directory whose name starts with the example's name
#     is run (e.g. h3Gen.meta, h3GenString.meta and h3GenCustom.meta).
#
# usage: doexamplesH3.sh [referenceDir]
#   referenceDir defaults to sampleOutput (relative to this directory,
#   or absolute). The dggrid exit status of each metafile is printed to
#   stdout (not to the example log). The examples are skipped if
#   "dggrid -v" does not report that dggrid was built with H3.
#   Set the environment variable DGGRID to use a different dggrid executable.
#

dggridExe=${DGGRID:-../../build/src/apps/dggrid/dggrid}
refDir=${1:-sampleOutput}
case "$refDir" in
   /*) refPath="$refDir" ;;
   *)  refPath="../$refDir" ;;
esac
# dggridExe is relative to the example directories
firstExample=`head -1 examplesH3.lst`
if ! ( cd "$firstExample" && $dggridExe -v 2>&1 | grep -q "built with H3" ); then
   echo "dggrid was built without H3 (or was not found); skipping the H3 examples"
   exit 0
fi

examples=( `cat examplesH3.lst` )

for f in ${examples[@]}
do
   echo "############################################"
   echo "#### running example $f "
   cd "$f" || { echo "MISSING EXAMPLE $f"; continue; }
   for meta in ${f}*.meta
   do
      m=${meta%.meta}
      $dggridExe ${meta} >& outputfiles/${m}.txt
      echo "dggrid exit status $? for $meta"
   done
   echo diffs\?
   if [ -d "$refPath/$f" ]; then
      diff -rwb outputfiles "$refPath/$f"
   else
      echo "no reference output $refPath/$f; diff skipped"
   fi
   echo end diffs for example $f
   cd ..
done

# the explicitly specified H3 grid must give the same cells as dggs_type H3
# (not the same boundaries: h3_boundary_def H3 requires dggs_type H3)
echo "############################################"
echo "#### h3Gen: dggs_type H3 vs explicitly specified parameters"
for ext in ndxChd ndxPrt; do
   cmp h3Gen/outputfiles/h3res3.$ext h3Gen/outputfiles/h3res3custom.$ext
done
cmp h3Gen/outputfiles/h3res3p.txt h3Gen/outputfiles/h3res3customp.txt
