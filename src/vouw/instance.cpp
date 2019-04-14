/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include <vouw/instance.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>
#include <vouw/equivalence.h>
#include <cmath>
#include <cstdio>

VOUW_NAMESPACE_BEGIN

/* class Instance implementation */

Instance::Instance( Pattern* pattern, const Coord2D& pivot, const Variant* variant, bool flag ) : 
    m_pivot( pivot ),
    m_variant( variant ),
    //m_mask( masked ? DirAll : DirNone )
    m_flagged( flag ),
    m_marker( -1 )
{
    setPattern( pattern );
}

Instance::Instance() : m_variant(0) {}
Instance::~Instance() {} 

void
Instance::apply( Matrix2D* mat ) {

}

void
Instance::setPattern( Pattern* p ) {
    m_pattern =p;
   /* if( p ) {
        bitmaskGrow( p->periphery( Pattern::PosteriorPeriphery ).size() );
    }*/
}

/*void 
Instance::setFlagged( bool b, DirT dir ) const { 
    if( b )
        m_mask |= dir;
    else
        m_mask &= ~dir;
}*/

bool operator<(const Instance& r1, const Instance& r2) {
    return r1.pivot().position() < r2.pivot().position();
}

/* class InstanceList implementation */

InstanceList::InstanceList( int matWidth, int matHeight, int matBase) : 
    std::vector<Instance*>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 0.0 ), m_tabuCount( 0 ) {
    setMatrixSize( matWidth, matHeight, matBase );
}

InstanceList::InstanceList( const Matrix2D* mat ) : 
    std::vector<Instance*>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 0.0 ), m_tabuCount( 0 ) {
    setMatrixSize( mat->width(), mat->height(), mat->base() );
}

InstanceList::~InstanceList() {}

double 
InstanceList::bitsPerPivot( std::size_t pivotCount ) {
    return log2( (double) pivotCount );
}

void InstanceList::updateCodeLengths( int modelSize ) {
/*
  //  m_stdBitsPerPivot = bitsPerPivot( this->size() );
    m_bits =uintCodeLength( this->size() );

    for( auto& r : *this ) {
        Pattern* p =r.pattern();
        //m_bits += p->codeLength();
        r.m_variantBits = log2( (double)p->usage() ); // TODO fix
        //m_bits += r.variantBits();
    }
    m_bits += lgamma( (double)size() + pseudoCount * (double)modelSize ) / log(2)
            - lgamma( pseudoCount * (double)modelSize ) / log(2); */
}

void InstanceList::clearBitmasks() {
    for( auto& r : *this ) {
        r->m_bitmask.assign( r->m_bitmask.size(), false );
    }
}
void InstanceList::unflagAll() {
    for( auto& r : *this ) {
        r->m_flagged = false;
    }
}

void 
InstanceList::setMatrixSize( int width, int height, int base ) { 
    m_width =width; m_height =height; m_base =base;
    m_nodeCount =width*height;

    /*if( width>0 && height>0 ) {
        m_stdBitsPerPivot = log2( (double)m_nodeCount );
    }*/

}

/** Decomposes instance @r into instances r1 and r2, defined by the composition on @r.pattern()
 *  If r1 and/or r2 contain an inactive pattern, each is decomposed recursively.
 *  The final instances are covering @r completely and are pushed to the back of the list.
 *  @r is not removed from the list and this function leaves the list in an unsorted state.
 */
void
InstanceList::decompose( const Instance& r ) {
    const Pattern* p =r.pattern();
    const Pattern::CompositionT& comp = r.pattern()->composition();
    if( !comp.isValid() ) return;

    Coord2D pivot2 = comp.offset.abs( r.pivot() );

    Instance r1( (Pattern*)comp.p1, r.pivot(), comp.v1 );
    if( !comp.p1->isActive() )
        decompose( r1 );
    else {
        ((Pattern*)comp.p1)->usage()++;
        push_back( new Instance(r1) );
    }

    Instance r2( (Pattern*)comp.p2, pivot2, comp.v2 );
    if( !comp.p2->isActive() )
        decompose( r2 );
    else {
        ((Pattern*)comp.p2)->usage()++;
        push_back( new Instance(r2) );
    }
}

void
InstanceList::deleteAll() { 
    for( Instance* inst : *this ) { 
        delete inst;
    }
    clear();
}

VOUW_NAMESPACE_END

