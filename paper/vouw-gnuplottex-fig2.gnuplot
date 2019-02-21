set terminal pdf color
set output 'vouw-gnuplottex-fig2.pdf'
    unset key
set xrange [0:19682]
set xlabel 'Rule #'
set ylabel 'Compression ratio %'
plot 'ratio-2.3.txt' with lines
