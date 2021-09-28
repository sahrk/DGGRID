#!/bin/bash
#
# doexamplesNoGDAL.sh: executes the examples and diff's the output 
#     against sample output for a cmake build without GDAL
#
# Kevin Sahr, 4/9/13
#

dggridExe=../../build/src/apps/dggrid/dggrid
examples=( `cat examplesNoGDAL.lst` )

for f in ${examples[@]}
do
   echo "############################################"
   echo "#### running example $f " 
   cd $f
   $dggridExe ${f}.meta >& outputfiles/${f}.txt
   echo diffs\?
   diff -r outputfiles ../sampleOutput/$f
   echo end diffs for example $f
   cd ..
done
