set terminal pdf color
set output 'vouw-gnuplottex-fig4.pdf'
    unset key
set autoscale xfix
set autoscale yfix
set autoscale cbfix
plot '3.2.cluster.txt' matrix nonuniform with image
