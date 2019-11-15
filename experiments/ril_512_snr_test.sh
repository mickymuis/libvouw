#!/bin/bash

export outfile="output_snr_512.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=512 -rh=512 -rr=$1 -ru=20:$2 -rs=10:200 -n10 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile" > $outfile

run .05 30
run .1 30
run .2 30
run .3 30
run .4 40
run .5 60
run .6 120
run .7 160
run .8 160

