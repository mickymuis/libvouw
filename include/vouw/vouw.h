/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#ifndef VOUW_H
#define VOUW_H

#ifdef __cplusplus
extern "C" {
#endif

#define VOUW_DEBUG_PRINT

#include "matrix.h"
#include "pattern.h"
#include "region.h"

typedef struct {
    region_t* encoded;
    pattern_t* codeTable;
    pattern_t* singleton;
    vouw_matrix_t *mat;
    double encodedBits;
    double ctBits;
    double stdBitsPerOffset;
    double stdBitsPerPivot;
    double stdBitsPerVariant;
    void* buffer;
    uint64_t bufferIndex;
} vouw_t;

vouw_t*
vouw_createFrom( vouw_matrix_t* m );

vouw_t*
vouw_createEncodedUsing( vouw_matrix_t* m, pattern_t* codeTable );

void
vouw_free( vouw_t* v );

int
vouw_test( vouw_t* v );

int
vouw_encode( vouw_t* v );

int
vouw_encodeStep( vouw_t* v );

vouw_matrix_t*
vouw_decode( vouw_t* v );

#ifdef __cplusplus
}
#endif

#endif
