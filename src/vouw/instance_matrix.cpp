/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include "vouw/instance_matrix.h"
#include <stdexcept>

VOUW_NAMESPACE_BEGIN

InstanceMatrix::IndexT InstanceMatrix::empty = -1;

InstanceMatrix::InstanceMatrix() 
    : m_rowLength(0) {
}

InstanceMatrix::InstanceMatrix( int rowLength ) 
    : m_rowLength( rowLength ) {
}

InstanceMatrix::~InstanceMatrix() {}

InstanceMatrix::IndexT 
InstanceMatrix::at( const Coord2D& c ) {
    if( c.col() >= m_rowLength || c.col() < 0 ) return empty;
    MapT::iterator it = m_map.find( key( c ) );
    if( it == m_map.end() ) return empty;
    return it->second;
}

InstanceMatrix::IndexT 
InstanceMatrix::at( int row, int col ) {
    if( col >= m_rowLength || col < 0 ) return empty;
    MapT::iterator it = m_map.find( key( row, col ) );
    if( it == m_map.end() ) return empty;
    return it->second;
}

InstanceMatrix::IndexT 
InstanceMatrix::operator[]( const Coord2D& c ) {
    return at( c );
}

const InstanceMatrix::IndexT 
InstanceMatrix::operator[]( const Coord2D& c ) const {
    return (const IndexT)((InstanceMatrix*)this)->at(c);
}

void 
InstanceMatrix::place( IndexT idx, const Instance& inst ) {
    place( idx, inst, inst.pivot() );
}

void 
InstanceMatrix::place( IndexT idx, const Instance& inst, const Coord2D& pivot ) {
    Pattern *p = inst.pattern();
    for( auto && elem : p->elements() ) {
        Coord2D c = elem.offset.abs( pivot );
        m_map[key(c)] = idx;
    }
}

void 
InstanceMatrix::remove( const Instance& inst ) {
    Pattern *p = inst.pattern();
    for( auto && elem : p->elements() ) {
        Coord2D c = elem.offset.abs( inst.pivot() );
        m_map.erase( key(c) );
    }
    //m_count--;
}

InstanceMatrix::KeyT
InstanceMatrix::key( const Coord2D& c ) const {
    return key( c.row(), c.col() );
}

InstanceMatrix::KeyT
InstanceMatrix::key( int row, int col ) const {
    return m_rowLength*row + col;
}

void
InstanceMatrix::clear() {
    m_map.clear();
    //m_count =0;
}

VOUW_NAMESPACE_END
