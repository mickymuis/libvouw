/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include "ril.h"

#include <vector>
#include <cinttypes>
#include <vouw/encoder.h>
#include <vouw/matrix.h>


class Statistics {
    public:
        struct Sample {
            uint64_t patterns_in, patterns_out, patterns_out_total, total_time;
            double compression, snr_in, precision, recall;
        };
        typedef std::vector<struct Sample> SampleVecT;

        Statistics();

        static void processResult( Sample& s, const Vouw::Encoder& e, const Vouw::Matrix2D *mat, const RilOpts& ropts, double maxErr, Vouw::Matrix2D *diff =nullptr );


        void setSeparatorChar( char c ) { sep =c; }

        void push( const Sample& );

        int count() const { return samples.size(); }
        Sample average() const;

        void printSample( const Sample& s );
        void print();

    private:
        SampleVecT samples;
        char sep;
};
