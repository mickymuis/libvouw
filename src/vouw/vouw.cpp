/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#include <vouw/vouw.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

typedef struct {
    pattern_t* p1,* p2;
    int row, col, variant;
    unsigned int usage;
} candidate_t;

/*typedef struct {

} vouw_private_t;*/

static void
computeStdBits( vouw_t* v ) {
    //v->stdBitsPerOffset = log2( (double)v->mat->base );
    v->stdBitsPerOffset = log2( (double)v->mat->count ) + log2( v->mat->base );
    v->stdBitsPerPivot = log2( (double)v->mat->count );
    v->stdBitsPerVariant = log2( (double)v->mat->base );
}

static double
computeCodeTableBits( vouw_t* v ) {
    double bits =0.0;
    struct list_head* pos;
    list_for_each( pos, &(v->codeTable->list) ) {
        pattern_t* tmp = list_entry( pos, pattern_t, list );
        bits += tmp->codeLength + v->stdBitsPerOffset * tmp->size;
    }
    return bits;
}

static double
computeEncodedBits( vouw_t* v ) {
    double bits =0.0;
    struct list_head* pos;
    list_for_each( pos, &(v->encoded->list) ) {
        region_t* tmp = list_entry( pos, region_t, list );
        bits += tmp->pattern->codeLength + v->stdBitsPerPivot + v->stdBitsPerVariant;
    }
    return bits;
}

static void
updateEncodedLength( vouw_t* v ) {
    pattern_list_updateCodeLength( v->codeTable, v->mat->count );
    v->ctBits = computeCodeTableBits( v );
    v->encodedBits = computeEncodedBits( v );
}

static void
maskRegion( vouw_matrix_t* m, pattern_t* p, vouw_coord_t pivot ) {
    for( int i =0; i < p->size; i++ ) {
        vouw_coord_t c = pattern_offset_abs( pivot, p->offsets[i] );

        vouw_matrix_setFlagged( m, c, true );
    }
}

static int 
computeUsage( vouw_t* v, pattern_t* p1, int v1, pattern_t* p2, int v2, pattern_offset_t p2_offset ) {
    int usage =0;
    const int base = v->mat->base;
    struct list_head *pos1, *pos2;

    list_for_each( pos1, &(v->encoded->list) ) {
        region_t* r1 = list_entry( pos1, region_t, list );
        if( r1->pattern != p1 )
            continue;

        list_for_each( pos2, &(v->encoded->list) ) { 
            region_t* r2 = list_entry( pos2, region_t, list );
            if( r2->pattern != p2 )
                continue;
            
            if ( r2->pivot.row - r1->pivot.row == p2_offset.row &&
                 r2->pivot.col - r1->pivot.col == p2_offset.col ) {
                // Compute the difference between r1's variant and variant v1
                int vv1 = (r1->variant - v1) % base;
                vv1 = vv1 < 0 ? vv1+base : vv1;
                
                // Compute the difference between r2's variant and variant v2
                int vv2 = (r2->variant - v2) % base;
                vv2 = vv2 < 0 ? vv2+base : vv2;
                if( vv1 == vv2 )
                    usage ++;
                break;
            }
        }
    }
    return usage;
}

/*
 * Calculate the gain in encoding size if patterns p1 and p2 were replaced by their union.
 * This union is assumed to have estimated usage p_usage,
 * which is assumed to be less or equal than the usages of p1 and p2.
 * The return value is the difference in encoding side in bits.
 */
