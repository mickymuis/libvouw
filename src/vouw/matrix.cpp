/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#include <vouw/matrix.h>
#include <vouw/massfunction.h>
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
    m_base( base ),
    m_massfunc( NULL ) {
    m_buffer = new ElementT[width*height];
}

Matrix2D::Matrix2D( const Matrix2D& mat ) : 
    m_width( mat.width() ),
    m_height( mat.height() ),
    m_base( mat.m_base ), 
    m_massfunc( NULL ) {
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

Matrix2D::ElementT 
Matrix2D::value( Coord2D c ) const {
    return data()[c.col() + c.row() * width()] & ~VOUW_UNODE32_FLAGGED;
}

void 
Matrix2D::setValue( Coord2D c, const ElementT& e ) {
    bool flag =isFlagged( c );
    data()[c.col() + c.row() * width()] = (e & ~VOUW_UNODE32_FLAGGED) | (flag ? VOUW_UNODE32_FLAGGED : 0);
}

void 
Matrix2D::setFlagged( const Coord2D& c, bool flag ) {
    if( flag )
        data()[c.col() + c.row() * width()] |= VOUW_UNODE32_FLAGGED;
    else
        data()[c.col() + c.row() * width()] &= ~VOUW_UNODE32_FLAGGED;
}

bool
Matrix2D::isFlagged( const Coord2D& c ) const {
    return data()[c.col() + c.row() * width()] & VOUW_UNODE32_FLAGGED;
}

void
Matrix2D::unflagAll() {
    unsigned int i =0;
    while( i < count() ) {
        data()[i++] &= ~VOUW_UNODE32_FLAGGED;
    }
}

bool
Matrix2D::checkBounds( const Coord2D& c ) const {
    if( c.row() < 0 || c.row() >= height() ) return false;
    if( c.col() < 0 || c.col() >= width() ) return false;
    return true;
}

bool 
Matrix2D::operator==( const Matrix2D& mat ) {
    if( !(mat.width() == width() && mat.height() == height() && mat.base() == base() ) )
        return false;
    return std::equal( data(), data() + count(), mat.data() );
}

const MassFunction&
Matrix2D::distribution( bool force_regenerate ) {
    // (re-)generate the distribution
    if( !m_massfunc || force_regenerate ) {
        if( m_massfunc ) m_massfunc->clear();
        else m_massfunc = new MassFunction();
        unsigned int i =0;
        while( i < count() ) {
            m_massfunc->increment( data()[i++] & ~VOUW_UNODE32_FLAGGED );
        }
    }
    return *m_massfunc;
}

VOUW_NAMESPACE_END

