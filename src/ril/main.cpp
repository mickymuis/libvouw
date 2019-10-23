/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include <vouw/vouw.h>
#include <vouw/encoder.h>
#include <vouw/codetable.h>

#include <unistd.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TimeVarT;

#define DURATION(a) std::chrono::duration_cast<std::chrono::milliseconds>(a).count()
#define TIMENOW() std::chrono::high_resolution_clock::now()

#include "ril.h"
#include "statistics.h"

#define DIM_MAX 65535

struct VouwOpts {
    Vouw::Encoder::LocalSearch ls;
    Vouw::Encoder::Heuristic heur;
};

static struct VouwOpts VOUWOPTS_DEFAULTS = { Vouw::Encoder::FloodFill, Vouw::Encoder::BestN };

void
printHelp( const char* exec ) {
    fprintf( stderr,
"RIL - Synthetic dataset generator for VOUW.\n \
\n\
Usage: %s [options]\n\
General options:\n\
\t-n\tNumber of generated matrices in total (1 by default).\n\
\t-f\tUse specified type for storing the output matrices (default 'pgm').\n\
\t-o\tStore the generated matrices with the specified filename (not stored if not specified).\n\
\t-e\tEncode the generated matrix/matrices using VOUW and print statistics\n\
\t-s\tSet separator character for printing statistics (defaults to tab)\n\
\t-h\tPrint this information.\n\
Options to RIL (specify using -r)\n\
\tw=\tWidth (number of columns) of the generated matrix.\n\
\th=\tHeight (number of rows) of the generated matrix.\n\
\ta=\tSize of the alphabet (number of symbols, 256 by default).\n\
\ts=\tMin and max pattern size as an interval, e.g. 10:20 (inclusive)\n\
\tu=\tMin and max pattern occurence as an interval, e.g. 5:30 (inclusive)\n\
\tr=\tDesired signal-to-noise ratio, accepts values from 0.0 to 1.0\n\
\tn=\tGenerate uniform noise (1, default) or no noise (0, debug only)\n\
\tb=\tAllowed branching factor when generating patterns. '0' gives 'flat' patterns (default).\n\
Options to VOUW (specify using -v)\n\
\tf=\tSet local search using flood-fill to either off (0) or on (1)\n\
\tb1\tUse 'Best 1' heuristic\n\
\tbn\tUse 'Best N' heuristic\n\
", exec );
}

bool
argInt( int& i, const char *arg ) {
    int l =strlen( arg );
    if( l < 3 ) return false;
    if( arg[1] != '=' ) return false;
    i = atoi( &arg[2] );
    return true;
}

bool
argInterval( int& i, int& j, const char *arg ) {
    int l =strlen( arg );
    if( l < 5 ) return false;
    if( arg[1] != '=' ) return false;
    char *str =(char*)malloc( sizeof(char) * (strlen(arg)+1) );
    strcpy( str, arg );
    char *colon =strchr( str, ':' );
    if( colon == NULL ) {
        free( str );
        return false;
    }
    colon[0] ='\0';
    i = atoi( &str[2] );
    j = atoi( &colon[1] );
    free( str );
    return true;
}

bool
argDouble( double& d, const char *arg ) {
    int l =strlen( arg );
    if( l < 3 ) return false;
    if( arg[1] != '=' ) return false;
    d = atof( &arg[2] );
    return true;
}

bool
argBool( bool& b, const char *arg ) {
    int i;
    bool r =argInt( i, arg );
    if( r ) 
        b = i ? true : false;
    return r;
}

bool
parseRILArg( RilOpts& ropts, const char *arg ) {

    int l =strlen( arg );
    if( l == 0 ) return false;

    switch( arg[0] ) {
        case 'h':
            return argInt( ropts.rows, arg );
        case 'w':
            return argInt( ropts.cols, arg );
        case 's':
            return argInterval( ropts.parms.minSize, ropts.parms.maxSize, arg );
        case 'u':
            return argInterval( ropts.parms.minUsage, ropts.parms.maxUsage, arg );
        case 'r':
            return argDouble( ropts.parms.targetSNR,  arg );
        case 'n':
            return argBool( ropts.parms.noise, arg );
        case 'a':
            return argInt( ropts.parms.symCount, arg );
        case 'b':
            return argDouble( ropts.parms.maxBranch, arg );
        default:
            return false;
    }

    return true;
}

