/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#include <vouw/matrix.h>
#include <algorithm>

VOUW_NAMESPACE_BEGIN

/* class Coord2D implementation */

int Coord2D::position() const {
    return rowLength()*row() + col();
}

bool operator==( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.row() == c2.row() && c1.col() == c2.col();
}

bool operator!=( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.row() != c2.row() || c1.col() != c2.col();
}

bool operator<=( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.position() <= c2.position();
}

bool operator<( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.position() < c2.position();
}

bool operator>=( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.position() >= c2.position();
}

bool operator>( const Coord2D& c1, const Coord2D& c2 ) {
    return c1.position() > c2.position();
}

/* class Matrix2D implementation */

Matrix2D::Matrix2D( unsigned int width, unsigned int height, unsigned int base ) :
    m_width( width ),
    m_height( height ),
    m_base( base ) {
    m_buffer = new ElementT[width*height];
}

Matrix2D::Matrix2D( const Matrix2D& mat ) : 
    m_width( mat.width() ),
    m_height( mat.height() ),
    m_base( mat.m_base ) {
    m_buffer = new ElementT[width()*height()];
    std::copy( mat.data(), mat.data() + mat.count(), data()); 
}

Matrix2D::~Matrix2D() {
    delete data();
}

Coord2D 
Matrix2D::makeCoord( int row, int col ) {
    return Coord2D( row, col, width() );
}

void 
Matrix2D::clear() {
    std::fill( data(), data() + count(), 0 );
}

Matrix2D::ElementT* 
Matrix2D::data() {
    return m_buffer;
}

const Matrix2D::ElementT* 
Matrix2D::data() const {
    return m_buffer;
}

Matrix2D::ElementT* 
Matrix2D::rowPtr( unsigned int row ) {
    return data() + row*width();
}

const Matrix2D::ElementT* 
Matrix2D::rowPtr( unsigned int row ) const {
    return data() + row*width();
}

/*Matrix2D::ElementT& Matrix2D::value( Coord2D c ) {
    return data()[c.col() + c.row() * c.rowLength()] & ~VOUW_UNODE32_FLAGGED;
}*/

Matrix2D::ElementT Matrix2D::value( Coord2D c ) const {
    return data()[c.col() + c.row() * width()] & ~VOUW_UNODE32_FLAGGED;
}

void Matrix2D::setValue( Coord2D c, const ElementT& e ) {
    bool flag =isFlagged( c );
    data()[c.col() + c.row() * width()] = (e & ~VOUW_UNODE32_FLAGGED) | (flag ? VOUW_UNODE32_FLAGGED : 0);
}

void Matrix2D::setFlagged( const Coord2D& c, bool flag ) {
    if( flag )
        data()[c.col() + c.row() * width()] |= VOUW_UNODE32_FLAGGED;
    else
        data()[c.col() + c.row() * width()] &= ~VOUW_UNODE32_FLAGGED;
}

bool Matrix2D::isFlagged( const Coord2D& c ) const {
    return data()[c.col() + c.row() * width()] & VOUW_UNODE32_FLAGGED;
}

void Matrix2D::unflagAll() {
    unsigned int i =0;
    while( i < count() ) {
        data()[i++] &= ~VOUW_UNODE32_FLAGGED;
    }
}

bool Matrix2D::checkBounds( const Coord2D& c ) const {
    if( c.row() >= height() ) return false;
    if( c.col() >= width() ) return false;
    return true;
}

bool Matrix2D::operator==( const Matrix2D& mat ) {
    if( !(mat.width() == width() && mat.height() == height() && mat.base() == base() ) )
        return false;
    return std::equal( data(), data() + count(), mat.data() );
}

VOUW_NAMESPACE_END

#if 0
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

//#ifdef __cplusplus
}
#endif
