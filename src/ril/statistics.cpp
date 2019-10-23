/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include "statistics.h"
#include <cstdio>

Statistics::Statistics() : sep( '\t' ) { }

void
Statistics::push( const Sample& s ) {
    samples.push_back( s );
}

void
Statistics::print() {
    printf( "pat in%cpat out%csnr%ccompression%ctime\n", sep,sep,sep,sep );
    for( auto& s : samples ) {
        printf( "%d%c%d%c%.3f%c%f%c%d\n", s.patterns_in, sep, s.patterns_out, sep, s.snr_in, sep, s.compression, sep, s.total_time );
    }
}