bool
parseVOUWArg( VouwOpts& vopts, const char *arg ) {

    int l =strlen( arg );
    if( l < 2 ) return false;

    switch( arg[0] ) {
        case 'f': 
            {
                int ff;
                bool b =argInt( ff, arg );
                if( b ) 
                    vopts.ls = ff ? Vouw::Encoder::FloodFill : Vouw::Encoder::NoLocalSearch;
                return b;
            }
        case 'b':
            switch( arg[1] ) {
                case '1':
                    vopts.heur = Vouw::Encoder::Best1;
                    break;
                case 'n':
                    vopts.heur = Vouw::Encoder::BestN;
                    break;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }

    return true;
}

void
setFilenameNumber( RilOpts& ropts, std::string filename, int n, int total ) {
    if( total == 1 || filename.empty() ) {
        ropts.outFilename =filename;
        return;
    }

    int width =log10( total ) + 1;
    std::size_t dot = filename.find_last_of( "." );

    std::stringstream ss;
    if( dot != std::string::npos ) {
        ss << filename.substr(0,dot) << std::setw(width) << std::setfill('0') << '_' << n << filename.substr(dot);
    } else {
        ss << filename << std::setw(width) << '_' << std::setfill('0') << n;
    }

    ropts.outFilename =ss.str();
}

bool
encode( Vouw::Matrix2D* mat, Statistics::Sample& s, const RilOpts& ropts, const VouwOpts& vopts ) {
    Vouw::Encoder e( mat );

    e.setLocalSearchMode( vopts.ls );
    e.setHeuristic( vopts.heur );

    TimeVarT start =TIMENOW();
    e.encode();
    TimeVarT stop  =TIMENOW();

    s.total_time =DURATION(stop-start);
    s.compression =e.ratio();
    s.patterns_out =e.codeTable()->countIfActiveGTE( ropts.parms.minSize );

    return true;
}

int
main( int argc, char **argv ) {
    

    if( argc == 1 ) {
        printHelp( argv[0] );
        return -1;
    }

    RilOpts ropts  = RILOPTS_DEFAULTS;
    VouwOpts vopts = VOUWOPTS_DEFAULTS;

    struct {
        int repeats;
        std::string outFilename;
        bool encode;
        char separator;
    } opts = {1,"",false,'\t'};

    int opt;
    while( (opt = getopt( argc, argv, "ev:r:n:f:o:s:h" )) != -1 ) {
        switch( opt ) {
            case 'e':
                opts.encode =true;
                break;
            case 'v':
                if( !parseVOUWArg( vopts, optarg ) ) {
                    fprintf( stderr, "%s - Invalid argument to VOUW (-v) '%s'\n", argv[0], optarg );
                    return -1;
                }
                break;
            case 'r':
                if( !parseRILArg( ropts, optarg ) ) {
                    fprintf( stderr, "%s - Invalid argument to RIL (-r) '%s'\n", argv[0], optarg );
                    return -1;
                }
                break;
            case 'n':
                opts.repeats = atoi( optarg );
                break;
            case 'f':
                ropts.outFiletype = std::string( optarg );
                break;
            case 'o':
                opts.outFilename = std::string( optarg );
                break;
            case 's':
                opts.separator = optarg[0];
                break;
            case 'h':
            default:
                printHelp( argv[0] );
                return -1;
        }
    }

    if( ropts.cols < 1 || ropts.cols > DIM_MAX ) {
        fprintf( stderr, "%s: Invalid or no number of columns specified.\n", argv[0] );
        return -1;
    }
    if( ropts.rows < 1 || ropts.rows > DIM_MAX ) {
        fprintf( stderr, "%s: Invalid or no number of rows specified.\n", argv[0] );
        return -1;
    }
    if( opts.repeats < 1 || opts.repeats > DIM_MAX ) {
        fprintf( stderr, "%s: Invalid or no number of total iterations specified.\n", argv[0] );
        return -1;
    }
    if( ropts.parms.symCount < 2 || ropts.parms.symCount > DIM_MAX ) {
        fprintf( stderr, "%s: Invalid or no alphabet size specified.\n", argv[0] );
        return -1;
    }
    if( ropts.parms.targetSNR < 0.0 || ropts.parms.targetSNR > 1.0 ) {
        fprintf( stderr, "%s: Invalid desired SNR specified.\n", argv[0] );
        return -1;
    }
    if( ropts.parms.maxBranch < 0.0 || ropts.parms.maxBranch > 1.0 ) {
        fprintf( stderr, "%s: Invalid desired branching factor specified.\n", argv[0] );
        return -1;
    }

    registerBuiltinWriters();

    ropts.writer = MatrixWriter::getWriter( ropts.outFiletype );
    if( ropts.writer == nullptr ) {
        fprintf( stderr, "%s: No writer available for specified filetype '%s'.\n", argv[0], ropts.outFiletype.c_str() );
        return -1;
    }

    Statistics stats;
    stats.setSeparatorChar( opts.separator );
    int err =0;

    //Ril r( ropts );
    //int err =r.run() == true ? 0 : -1;

    for( int i =0; i < opts.repeats; i++ ) {
        setFilenameNumber( ropts, opts.outFilename, i+1, opts.repeats );
        Statistics::Sample s = {0};
        Ril ril ( ropts );
        if( !ril.generate() ) {
            err =-1; break;
        }
        ropts =ril.opts();
        s.patterns_in   =ril.totalPatterns();
        s.snr_in        =ril.effectiveSNR();

        if( opts.encode ) {
            if( !encode( ril.matrix(), s, ropts, vopts ) ) {
                err =-1; break;
            }
        }
        stats.push( s );
    }

    if( opts.encode )
        stats.print();

    MatrixWriter::destroy();

    return err;
}
