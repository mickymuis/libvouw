/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#pragma once

#define VOUW_NAMESPACE_BEGIN namespace Vouw {
#define VOUW_NAMESPACE_END };

VOUW_NAMESPACE_BEGIN

enum DirT {
    DirNone =0,
    DirHoriz =1,
    DirVert =1<<1,
    DirDiagL = 1<<2,
    DirDiagR = 1<<3,
    DirAll = (1<<4)-1
};

VOUW_NAMESPACE_END

#if 0
//#ifndef VOUW_H
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
