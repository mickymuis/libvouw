/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include "statistics.h"
#include <cstdio>
#include <functional>
#include <algorithm>
#include <vouw/pattern.h>
#include <vouw/codetable.h>
#include <vouw/instance.h>
#include <vouw/instance_matrix.h>

/** Test whether a pattern fits the requirements of a pattern that we expect to find given the input data */
bool
patternIsExpected( const Vouw::Pattern* p, double maxErr, const RilOpts& ropts ) {

    if( !p->isActive() ) return false;
    if( p->size() == 1 ) return false; // Singleton
    if( p->size() < ((double)ropts.parms.minSize - (double)ropts.parms.minSize * maxErr)
    ||  p->size() > ((double)ropts.parms.maxSize + (double)ropts.parms.maxSize * maxErr) )
        return false;
    
    if( p->usage() < ((double)ropts.parms.minUsage - (double)ropts.parms.minUsage * maxErr)
    ||  p->usage() > ((double)ropts.parms.maxUsage + (double)ropts.parms.maxUsage * maxErr) )
        return false;

    return true;
}

Statistics::Statistics() : sep( '\t' ) { }

void 
Statistics::processResult( Sample& s, const Vouw::Encoder& e, const Vouw::Matrix2D *mat, const RilOpts& ropts, double maxErr ) {
    s.compression =e.ratio();

    // Let's count the patterns
    auto f =std::bind( patternIsExpected, std::placeholders::_1, maxErr, ropts );

    s.patterns_out =std::count_if( e.codeTable()->begin(), e.codeTable()->end(), f );
    s.patterns_out_total =e.codeTable()->countIfActiveNonSingleton();

    // Compare the encoded dataset with the input data
    // We try to 'overlay' the input patterns with the found patterns

    int np =0, tp =0, n =0; // Total positives, true positives, total ground truth elements

    for( int i =0; i < mat->height(); i++ )
        for( int j =0; j < mat->width(); j++ ) {

            Vouw::Coord2D c( i,j,mat->width() );
            bool inputIsSignal = mat->isFlagged( c );

            Vouw::InstanceVector::IndexT it =e.instanceMatrix()[c];
            Vouw::Pattern *p =e.instanceVector()[it].pattern();
            
            bool outputIsSignal =(p->size() > 1);

            if( outputIsSignal ) {
                np++;
                if( inputIsSignal )
                    tp++;
            }
            if( inputIsSignal ) n++;
        }

    // Precision is true positives divided by total positives 
    s.precision =(double)tp / (double)np; 

    // Recall is true positives divided by actual ground truth elements
    s.recall =(double)tp / (double)n;


}

void
Statistics::push( const Sample& s ) {
    samples.push_back( s );
}

Statistics::Sample 
Statistics::average() const {

    Sample avg ={0};
    // Programmers are lazy, so we reinterpret 
    // each sample as two arrays (int and doubles) and sum them in a for-loop
    uint64_t *A =&avg.patterns_in;
    double   *B =&avg.compression;

    for( auto& s : samples ) {
        const uint64_t *a =&s.patterns_in; // first elements
        const double   *b =&s.compression;
        for( int i =0; i < 4; i++ ) {
            A[i] += a[i];
            B[i] += b[i];
        }
    }
    // Divide to obtain the averages
    for( int i =0; i < 4; i++ ) {
        A[i] /= samples.size();
        B[i] /= (double)samples.size();
    }
    return avg;
}

void 
Statistics::printSample( const Sample& s ) {
        printf( "%d%c%d%c%d%c%.3f%c%f%c%.4f%c%.4f%c%d\n", 
                s.patterns_in, sep, 
                s.patterns_out, sep, 
                s.patterns_out_total, sep, 
                s.snr_in, sep, 
                s.compression, sep, 
                s.precision, sep, 
                s.recall, sep, 
                s.total_time );

}

void
Statistics::print() {
    printf( "pat in%cpat out%cpat tot%csnr%ccompression%cprec%crecall%ctime\n", sep,sep,sep,sep,sep,sep,sep );
    for( auto& s : samples ) {
        printSample( s );
    }
    printf( "\n" );
    printSample( average() );
}
