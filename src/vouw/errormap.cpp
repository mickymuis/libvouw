/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include <vouw/errormap.h>
#include <vouw/vouw.h>
#include <cmath>
#include <cassert>

VOUW_NAMESPACE_BEGIN

double 
errorCodeLength( unsigned int mwidth, unsigned int mheight, int uniqueElems ) {
    return log2( mwidth*mheight) + log2( uniqueElems );
}

int 
errorCount( const Pattern& p1, const Pattern& p2 ) {
    if( p1.configuration() != p2.configuration() ) return -1;
    if( p1.size() != p2.size() ) return -1;

    int error =0;

    for( int i =0; i < p1.size(); i++ ) {
        const Pattern::ElementT e1 = p1.elements()[i];
        const Pattern::ElementT e2 = p2.elements()[i];

        //if( e1.offset != e2.offset ) return -1;
        assert( e1.offset == e2.offset );
        if( e1.value != e2.value ) error++;
    }
    return error;
}

int 
errorMapDelta( ErrorMapT& map, const Pattern& from, const Pattern& to, const Coord2D& pivot ) {
    if( from.configuration() != to.configuration() ) return -1;
    if( from.size() != to.size() ) return -1;

    int error =0;

    for( int i =0; i < from.size(); i++ ) {
        const Pattern::ElementT e1 = from.elements()[i];
        const Pattern::ElementT e2 = to.elements()[i];

        //if( e1.offset != e2.offset ) return -1;
        assert( e1.offset == e2.offset );
        if( e1.value != e2.value ) {
            error++;
            Coord2D c =e2.offset.abs( pivot );
            map[c] = e2.value;
        }
    }
    return error;

}


VOUW_NAMESPACE_END
