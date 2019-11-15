#!/bin/bash

export outfile="output_snr_2048.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=2048 -rh=2048 -rr=$1 -ru=50:$2 -rs=10:3200 -n2 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile" > $outfile

run .05 100
run .1 100
run .2 100
run .3 100
run .4 200
run .5 200
run .6 400
run .7 400
run .8 400

