#!/bin/bash

size=128
file=usage_test_$size.txt

echo "# usage $size" > $file
for i in {1..60}
do
    echo -e -n "$i\t" >> $file
    ../build/ril -rw=$size -rh=$size -rr=.1 -ru=$i:$i -rs=10:50 -n10 -e | tail -n1 >> $file
done
