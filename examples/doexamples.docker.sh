#!/bin/bash
#
# doexamples.sh: executes the examples and diff's the output 
#     against sample output
#
# Kevin Sahr, 4/9/13
# Alexander Kmoch, May, 4th, 2023
#
# IMG=dggrid:latest
IMG=dggrid:7.8


examples=( `cat examples.lst` )

for f in ${examples[@]}
do
   echo "############################################"
   echo "#### running example $f " 
   cd $f
   # needs exact local dir
   dggridExe="docker run -it -v $PWD:/data -w /data -u $(id -u):$(id -g) --rm $IMG dggrid"
   $dggridExe ${f}.meta >& outputfiles/${f}.txt

   echo diffs\?
   diff -r outputfiles ../sampleOutput/$f
   echo end diffs for example $f
   cd ..
done
