/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/region.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>
#include <vouw/equivalence.h>
#include <cmath>
#include <cstdio>

VOUW_NAMESPACE_BEGIN

/* class Region implementation */

Region::Region( Pattern* pattern, const Coord2D& pivot, const Variant* variant, bool flag ) : 
    m_pattern( pattern ),
    m_pivot( pivot ),
    m_variant( variant ),
    //m_mask( masked ? DirAll : DirNone )
    m_flagged( flag )
{
}

Region::Region() : m_variant(0) {}
Region::~Region() {} 

void
Region::apply( Matrix2D* mat ) {

}

/*void 
Region::setFlagged( bool b, DirT dir ) const { 
    if( b )
        m_mask |= dir;
    else
        m_mask &= ~dir;
}*/

bool operator<(const Region& r1, const Region& r2) {
    return r1.pivot().position() < r2.pivot().position();
}

/* class RegionList implementation */

RegionList::RegionList( int matWidth, int matHeight, int matBase) : 
    std::vector<Region>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 1.0 ) {
    setMatrixSize( matWidth, matHeight, matBase );
}

RegionList::RegionList( const Matrix2D* mat ) : 
    std::vector<Region>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 1.0 ) {
    setMatrixSize( mat->width(), mat->height(), mat->base() );
}

RegionList::~RegionList() {}

double 
RegionList::bitsPerPivot( std::size_t pivotCount ) {
    return log2( (double) pivotCount );
}

void RegionList::updateCodeLengths() {

  //  m_stdBitsPerPivot = bitsPerPivot( this->size() );
    m_bits =uintCodeLength( this->size() );

    for( auto& r : *this ) {
        Pattern* p =r.pattern();
        m_bits += p->codeLength();
        r.m_variantBits = log2( (double)p->usage() ); // TODO fix
        //m_bits += r.variantBits();
    }
    m_bits += this->size() * m_stdBitsPerPivot;
}

void RegionList::clearBitmasks() {
    for( auto& r : *this ) {
        r.m_bitmask.clear();
    }
}
void RegionList::unflagAll() {
    for( auto& r : *this ) {
        r.m_flagged = false;
    }
}

void 
RegionList::setMatrixSize( int width, int height, int base ) { 
    m_width =width; m_height =height; m_base =base;
    m_nodeCount =width*height;

    /*if( width>0 && height>0 ) {
        m_stdBitsPerPivot = log2( (double)m_nodeCount );
    }*/

}

/** Decomposes region @r into regions r1 and r2, defined by the composition on @r.pattern()
 *  If r1 and/or r2 contain an inactive pattern, each is decomposed recursively.
 *  The final regions are covering @r completely and are pushed to the back of the list.
 *  @r is not removed from the list and this function leaves the list in an unsorted state.
 */
void
RegionList::decompose( const Region& r ) {
    const Pattern* p =r.pattern();
    const Pattern::CompositionT& comp = r.pattern()->composition();
    if( !comp.isValid() ) return;

    Coord2D pivot2 = comp.offset.abs( r.pivot() );

    Region r1( (Pattern*)comp.p1, r.pivot(), comp.v1 );
    if( !comp.p1->isActive() )
        decompose( r1 );
    else {
        ((Pattern*)comp.p1)->usage()++;
        push_back( r1 );
    }

    Region r2( (Pattern*)comp.p2, pivot2, comp.v2 );
    if( !comp.p2->isActive() )
        decompose( r2 );
    else {
        ((Pattern*)comp.p2)->usage()++;
        push_back( r2 );
    }
}

VOUW_NAMESPACE_END

