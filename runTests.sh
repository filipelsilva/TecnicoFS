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

# Make and check for errors
if ! make > /dev/null; then
	echo "Error: Compilation failed"
	exit 1
fi

# Run the program
if [ -d $outputdir ]; then
	for file in `ls -p $inputdir | grep -v /`; do
	#for file in $inputdir/*.txt; do 
		#file=${file#"${inputdir}/"}
		for (( numthreads=1; numthreads<=$maxthreads; numthreads++ )); do
			echo "InputFile=$file NumThreads=$numthreads"
			./tecnicofs "$inputdir/$file" "$outputdir/${file%.*}-$numthreads.txt" $numthreads mutex | grep "$filter"
		done
	done
else
	# if outputdir does not exist
	echo "Error: $outputdir does not exist"
	exit 1
fi

# Make clean and check for errors
if ! make clean > /dev/null; then
	echo "Error: Compilation failed"
	exit 1
fi
