/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/codetable.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>
#include <vouw/massfunction.h>
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

void CodeTable::updateCodeLengths( int totalInstances, const MassFunction& distr ) {

    int ctSize =countIfActive();

    double ct_bits =uintCodeLength( ctSize );
    double inst_bits = uintCodeLength( m_width )
                     + uintCodeLength( m_height )
                     + log2( m_width * m_height )
                     ;//+ uintCodeLength( binom( m_width * m_height, totalInstances ) );

    inst_bits += lgamma( (double)totalInstances + pseudoCount * (double)ctSize ) / log(2)
               - lgamma( pseudoCount * (double)ctSize ) / log(2); 

    for( auto p : *this ) {
        if( p->isActive() ) {
            inst_bits += p->updateCodeLength( totalInstances, ctSize );
            if( p->entryLength() != 0.0 )
                ct_bits += p->entryLength();
            else
                ct_bits += p->updateEntryLength( distr );
        } else
            p->updateCodeLength( totalInstances, ctSize );
        //m_bits += p->size() * m_stdBitsPerOffset;
    }

    m_bits =ct_bits + inst_bits;

    fprintf( stderr, "L(H) = %f, L(D|H) = %f\n", ct_bits, inst_bits );
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

