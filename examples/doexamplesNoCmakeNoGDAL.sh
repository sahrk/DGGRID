#!/bin/bash
#
# doexamplesNoCmake.sh: executes the examples and diff's the output 
#     against sample output, assuming DGGRID was built with the
#     legacy Makefile system
#
# Kevin Sahr, 4/19/21
#

#dggridExe=../../build/src/apps/dggrid/dggrid
dggridExe=../../src/apps/dggrid/dggrid
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
