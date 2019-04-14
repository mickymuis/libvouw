/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include "vouw/instance_matrix.h"
#include <stdexcept>

VOUW_NAMESPACE_BEGIN

InstanceMatrix::InstanceMatrix() 
    : m_rowLength(0)/*, m_count(0)*/ {
}

InstanceMatrix::InstanceMatrix( int rowLength ) 
    : m_rowLength( rowLength )/*, m_count(0)*/ {
}

InstanceMatrix::~InstanceMatrix() {}

Instance* 
InstanceMatrix::at( const Coord2D& c ) {
    if( c.col() >= m_rowLength || c.col() < 0 ) return NULL;
    MapT::iterator it = m_map.find( key( c ) );
    if( it == m_map.end() ) return NULL;
    return it->second;
}

Instance* 
InstanceMatrix::at( const Coord2D& c, bool& isPivot ) {
    if( c.col() >= m_rowLength || c.col() < 0 ) return NULL;
    MapT::iterator it = m_map.find( key( c ) );
    if( it == m_map.end() ) return NULL;
    isPivot = (c == (it->second)->pivot());
    return it->second;
}

Instance* 
InstanceMatrix::at( int row, int col ) {
    if( col >= m_rowLength || col < 0 ) return NULL;
    MapT::iterator it = m_map.find( key( row, col ) );
    if( it == m_map.end() ) return NULL;
    return it->second;
}

Instance* 
InstanceMatrix::at( int row, int col, bool& isPivot ) {
    if( col >= m_rowLength || col < 0 ) return NULL;
    MapT::iterator it = m_map.find( key( row, col ) );
    if( it == m_map.end() ) return NULL;
    isPivot = (row == (it->second)->pivot().row() && col == (it->second)->pivot().col());
    return it->second;
}

Instance* 
InstanceMatrix::operator[]( const Coord2D& c ) {
    return at( c );
}

const Instance* 
InstanceMatrix::operator[]( const Coord2D& c ) const {
    return (const Instance*)((InstanceMatrix*)this)->at(c);
}

void 
InstanceMatrix::place( Instance* inst ) {
    place( inst, inst->pivot() );
}

void 
InstanceMatrix::place( Instance* inst, const Coord2D& pivot ) {
    inst->setPivot( pivot );
    Pattern *p = inst->pattern();
    for( auto && elem : p->elements() ) {
        Coord2D c = elem.offset.abs( pivot );
        m_map[key(c)] = inst;
    }
    //m_count++; // Well, this is not very precise and/or reliable
}

void 
InstanceMatrix::remove( Instance* inst ) {
    Pattern *p = inst->pattern();
    for( auto && elem : p->elements() ) {
        Coord2D c = elem.offset.abs( inst->pivot() );
        m_map.erase( key(c) );
    }
    //m_count--;
}

void 
InstanceMatrix::remove( const Coord2D& c ) {
    Instance* inst =at( c );
    if( inst ) remove( inst );
}

unsigned int 
InstanceMatrix::key( const Coord2D& c ) const {
    return key( c.row(), c.col() );
}

unsigned int
InstanceMatrix::key( int row, int col ) const {
    return m_rowLength*row + col;
}

void
InstanceMatrix::clear() {
    m_map.clear();
    //m_count =0;
}

VOUW_NAMESPACE_END
