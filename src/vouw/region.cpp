/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/region.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>
#include <cmath>

VOUW_NAMESPACE_BEGIN

/* class Region implementation */

Region::Region( Pattern* pattern, const Coord2D& pivot, const Variant& variant, bool masked ) : 
    m_pattern( pattern ),
    m_pivot( pivot ),
    m_variant( variant ),
    m_mask( masked ? DirAll : DirNone )
{
}

Region::Region() : m_variant(true) {}
Region::~Region() {} 

void
Region::apply( Matrix2D* mat ) {

}

void 
Region::setMasked( bool b, DirT dir ) const { 
    if( b )
        m_mask |= dir;
    else
        m_mask &= ~dir;
}

bool operator<(const Region& r1, const Region& r2) {
    return r1.pivot().position() < r2.pivot().position();
}

/* class RegionList implementation */

RegionList::RegionList( int matWidth, int matHeight, int matBase) : 
    std::vector<Region>(), 
    m_bits( 0.0 ) {
    setMatrixSize( matWidth, matHeight, matBase );
}

RegionList::RegionList( const Matrix2D* mat ) : 
    std::vector<Region>(), 
    m_bits( 0.0 ) {
    setMatrixSize( mat->width(), mat->height(), mat->base() );
}

RegionList::~RegionList() {}

double 
RegionList::bitsPerPivot( std::size_t pivotCount ) {
    return log2( (double) pivotCount );
}

void RegionList::updateCodeLengths() {

  //  m_stdBitsPerPivot = bitsPerPivot( this->size() );
    m_bits =0.0;

    for( auto& r : *this ) {
        Pattern* p =r.pattern();
        m_bits += p->codeLength();
        r.m_variantBits = log2( (double)p->usage() ); // TODO fix
        //m_bits += r.variantBits();
    }
    m_bits += this->size() * m_stdBitsPerPivot;
}

void RegionList::unmaskAll() {
    for( auto& r : *this ) {
        r.m_mask = DirNone;
    }
}

void 
RegionList::setMatrixSize( int width, int height, int base ) { 
    m_width =width; m_height =height; m_base =base;
    m_nodeCount =width*height;

    if( width>0 && height>0 ) {
        m_stdBitsPerPivot = log2( (double)m_nodeCount );
    }
}

VOUW_NAMESPACE_END

#if 0
//#ifdef __cplusplus
extern "C" {
//#endif

#include <vouw/region.h>
#include <stdlib.h>

region_t*
region_create( pattern_t* pattern, vouw_coord_t pivot ) {
    region_t* r = (region_t*)malloc( sizeof( region_t ) );
    r->pivot =pivot;
    r->pattern = pattern;
    r->masked = false;
    INIT_LIST_HEAD( &(r->list ) );
    return r;
}

void
region_apply( const region_t* region, vouw_matrix_t* m ) {
    pattern_t* p =region->pattern;
    for( int i =0; i < p->size; i++ ) {
        // For each offset, compute its location on the automaton
        vouw_coord_t c = pattern_offset_abs( region->pivot, p->offsets[i] );
        // Set the buffer's value at c
        vouw_matrix_setValue( m, c, (p->offsets[i].value + region->variant) % m->base );
    }
}

void
region_free( region_t* r ) {
    free( r );
}

void
region_list_free( region_t* r ) {
    struct list_head* tmp,* pos;
    list_for_each_safe( pos, tmp, &(r->list) ) {
        region_t* entry = list_entry( pos, region_t, list );
        list_del( pos );
        
        region_free( entry );
    }
    free( r );
}

void 
region_list_unmask( region_t* r ) {
    struct list_head* pos;
    list_for_each( pos, &(r->list) ) {
        region_t* entry = list_entry( pos, region_t, list );
        entry->masked =false;
    }
}

//#ifdef __cplusplus
}
#endif
