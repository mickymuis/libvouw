/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include "ril.h"
#include <cstdio>

static std::default_random_engine rgen;
static std::uniform_int_distribution<int> ddist(1,8);

int DIRSTEP[][2] = {
    { 0, 0 },
    { 0,-1 },
    {-1,-1 },
    {-1, 0 },
    {-1, 1 },
    { 0, 1 },
    { 1, 1 },
    { 1, 0 },
    { 1,-1 }
};

void 
Direction::randomize() {
    d =ddist( rgen );
}

void 
Direction::cwTurn() {
    if( d == Invalid ) return;
    if( d == NorthWest ) d = North;
    d++;
}

void 
Direction::ccwTurn() {
    if( d == Invalid ) return;
    if( d == North ) d = NorthWest;
    d--;
}

Vouw::Coord2D 
Direction::stepOnce( const Vouw::Coord2D& coord ) const {
    return Vouw::Coord2D( coord.row() + DIRSTEP[d][1], coord.col() + DIRSTEP[d][0], coord.rowLength() );
}

Ril::~Ril() {
    if( mat ) delete mat;
}

bool
Ril::generate() {
    pCount =0;
    flagCount =0;
    
    if( !mat )
        mat = new Vouw::Matrix2D( ropts.cols, ropts.rows, 0 );

    if( ropts.parms.noise ) {
        noise_dist = std::uniform_int_distribution<int>(0,ropts.parms.symCount-1);
        signal_dist = std::uniform_int_distribution<int>(0,ropts.parms.symCount-1);
    } else {
        signal_dist = std::uniform_int_distribution<int>(1,ropts.parms.symCount);
    }

    mat->clear();


    while( 1 ) {
        snr = (double)flagCount / (double)mat->count();
        //printf( "snr = %f\n", snr );
        if( snr >= ropts.parms.targetSNR ) break;
        if( !generatePattern2() ) continue;
    }
    
    if( ropts.parms.noise ) {
        Vouw::Matrix2D::ElementT *e =mat->data();
        for( int i =0; i < mat->count(); i++ ) {
            if( !((*e) & VOUW_UNODE32_FLAGGED) )
                (*e) =noise_dist(rgen);
            e++;
        }
    }
    
    ropts.writer->writeMatrix( *mat, ropts.outFilename );

    return true;
}

//////

bool
Ril::generatePattern2() {

    CoordVecT coords;
    for( int i =0; i < ropts.parms.maxUsage; i++ ) {

        Vouw::Coord2D c; 
        if( !randEmptyCoord( c ) ) break;
        coords.push_back( c );
    }

    if( coords.size() < ropts.parms.minUsage ) return false;

    bool enabled[coords.size()];

    for( int i=0; i < coords.size(); i++ ) {
        flag( coords[i] );
        enabled[i] =true;
    }

    int size =0, branches =0;
    bool b =generatePatternRecursive2( size, branches, 0, coords, enabled );

    for( int i=0; i < coords.size(); i++ ) {
        if( !b || !enabled[i] ) unflag( coords[i] );
    }

    //printf( "Generated pattern with %d elements, %d fragments/subpatterns, flagcount=%d\n", size, branches, flagCount );
    pCount += branches;

    return b;
}

bool
Ril::generatePatternRecursive2( int& size, int& branches, int usage, CoordVecT coords, bool enabled[] ) {
    
    size++;

    bool quit;
    CoordVecT ncoords;
    std::vector<int> flips, flags;
    Direction d; d.randomize();
    bool directions_tried[9] = { false };

    Direction bestD;
    int bestCount;
    
    Vouw::Matrix2D::ElementT el =signal_dist( rgen );
    
    if( size == ropts.parms.maxSize || branches > ropts.parms.maxUsage * ropts.parms.maxBranch ) goto GENERATE;

SEARCH:

    bestCount =0;

    // There are 8 directions to expand this pattern in, we try them all
    // The goal is to find the direction where the most coords have an unflagged (free) adjacent coord
    for( int i =0; i < 8; i++ ) {
        int count =0;
        if( directions_tried[d.getDirection()] ) continue;
        for( int j=0; j < coords.size(); j++ ) {
            if( !enabled[j] ) continue;
            Vouw::Coord2D c2 =d.stepOnce( coords[j] );
            if( !mat->checkBounds( c2 ) || mat->isFlagged( c2 ) ) 
                continue;

            count++;
        }

        if( count > bestCount ) {
            bestCount =count;
            bestD =d;
        }
        d.cwTurn();
    }
    //printf( "bestCount=%d\n", bestCount );

    if( bestCount < ropts.parms.minUsage ) goto GENERATE;

    directions_tried[bestD.getDirection()] =true;
    ncoords.clear();

    for( int i=0; i < coords.size(); i++ ) {
        Vouw::Coord2D c2 =bestD.stepOnce( coords[i] );
        if( enabled[i] ) {
            if( !mat->checkBounds( c2 ) || mat->isFlagged( c2 ) ) {
                flips.push_back( i );
                enabled[i] =false;
            } else {
                flags.push_back( i );
                flag( c2 );
            }
        }
        ncoords.push_back( c2 );
    }
    
    if( !generatePatternRecursive2( size, branches, coords.size(), ncoords, enabled ) ) {
        if( size < ropts.parms.minSize )
       //     goto ROLLBACK;
            goto ROLLBACK_AND_SEARCH;
    }
    //if( size < ropts.parms.maxSize ) goto SEARCH;
   
GENERATE:
    
    if( size < ropts.parms.minSize ) goto ROLLBACK_AND_QUIT;
    
    // Paint the pixels with a random value
    for( int i=0; i < coords.size(); i++ ) {
        if( enabled[i] )
            mat->setValue( coords[i], el );
    }
    // Reset any flags we may have set at coordinates that do not belong to the pattern
    for( auto i : flags ) {
        if( !enabled[i] )
            unflag( ncoords[i] );
    }

    if( coords.size() != usage ) branches++;

    //printf( "-- (|coords|/usage) (%d,%d) drawn %d elements\n", coords.size(), usage, debugCount ); 
    return true;
   
    

ROLLBACK_AND_SEARCH:

    quit =false; goto ROLLBACK;

ROLLBACK_AND_QUIT:

    quit =true; goto ROLLBACK;

ROLLBACK:


    for( auto i : flags ) {
        unflag( ncoords[i] );
    }
    for( auto i : flips ) {
        enabled[i] =true;
    }
    flips.clear(); flags.clear();

    if( quit ) {
        size--;
        return false;
    } else
        goto SEARCH;
}

