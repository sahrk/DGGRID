#!/bin/bash
#
# cleanexamples.sh: remove all output from each example
#   (does not change sampleOutput)
#
# Kevin Sahr, 5/20/13
#

examples=( `cat examples.lst` )

for f in ${examples[@]}
do
   echo \*\* cleaning example $f
   rm -f ${f}/outputfiles/*
   touch ${f}/outputfiles/.keep
done
