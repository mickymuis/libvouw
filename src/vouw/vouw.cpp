/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#include <vouw/vouw.h>
#include <cmath>
#include <climits>

VOUW_NAMESPACE_BEGIN

/* The 2 logarithm of c = 2.865064 */
#define LOG2C 1.5185673663648485

/** This funcion computes log2*(n) + log2(c)
  * log2*(n) is defined as log2(n) + log2(log2(n)) + ...
  */
double 
uintCodeLength( unsigned int n ) {
    double l =LOG2C; // Code lengt in bits
    double logn =log2( n );

    // Only add the result if non-zero and positive
    // This is guaranteed to halt as log2(log2(... will eventually become negative
    while( logn > 0.0 ) {
        l += logn;
        logn = log2( logn );
    };

    return l;
}

double binom( unsigned int n, unsigned int k ) {
    unsigned c = 1, i;
  if (k > n-k) k = n-k;  /* take advantage of symmetry */
  for (i = 1; i <= k; i++, n--) {
    if (c/i > UINT_MAX/n) return 0;  /* return 0 on overflow */
    c = c/i * n + c%i * n / i;  /* split c*n/i into (c/i*i + c%i)*n/i */
  }
  return c;
}

VOUW_NAMESPACE_END
