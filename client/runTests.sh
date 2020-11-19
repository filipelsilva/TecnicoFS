#!/bin/bash

# Number of arguments check
if [ $# -ne 3 ]; then
	echo "Usage: $0 inputdir outputdir maxthreads"
	exit 1
fi

inputdir=$1
outputdir=$2
maxthreads=$3
filter="TecnicoFS completed in"

if [ "$maxthreads" -lt 1 ]; then
	echo "Error: Invalid number of threads"
	exit 1
fi

if [ ! -d "$inputdir" ]; then
	echo "Error: folder named $inputdir does not exist"
	exit 1
fi

if [ ! -d "$outputdir" ]; then
	echo "Error: folder named $outputdir does not exist"
	exit 1
fi

for file in "$inputdir"/*.txt; do
	filename=${file#"$inputdir"/}
	filename=${filename%.*}
	for numthreads in $(seq 1 "$maxthreads"); do
		echo "InputFile=$file NumThreads=$numthreads"
		./tecnicofs "$file" "$outputdir/$filename-$numthreads.txt" "$numthreads" | grep "$filter"
	done
done
