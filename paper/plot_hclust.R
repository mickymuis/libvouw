E <- read.table( '2.2.cluster.txt', header=TRUE, row.names=1 )
A <- data.matrix(E)
At <- t(A)
B <- A*At
fit <- hclust(dist(B), method="ward.D")
plot(fit, main="Hierarchical Clustering of 2.2")
