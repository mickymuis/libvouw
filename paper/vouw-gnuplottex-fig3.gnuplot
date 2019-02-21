set terminal pdf color
set output 'vouw-gnuplottex-fig3.pdf'
    unset key
set autoscale xfix
set autoscale yfix
set autoscale cbfix
plot '2.2.cluster.txt' matrix nonuniform with image
