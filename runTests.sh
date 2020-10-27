#!/bin/bash

# Number of arguments check
if [ $# -ne 3 ] ; then
    echo "Usage: $0 inputdir outputdir maxthreads"
    exit 1
fi

inputdir=$1
outputdir=$2
maxthreads=$3

filter="TecnicoFS completed in"

if [ ! -d $outputdir ]; then
	echo "Error: folder named $outputdir does not exist"
	exit 1
else
	for file in `ls -p $inputdir | grep -v /`; do
		for numthreads in $(seq 1 $maxthreads); do
			echo "InputFile=$file NumThreads=$numthreads"
			./tecnicofs "$inputdir/$file" "$outputdir/${file%.*}-$numthreads.txt" $numthreads mutex | grep "$filter"
		done
	done
fi
