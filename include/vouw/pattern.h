/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include <vouw/matrix.h>
#include <stdbool.h>

typedef struct {
    int row;
    int col;
    vouw_unode32_t value;
} pattern_offset_t;

vouw_coord_t
pattern_offset_abs( vouw_coord_t c, pattern_offset_t offs );

pattern_offset_t
pattern_offset( vouw_coord_t pivot1, vouw_coord_t pivot2 );

typedef struct {
    int rowMin;
    int rowMax;
    int colMin;
    int colMax;
    int width, height;
} pattern_bounds_t;

typedef struct {
    struct list_head list;
    pattern_offset_t* offsets;
    unsigned int usage;
    unsigned int size;
    double codeLength;
    char label; // for debug printingi
    pattern_bounds_t bounds;
} pattern_t;


pattern_t*
pattern_createSingle( int value );

pattern_t*
pattern_createUnion( const pattern_t* p1, const pattern_t* p2, pattern_offset_t p2_offset );

pattern_t*
pattern_createVariantUnion( const pattern_t* p1, const pattern_t* p2, int variant, pattern_offset_t p2_offset, int base );

pattern_t*
pattern_createCopy( const pattern_t* src );

void
pattern_free( pattern_t* p );

void
pattern_updateCodeLength( pattern_t* p, unsigned int totalNodeCount );

double 
pattern_computeCodeLength( const pattern_t* p, unsigned int totalNodeCount );

bool
pattern_isMatch( const pattern_t* p, const vouw_matrix_t* m, vouw_coord_t pivot, int* variant );

pattern_bounds_t
pattern_computeBounds( const pattern_t* p );

void
pattern_setMatrixValues( const pattern_t* p, vouw_coord_t pivot, vouw_matrix_t* m, vouw_unode32_t value );

void
pattern_list_free( pattern_t* list );

void 
pattern_list_setLabels( pattern_t* list );

double
pattern_list_updateCodeLength( pattern_t* list, unsigned int totalNodeCount );

void
pattern_list_sortByUsageDesc( pattern_t* head );

void
pattern_list_sortBySizeDesc( pattern_t* head );

#ifdef __cplusplus
}
#endif

#endif