static double
computeGain( vouw_t* v, pattern_t* p1, pattern_t* p2, int p_usage ) {

    const int totalNodes = v->mat->count;
    const double oldBits = v->ctBits + v->encodedBits; // MDL's L(M) + L(M|D)
    double newBits = oldBits;

    // Step 1. remove the size of the old patterns completely
    newBits -= (p1->codeLength + v->stdBitsPerPivot + v->stdBitsPerVariant) * p1->usage;
    newBits -= (p2->codeLength + v->stdBitsPerPivot + v->stdBitsPerVariant) * p2->usage;

    // Step 2. add the bits from the old patterns that still have usage > 0
    int p1_usage = p1->usage - p_usage;
    if( p1_usage > 0 )
        // Pattern p1 will remain in the code table
        newBits += (-log2( (double)p1_usage / (double)totalNodes ) + v->stdBitsPerPivot + v->stdBitsPerVariant) * p1_usage;
    else if( p1->size > 1 ) 
        // Pattern p1 will be removed from the code table
        newBits -= (v->stdBitsPerOffset * p1->size + p1->codeLength);

    int p2_usage = p2->usage - p_usage;
    if( p2_usage > 0 )
        newBits += (-log2( (double)p2_usage / (double)totalNodes ) + v->stdBitsPerPivot + v->stdBitsPerVariant) * p2_usage;
    else if( p2->size > 1 ) 
        // Pattern p2 will be removed from the code table
        newBits -= (v->stdBitsPerOffset * p2->size + p2->codeLength);

    // Step 3. add the length from the union pattern p
    double p_codeLength = -log2( (double)p_usage / (double)totalNodes );  
    // data part
    newBits += (p_codeLength + v->stdBitsPerPivot + v->stdBitsPerVariant) * (p_usage);
    // code table part
    newBits += v->stdBitsPerOffset * (p1->size + p2->size) + p_codeLength;
    
    return oldBits - newBits;
}

static region_t*
createRegion( vouw_t* v, pattern_t* p, vouw_coord_t pivot, int variant, bool mask ) {
    region_t* region = (region_t*)malloc( sizeof( region_t ) );
    region->pivot =pivot;
    region->pattern =p;
    region->variant =variant;

    if( mask ) {
        for( int i =0; i < p->size; i++ ) {
            // For each offset, compute its location on the matrix
            vouw_coord_t c = pattern_offset_abs( pivot, p->offsets[i] );

            vouw_matrix_setFlagged( v->mat, c, true );
        }
    }

    list_add( &(region->list), &(v->encoded->list) );
    return region;
}

static pattern_t*
mergeEncodedPatterns( vouw_t* v, pattern_t* p1, pattern_t* p2, int variant, pattern_offset_t p2_offset ) {
    const int base = v->mat->base;
    // Create the union pattern of p1 and p2
    pattern_t* p_union = pattern_createVariantUnion( p1, p2, variant, p2_offset, base );
    //list_add( &(p_union->list), &(p2->list) );
    list_add( &(p_union->list), &(v->codeTable->list) );

    // Iterate over every possible combination of p1 and p2
    // Complexity is approx. (N/2)^2 in the list of regions. I'm not proud of it.
    struct list_head *pos1, *pos2, *tmp1, *tmp2;
    list_for_each_safe( pos1, tmp1, &(v->encoded->list) ) {
        region_t* r1 = list_entry( pos1, region_t, list );
        if( r1->pattern != p1 )
            continue;
        
        list_for_each_safe( pos2, tmp2, &(v->encoded->list) ) { 
            region_t* r2 = list_entry( pos2, region_t, list );
            if( (r1->pivot.row == r2->pivot.row && r1->pivot.col == r2->pivot.col) || 
                r2->pattern != p2 )
                continue;
            
            
            if( r2->pivot.row - r1->pivot.row == p2_offset.row &&
                r2->pivot.col - r1->pivot.col == p2_offset.col ) {
        
                // Compute the difference between r1's variant and variant r2's
                int vv = ((r2->variant + base) - r1->variant) % base;

                if( vv != variant )
                    continue;

                // Compute the variant for the new region
                int r1_value = (r1->pattern->offsets[0].value + r1->variant) % base; 
                int vn = ((r1_value + base) - p1->offsets[0].value) % base;
                assert( r1->pattern->offsets[0].col == 0 && r1->pattern->offsets[0].row == 0 &&
                        p2->offsets[0].col == 0 && p1->offsets[0].row == 0 );

                vouw_coord_t pivot = r1->pivot;
                // Fix the list iterators if we're removing r1 and r2
                if( tmp1 == &r2->list )
                    tmp1 = r2->list.next;
                if( tmp2 == &r1->list )
                    tmp2 = r1->list.next;
                // Create a new region at this pivot containing p_union
                region_t* region = (region_t*)malloc( sizeof( region_t ) );
                region->pivot =pivot;
                region->pattern =p_union;
                region->variant =vn;
                
                //list_add( &(region->list), &(v->encoded->list) );
                list_add_tail( &(region->list), &(r1->list) );
                
                // Remove and free both r1 and r2
                r1->pattern->usage--;
                list_del( &(r1->list) );
                region_free( r1 );
                r2->pattern->usage--;
                list_del( &(r2->list) );
                region_free( r2 );


                p_union->usage ++;
                break;
            }
        }
    }

    return p_union;
}

