/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include <string>
#include <vector>
#include <random>
#include "matrixwriter.h"

typedef std::vector<Vouw::Coord2D> CoordVecT;

struct RilParms {
    int symCount;
    bool noise;
    int minSize, maxSize;
    int minUsage, maxUsage;
    double targetSNR;
    double maxBranch;
};

struct RilOpts {
    int cols, rows;
    std::string outFilename;
    std::string outFiletype;
    MatrixWriter *writer;
    RilParms parms;
};
static const struct RilParms RILPARMS_DEFAULTS={256,true,40,80,20,50,.1,0.0};
static const struct RilOpts RILOPTS_DEFAULTS = {0,0,"","pgm",nullptr,RILPARMS_DEFAULTS};

class Direction {
    public:
        enum DirT {
            Invalid, North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest
        };
        Direction() {}
        Direction( int dir ) : d( dir ) {}

        void setDirection( DirT dir ) { d =dir; }
        void randomize();
        void cwTurn();
        void ccwTurn();

        int getDirection() const { return d; }

        Vouw::Coord2D stepOnce( const Vouw::Coord2D& coord ) const;

    private:
        int d;
};

class Ril {
    public:
        Ril( const RilOpts& opts ) : ropts ( opts ), mat( nullptr) {}
        ~Ril();

        RilOpts opts() const { return ropts; }

        bool generate();
        Vouw::Matrix2D *matrix() { return mat; }
        int totalPatterns() const { return pCount; }
        double effectiveSNR() const { return snr; }

    private:
        bool generatePatternRecursive( int& size, int& frags, int usage, CoordVecT coords );
        bool generatePattern();
        bool generatePatternRecursive2( int& size, int& frags, int usage, CoordVecT coords, bool [] );
        bool generatePattern2();

        bool randEmptyCoord( Vouw::Coord2D& coord );
        void flag( const Vouw::Coord2D& c );
        void unflag( const Vouw::Coord2D& c );
        
        int flagCount;
        int pCount;
        double snr;
        RilOpts ropts;
        Vouw::Matrix2D *mat;
        std::uniform_int_distribution<int> noise_dist;
        std::uniform_int_distribution<int> signal_dist;
};
