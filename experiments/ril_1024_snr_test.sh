#!/bin/bash

export outfile="output_snr_1024.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=1024 -rh=1024 -rr=$1 -ru=40:$2 -rs=10:800 -n5 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile" > $outfile

run .05 60
run .1 60
run .2 60
run .3 60
run .4 80
run .5 120
run .6 150
run .7 180
run .8 220