/*
 * Compute the gain when @p is removed from @v.
 * If this gain is positive, actually remove @p and replace all of its regions
 * with the singleton pattern.
 */
static void
prunePattern( vouw_t* v, pattern_t* p ) {
    // We cannot prune the singleton pattern
    if( p->size == 1 )
        return;

    // Usage is zero, removing anyway
    if( p->usage == 0 ) {
        list_del( &(p->list) );
        pattern_free( p );
        return;
    }

    // The gain of removing p consists of (a) removing regions with p 
    // (b) removing p from the code table.
    double gain =(p->codeLength + v->stdBitsPerPivot + v->stdBitsPerVariant) * p->usage
                +(v->stdBitsPerOffset * p->size + p->codeLength);

    // The singleton pattern can now be used to cover all regions of p
    // This adds to the gain because it can be coded with fewer bits.
    gain += v->singleton->codeLength * (v->singleton->usage + 1);
    // The number of singleton regions that will be encoded
    int d = p->usage * p->size;
    int u = d + v->singleton->usage;
    gain -= -log2( (double)u / (double)v->mat->count ) * (u+1);
    // Extra pivots and variants that are needed
    gain -= (v->stdBitsPerPivot + v->stdBitsPerVariant) * d;

#ifdef VOUW_DEBUG_PRINT
    fprintf( stderr, "-- Optional removal of %c gives a gain of %f bits.\n", 'A' + p->label, gain );
#endif

}

vouw_t*
vouw_createFrom( vouw_matrix_t* m ) {
    // We're creating an encoded version of r using a standard code table
    vouw_t* v = (vouw_t*)malloc( sizeof( vouw_t ) );
    v->buffer = NULL;
    v->mat =m;

    // The initial code table contains only one pattern
    v->codeTable = (pattern_t*)malloc( sizeof( pattern_t ) );
    INIT_LIST_HEAD( &(v->codeTable->list) );
    v->codeTable->size =0;
    pattern_t* p0 = pattern_createSingle( 0 );
    list_add( &(p0->list), &(v->codeTable->list ) );
    v->singleton = p0;

    // The encoded data is represented in a linked list
    v->encoded = (region_t*)malloc( sizeof( region_t ) );
    v->encoded->pattern =NULL;
    v->encoded->masked =false;
    INIT_LIST_HEAD( &(v->encoded->list) );

    // Now we encode each node in the matrix using the standard code table
    for( int i =0; i < m->height; i++ ) {
        vouw_row_t* row = &m->rows[i];
        for( int j =0; j < row->size; j++ ) {

            // Create a region for every singleton on every node
            vouw_coord_t pivot = { i,j };
            int value = vouw_matrix_value( m, pivot );
            region_t* region = region_create( p0, pivot );
            region->variant = value;

            // Add to the encoded dataset
            list_add_tail( &(region->list), &(v->encoded->list) );

            // Increment the pattern's usage so we can compute its code length later
            p0->usage++;
        }
    }

    // Compute the initial encoding sizes for the data and the code table
    computeStdBits( v );
    updateEncodedLength( v );
    return v;
}

