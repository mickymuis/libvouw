/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include "ril.h"

#include <vector>
#include <vouw/encoder.h>
#include <vouw/matrix.h>

class Statistics {
    public:
        struct Sample {
            int patterns_in, patterns_out, patterns_out_total;
            double compression, snr_in, precision, recall;
            int total_time;
        };
        typedef std::vector<struct Sample> SampleVecT;

        Statistics();

        static void processResult( Sample& s, const Vouw::Encoder& e, const Vouw::Matrix2D *mat, const RilOpts& ropts, double maxErr );

        void setSeparatorChar( char c ) { sep =c; }

        void push( const Sample& );

        int count() const { return samples.size(); }

        void print();

    private:
        SampleVecT samples;
        char sep;
};
