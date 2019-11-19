#!/bin/bash

export outfile="output_snr_b1_1024.txt"

function run {
    echo "Running tests with SNR=$1 Best-1"
    ../build/ril -rw=1024 -rh=1024 -rr=$1 -ru=50:$2 -rs=10:800 -vb1 -n10 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile usage=50 Best-1" > $outfile

run .05 80
run .1 80
run .2 80
run .3 100
run .4 100
run .5 150
run .6 150
run .7 150
run .8 180