vouw_t*
vouw_createEncodedUsing( vouw_matrix_t* m, pattern_t* codeTable ) {
    // We're creating an encoded version of r using a given code table
    vouw_t* v = (vouw_t*)malloc( sizeof( vouw_t ) );
    v->buffer = NULL;
    v->mat =m;

    // Copy the code table to the newly created object
    v->codeTable = (pattern_t*)malloc( sizeof( pattern_t ) );
    INIT_LIST_HEAD( &(v->codeTable->list) );

    struct list_head* pos;
    list_for_each( pos, &(codeTable->list) ) {
        pattern_t* tmp = list_entry( pos, pattern_t, list );
        pattern_t* p =pattern_createCopy( tmp );
        p->usage =0;
        list_add( &(p->list), &(v->codeTable->list) );
        if( p->size == 1 )
            v->singleton = p;
    }

    // The code table has to be sorted descending by pattern size
    pattern_list_sortBySizeDesc( v->codeTable );

    // The encoded data is represented in a linked list
    v->encoded = (region_t*)malloc( sizeof( region_t ) );
    INIT_LIST_HEAD( &(v->encoded->list) );

    // Encode the matrix by running each code table pattern over the output buffer
    list_for_each( pos, &(v->codeTable->list) ) {
        pattern_t* p = list_entry( pos, pattern_t, list );
        pattern_bounds_t pb = pattern_computeBounds( p );
        
        //for( int i =0/*-pb.rowMin*/; i < m->height /*- pb.rowMax*/; i++ ) {
         //   for( int j =0/*-pb.colMin*/; j < vouw_matrix_rowLength( m, i ) /*- pb.colMax*/; j++ ) {
        for( int i = m->height -1; i >= 0; i-- ) {
            for( int j = vouw_matrix_rowLength( m, i ) -1; j>= 0; j-- ) {
                vouw_coord_t pivot = { i,j };
                int variant =0;
                if( pattern_isMatch( p, m, pivot, &variant ) ) {
                    createRegion( v, p, pivot, variant, true );
                    p->usage++;
                }
            }
        }

    }
    vouw_matrix_unflagAll( m );
    
    // Compute the encoding sizes for the data and the code table
    computeStdBits( v );
    updateEncodedLength( v );
    return v; 
}

void
vouw_free( vouw_t* v ) {
    region_list_free( v->encoded );
    pattern_list_free( v->codeTable );
    if( v->buffer )
        free( v->buffer );
    free( v );
}

static void
candidates_alloc( vouw_t* v ) {
    v->bufferIndex =0;
    if( !v->buffer ) {
        int maxOffsets = v->mat->count;
        maxOffsets *= maxOffsets;
        v->buffer = malloc( maxOffsets * sizeof( candidate_t ) );
    }
}

static void
candidates_add( vouw_t* v, pattern_t* p1, pattern_t* p2, pattern_offset_t offset, int variant ) {
    for( int i =0; i < v->bufferIndex; i++ ) {
        candidate_t* c = &(((candidate_t*)v->buffer)[i]);
        if( offset.row == c->row &&
            offset.col == c->col &&
            variant == c->variant &&
            p1 == c->p1 &&
            p2 == c->p2 ) {
            c->usage++;
            return;
        }   
    }
    candidate_t* c = &(((candidate_t*)v->buffer)[v->bufferIndex++]);
    c->p1 = p1;
    c->p2 = p2;
    c->variant = variant;
    c->row = offset.row;
    c->col = offset.col;
    c->usage =1;
}

static candidate_t
candidates_index( vouw_t* v, int i ) {
    return ((candidate_t*)v->buffer)[i];
}

static int
candidates_count( vouw_t* v ) {
    return v->bufferIndex;
}

int
vouw_encode( vouw_t* v ) {
    int steps =0;
    while( vouw_encodeStep( v ) ) steps++;

    return steps;
}

