#!/bin/bash

export outfile="output_snr_noff_256.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=256 -rh=256 -rr=$1 -ru=10:$2 -rs=10:50 -vf=0 -n10 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile usage=10" > $outfile

run .05 20
run .1 20
run .2 20
run .3 20
run .4 20
run .5 20
run .6 50
run .7 100
run .8 150