/////

bool
Ril::generatePattern() {

    CoordVecT coords;
    for( int i =0; i < ropts.parms.maxUsage; i++ ) {

        Vouw::Coord2D c; 
        if( !randEmptyCoord( c ) ) break;
        coords.push_back( c );
    }

    if( coords.size() < ropts.parms.minUsage ) return false;

    for( auto& c : coords ) {
        flag( c );
    }

    int size =0, branches =0;
    bool b =generatePatternRecursive( size, branches, 0, coords );

    if( !b ) {
        for( auto& c : coords ) {
            unflag( c );
        }

    }
    
    pCount += branches;

    //printf( "Generated pattern with %d elements, %d fragments/subpatterns, flagcount=%d\n", size, branches, flagCount );

    return b;
}

bool
Ril::generatePatternRecursive( int& size, int& branches, int usage, CoordVecT coords ) {
    
    size++;

    CoordVecT ncoords;
    CoordVecT flags;
    Direction d;

    Direction bestD;
    int bestCount;
    
    if( size == ropts.parms.maxSize || branches > ropts.parms.maxUsage * ropts.parms.maxBranch ) goto GENERATE;

SEARCH:

    d.randomize();
    bestCount =0;

    // There are 8 directions to expand this pattern in, we try them all
    // The goal is to find the direction where the most coords have an unflagged (free) adjacent coord
    for( int i =0; i < 8; i++ ) {
        int count =0;
        for( auto& c : coords ) {
            Vouw::Coord2D c2 =d.stepOnce( c );
            if( !mat->checkBounds( c2 ) || mat->isFlagged( c2 ) ) 
                continue;

            count++;
        }

        if( count > bestCount ) {
            bestCount =count;
            bestD =d;
        }
        //if( ncoords.size() == coords.size() ) goto DIRFOUND;
        d.cwTurn();
    }
    //printf( "bestCount=%d\n", bestCount );

    //if( bestCount < coords.size() ) goto GENERATE;
    if( bestCount < ropts.parms.minUsage ) goto GENERATE;

    ncoords.clear();
    for( auto& c : coords ) {
        Vouw::Coord2D c2 =bestD.stepOnce( c );
        if( !mat->checkBounds( c2 ) || mat->isFlagged( c2 ) ) 
            continue;
        ncoords.push_back( c2 );
        flags.push_back( c2 );
        flag( c2 );
    }
    
    //if( bestCount < coords.size() ) 

    if( !generatePatternRecursive( size, branches, coords.size(), ncoords ) ) {
       // if( size < ropts.parms.minSize )
       //     goto ROLLBACK;
       goto SEARCH;
    }
    if( size < ropts.parms.maxSize ) goto SEARCH;
   
GENERATE:

    Vouw::Matrix2D::ElementT el =signal_dist( rgen );
    
    if( size < ropts.parms.minSize ) goto ROLLBACK;

    for( auto& c : coords ) {
        mat->setValue( c, el );
    }

    if( coords.size() != usage ) branches++;

    //printf( "-- usage %d\n", coords.size() ); 

    return true;
   
    

ROLLBACK:

    size--;

    for( auto& c : flags ) {
        unflag( c );
    }
    return false;
}

bool
Ril::randEmptyCoord( Vouw::Coord2D& coord ) {
    std::uniform_int_distribution<int> rdist(0,ropts.rows-1);
    std::uniform_int_distribution<int> cdist(0,ropts.cols-1);

    for( int i =0; i < 1000; i++ ) { // TODO: fix this wtf
        Vouw::Coord2D c( rdist(rgen), cdist(rgen), ropts.cols );
        if( mat->isFlagged( c ) == false ) {
            coord =c;
            //printf( "(%d,%d)\n", c.col(), c.row() );
            return true;
        }
    }
    return false;
}

void 
Ril::flag( const Vouw::Coord2D& c ) {
    if( !mat->isFlagged( c ) ) {
        mat->setFlagged( c, true );
        flagCount++;
    }
}

void 
Ril::unflag( const Vouw::Coord2D& c ) {
    if( mat->isFlagged( c ) ) {
        mat->setFlagged( c, false );
        flagCount--;
    }

}

