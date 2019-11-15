#!/bin/bash

export outfile="output_snr_4096.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=4096 -rh=4096 -rr=$1 -ru=70:$2 -rs=10:128000 -n2 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile" > $outfile

run .05 100
run .1 100
run .2 150
run .3 150
run .4 250
run .5 250
run .6 450
run .7 600
run .8 800


