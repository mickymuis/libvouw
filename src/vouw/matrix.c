/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <vouw/matrix.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void
setRowPtrs( vouw_matrix_t* m ) {
    for( int i =0; i < m->height; i++ ) {
        m->rows[i].cols = m->buffer + i*m->width;
        m->rows[i].size = m->width;
    }
}

vouw_matrix_t*
vouw_matrix_create( int width, int height, int base ) {
    vouw_matrix_t* m = (vouw_matrix_t*)malloc( sizeof( vouw_matrix_t ) );
    
    m->width = width;
    m->height = height;
    m->base = base;
    m->count = width*height;
    m->rows = (vouw_row_t*)malloc( height * sizeof( vouw_row_t ) );
    m->buffer = (vouw_unode32_t*)malloc( m->count * sizeof( vouw_unode32_t ) );

    setRowPtrs( m );

    return m;
}

void
vouw_matrix_free( vouw_matrix_t* m ) {
    /*for( int i =0; i < m->height; i++ ) {
        free( m->rows[i].cols );
    }*/
    free( m->buffer );
    free( m->rows );
    free( m );
}

/**
 * Clears the matrix object @m by writing 0 to each element.
 */
void
vouw_matrix_clear( vouw_matrix_t* m ) {
    for( int i =0; i < m->height; i++ ) {
        memset( m->rows[i].cols, 0, m->rows[i].size );
    }
}

/**
 * Returns the value of the element at coordinate @c
 */
vouw_unode32_t 
vouw_matrix_value( const vouw_matrix_t* m, vouw_coord_t c ) {
    assert( c.col >= 0 && c.col < m->rows[c.row].size );
    assert( c.row >= 0 && c.row < m->height );
    return m->rows[c.row].cols[c.col];
}

/**
 * Changes the value on coordinate @c to @value
 */
void
vouw_matrix_setValue( vouw_matrix_t* m, vouw_coord_t c, vouw_unode32_t value ) {
    assert( c.col >= 0 && c.col < m->rows[c.row].size );
    assert( c.row >= 0 && c.row < m->height );
    m->rows[c.row].cols[c.col] = value;
}

/*
 * Returns the number of columns in row `row'
 */
int 
vouw_matrix_rowLength( const vouw_matrix_t* m, int row ) {
    if( row >= m->height )
        return 0;
    return m->rows[row].size;
}

/*
 * Return true if the coordinate c is within bounds of b
 */
bool
vouw_matrix_checkBounds( const vouw_matrix_t* m, vouw_coord_t c ) {
    return ( c.row >= 0 && c.row < m->height 
             && c.col >= 0 && c.col < m->rows[c.row].size );
}

/*
 * Returns true if b1 and b2 are equally sized and have equal values
 */
bool
vouw_matrix_isEqual( const vouw_matrix_t* m1, const vouw_matrix_t* m2 ) {
    if(  m1->height != m2->height ) return false;
    for( int i =0; i < m1->height; i++ ) {
        if( m1->rows[i].size != m2->rows[i].size ) return false;
        if( memcmp( m1->rows[i].cols, m2->rows[i].cols, m1->rows[i].size * sizeof( vouw_unode32_t ) ) != 0 ) 
            return false;
    }
    return true;
}


void
vouw_matrix_setFlagged( vouw_matrix_t* m, vouw_coord_t c, bool flag ) {
    assert( c.col >= 0 && c.col < m->rows[c.row].size );
    if( flag )
        m->rows[c.row].cols[c.col] |= VOUW_UNODE32_FLAGGED;
    else
        m->rows[c.row].cols[c.col] &= ~VOUW_UNODE32_FLAGGED;
}

bool
vouw_matrix_isFlagged( const vouw_matrix_t* m, vouw_coord_t c ) {
    assert( c.col >= 0 && c.col < m->rows[c.row].size );
    return m->rows[c.row].cols[c.col] & VOUW_UNODE32_FLAGGED;
}

void
vouw_matrix_unflagAll( vouw_matrix_t* m ) {
    for( int i =0; i < m->height; i++ ) {
        for( int j =0; j < vouw_matrix_rowLength( m, i ); j++ ) {
            m->rows[i].cols[j] &= ~VOUW_UNODE32_FLAGGED;
        }
    }
}

#ifdef __cplusplus
}
#endif