int
vouw_encodeStep( vouw_t* v ) {
    // Label all the patterns so we can print them
    //pattern_list_setLabels( v->codeTable ); // ONLY FOR DEBUG
    
    candidates_alloc( v );

    // Sort the code table by usage, then size (descending order)
    //pattern_list_sortByUsageDesc( v->codeTable );

    pattern_t* p1 = NULL;
    pattern_t* p2 = NULL;
    pattern_t* bestP1 =NULL,* bestP2 = NULL;
    int base =v->mat->base;

    double bestGain =0.0;
    pattern_offset_t bestP2Offset;
    int bestUsage =0, bestVar =0;

    struct list_head *pos1, *pos2;
    list_for_each( pos1, &(v->encoded->list) ) {
        region_t* r1 = list_entry( pos1, region_t, list );
        if( r1->masked )
            continue;
        p1 = r1->pattern;
        // Make sure we don't visit these regions again
        r1->masked =true;
        list_for_each( pos2, &(v->encoded->list) ) { 
        //list_for_each( pos2, &(v->encoded->list) ) { 
            region_t* r2 = list_entry( pos2, region_t, list );

            if( r2->masked )
            //if( (r1->pivot.row == r2->pivot.row && r1->pivot.col == r2->pivot.col) )
                continue;

            p2 =r2->pattern;
            pattern_offset_t p2_offset = pattern_offset( r1->pivot, r2->pivot );
//            if( p2_offset.row < 0 && p2_offset.col < 0 ) continue;
            if( p2_offset.row > (p1->bounds.rowMax+1) 
                    || p2_offset.row < 0 
                    || p2_offset.col > (p1->bounds.colMax+1) 
                    || p2_offset.col < (-p1->bounds.colMin) -1 ) continue;
            //if( p2_offset.row > 5 || p2_offset.row < -5 || p2_offset.col > 5 || p2_offset.col < -5 ) continue;

            int variant = ((r2->variant + base) - r1->variant) % base;

            candidates_add( v, p1, p2, p2_offset, variant );

        }
    }
    region_list_unmask( v->encoded );
#ifdef VOUW_DEBUG_PRINT
    fprintf( stderr, "encoded_step(): number of candidates: %d\n", candidates_count( v ) );
#endif
    for( int i =0; i < candidates_count( v ); i++ ) {
        candidate_t c =candidates_index( v, i );

        double gain = computeGain( v, c.p1, c.p2, c.usage );
        if( gain >= bestGain ) {
            bestGain =gain;
            bestP2Offset.col= c.col;
            bestP2Offset.row= c.row;
            bestUsage =c.usage;
            bestP1 = c.p1;
            bestP2 = c.p2;
            bestVar = c.variant;
        }
    }
    if( bestGain == 0.0 ) {
#ifdef VOUW_DEBUG_PRINT
        printf( "vouw_step(): No compression gain.\n" );
#endif
        return false;
    }

#ifdef VOUW_DEBUG_PRINT
    fprintf( stderr, "vouw_step(): merging: %c and %c +%d with offset (%d,%d)\n", 
            'A' + bestP1->label, 'A' + bestP2->label, bestVar, bestP2Offset.row, bestP2Offset.col );
    fprintf( stderr,"vouw_step(): best usage: %d\n", bestUsage );
    fprintf( stderr,"vouw_step(): compression size gain: %f bits\n", bestGain );
#endif

    mergeEncodedPatterns( v, bestP1, bestP2, bestVar, bestP2Offset );

    updateEncodedLength( v );
    
    prunePattern( v, bestP1 );
    if( bestP1 != bestP2 )
        prunePattern( v, bestP2 );
    
    return true;
}

vouw_matrix_t*
vouw_decode( vouw_t* v ) {
    vouw_matrix_t* m = vouw_matrix_create( v->mat->width, v->mat->height, v->mat->base );
    struct list_head* pos;
    list_for_each( pos, &(v->encoded->list) ) {
        region_t* region = list_entry( pos, region_t, list );

        region_apply( region, m );
    }
    return m;
}

#ifdef __cplusplus
}
#endif
