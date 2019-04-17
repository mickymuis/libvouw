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

Instance::Instance( Pattern* pattern, const Coord2D& pivot, const Variant* variant ) : 
    m_pivot( pivot ),
    m_variant( variant )/*,
    m_marker( -1 )*/
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

/* class InstanceVector implementation */

InstanceVector::InstanceVector( int matWidth, int matHeight, int matBase) : 
    std::vector<Instance>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 0.0 ) {
    setMatrixSize( matWidth, matHeight, matBase );
}

InstanceVector::InstanceVector( const Matrix2D* mat ) : 
    std::vector<Instance>(), 
    m_bits( 0.0 ), m_stdBitsPerPivot( 0.0 ) {
    setMatrixSize( mat->width(), mat->height(), mat->base() );
}

InstanceVector::~InstanceVector() {}

double 
InstanceVector::bitsPerPivot( std::size_t pivotCount ) {
    return log2( (double) pivotCount );
}

void InstanceVector::updateCodeLengths( int modelSize ) {
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

void InstanceVector::clearBitmasks() {
    for( auto& r : *this ) {
    //    r.m_bitmask.assign( r.m_bitmask.size(), false );
    }
}
/*void InstanceVector::unflagAll() {
    for( auto& r : *this ) {
        r->m_flagged = false;
    }
}*/

void 
InstanceVector::setMatrixSize( int width, int height, int base ) { 
    m_width =width; m_height =height; m_base =base;
    m_nodeCount =width*height;

    /*if( width>0 && height>0 ) {
        m_stdBitsPerPivot = log2( (double)m_nodeCount );
    }*/

}

VOUW_NAMESPACE_END

