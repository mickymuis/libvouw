set terminal pdf color
set output 'vouw-gnuplottex-fig1.pdf'
    unset key
set xrange [0:255]
set xlabel 'Rule #'
set ylabel 'Compression ratio %'
plot 'ratio-3.2.txt' with lines
