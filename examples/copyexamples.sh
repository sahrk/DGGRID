#!/bin/bash
#
# copyexamples.sh: replace the sampleOutput output files with the
#      outputs from the current runs of the examples
#
# Kevin Sahr, 10/20/14
#

examples=( `cat examples.lst` )

for f in ${examples[@]}
do
   echo \*\* copy example $f
   rm -f sampleOutput/${f}/*
   cp ${f}/outputfiles/* sampleOutput/${f}
   touch sampleOutput/${f}/.keep
done
