/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/codetable.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>
#include <math.h>

VOUW_NAMESPACE_BEGIN

CodeTable::CodeTable( int matWidth, int matHeight, int matBase) : 
    std::list<Pattern*>(), 
    m_bits( 0.0 ) {
    setMatrixSize( matWidth, matHeight, matBase );
}

CodeTable::CodeTable( const Matrix2D* mat ) : 
    std::list<Pattern*>(), 
    m_bits( 0.0 ) {
    setMatrixSize( mat->width(), mat->height(), mat->base() );
}

CodeTable::~CodeTable() {
    for( auto&& p : *this ) delete p;
}

void CodeTable::updateCodeLengths( int totalInstances ) {

    m_bits =0.0;

    for( auto p : *this ) {
        if( p->isActive() ) {
            m_bits += p->updateCodeLength( totalInstances );
            m_bits += p->size() * p->updateBitsPerOffset( m_base );
        } else
            p->updateCodeLength( totalInstances );
        //m_bits += p->size() * m_stdBitsPerOffset;
    }
}

void CodeTable::sortBySizeDesc() {

}

void 
CodeTable::setMatrixSize( int width, int height, int base ) { 
    m_width =width; m_height =height; m_base =base;
    m_nodeCount =width*height;

    /*if( width>0 && height>0 ) {
        m_stdBitsPerOffset = log2( (double)m_nodeCount ) + log2( (double)m_base );
    }*/
}
VOUW_NAMESPACE_END

