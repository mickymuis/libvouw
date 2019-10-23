/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include <vector>

class Statistics {
    public:
        struct Sample {
            int patterns_in, patterns_out, patterns_out_total;
            double compression, snr_in;
            int total_time;
        };
        typedef std::vector<struct Sample> SampleVecT;

        Statistics();

        void setSeparatorChar( char c ) { sep =c; }

        void push( const Sample& );

        int count() const { return samples.size(); }

        void print();

    private:
        SampleVecT samples;
        char sep;
};
