/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#ifndef VOUW_MATRIX_H
#define VOUW_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define VOUW_UNODE32_FLAGGED (1U << 31)

/** Pricipal element type for VOUW input matrices. 
 * The 32nd bit is reserved for internal use. */
typedef uint32_t vouw_unode32_t;

typedef struct {
    int row;
    int col;
} vouw_coord_t;

typedef struct {
    vouw_unode32_t* cols;
    int size;

} vouw_row_t;

/** Dense, rectangular matrix of postive integers.
 * The @base field denotes the number of possible values for each element. */
typedef struct {
    int base;                   // Largest element value is base-1
    int height;                 // Number of rows
    int width;                  // Number of columns
    int count;                  // Total number of elements
    vouw_row_t* rows;           // Array of pointers to each row
    vouw_unode32_t* buffer;     // Pointer to storage buffer
} vouw_matrix_t;

vouw_matrix_t*
vouw_matrix_create( int width, int height, int base );

void
vouw_matrix_free( vouw_matrix_t* m );

void
vouw_matrix_clear( vouw_matrix_t* m );

vouw_unode32_t 
vouw_matrix_value( const vouw_matrix_t* m, vouw_coord_t c );

void
vouw_matrix_setValue( vouw_matrix_t* m, vouw_coord_t c, vouw_unode32_t value );

int 
vouw_matrix_rowLength( const vouw_matrix_t* m, int row );

bool
vouw_matrix_checkBounds( const vouw_matrix_t* m, vouw_coord_t c );

bool
vouw_matrix_isEqual( const vouw_matrix_t* m1, const vouw_matrix_t* m2 );

void
vouw_matrix_setFlagged( vouw_matrix_t* m, vouw_coord_t c, bool mask );

bool
vouw_matrix_isFlagged( const vouw_matrix_t* m, vouw_coord_t c );

void
vouw_matrix_unflagAll( vouw_matrix_t* m );

#ifdef __cplusplus
}
#endif

#endif
