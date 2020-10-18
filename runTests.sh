#!/bin/sh

# Number of arguments check
if [ $# -ne 3 ] ; then
    echo "Usage: $0 inputdir outputdir maxthreads"
    exit 1
fi

inputdir=$1
outputdir=$2
maxthreads=$3

# Make and check for errors
if ! make > /dev/null; then
	echo "Error: Compilation failed"
	exit 1
fi

for file in `ls $1`; do
	echo $file
done

# Make clean and check for errors
if ! make clean > /dev/null; then
	echo "Error: Compilation failed"
	exit 1
fi
