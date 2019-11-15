#!/bin/bash

size=256
file=output_sym_$size.txt

echo "# symbol count test $size" > $file
for i in {2..256}
do
    echo -e -n "$i\t" >> $file
    ../build/ril -rw=$size -rh=$size -ra=$i -rr=.1 -ru=10:20 -rs=10:50 -n1 -e | tail -n1 >> $file
done

