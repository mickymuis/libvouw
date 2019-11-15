#!/bin/bash

export outfile="output_snr_noff_2048.txt"

function run {
    echo "Running tests with SNR=$1"
    ../build/ril -rw=2048 -rh=2048 -rr=$1 -ru=120:$2 -rs=10:3200 -vf=0 -n3 -e 2> /dev/null | tail -n1 >> $outfile
}

echo "# $outfile usage=120" > $outfile

run .05 150
run .1 150
run .2 150
run .3 200
run .4 240
run .5 280
run .6 400
run .7 500
run .8 600

