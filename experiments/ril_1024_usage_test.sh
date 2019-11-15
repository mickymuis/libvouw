#!/bin/bash

size=1024
file=usage_test_$size.txt

echo "# usage $size" > $file
for i in {1..60}
do
    echo -e -n "$i\t" >> $file
    ../build/ril -rw=$size -rh=$size -rr=.1 -ru=$i:$i -rs=20:100 -n2 -e | tail -n1 >> $file
done
