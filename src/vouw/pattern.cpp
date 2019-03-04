/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/pattern.h>
#include <vouw/equivalence.h>
#include <vouw/massfunction.h>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstring>

VOUW_NAMESPACE_BEGIN

/* class Pattern::OffsetT implementation */

Pattern::OffsetT::OffsetT( int row, int col, int rowLength ) 
    : Coord2D( row, col, rowLength ) {}

Pattern::OffsetT::OffsetT( const Coord2D& pivot1, const Coord2D& pivot2 ) 
    : Coord2D( pivot2.row() - pivot1.row(), pivot2.col() - pivot1.col(), pivot1.rowLength() ) {}

Coord2D 
Pattern::OffsetT::abs( const Coord2D& c ) const {
    return Coord2D( c.row() + row(), c.col() + col(), c.rowLength() );
}

Pattern::OffsetT 
Pattern::OffsetT::translate( const OffsetT& o ) const {
    return OffsetT( o.row() + row(), o.col() + col(), rowLength() );
}

DirT
Pattern::OffsetT::direction() const { 
    if( !row() )
        return col() ? DirHoriz : DirNone;
        //return (DirT)!!col(); 
    else
        return !col() ? DirVert : (col()<0 ? DirDiagL : DirDiagR);
        //return (DirT)(2 << ((!!col()) << (col()<0)));
}

int
Pattern::OffsetT::position() const {
    return 2*(rowLength()-1)*row() + (rowLength()-1)+col();
}

/* class Pattern implementation */

Pattern::Pattern()
    : m_usage( 0 ),
    m_label( 0 ),
    m_codeBits( 0 ), m_entryOffsetsBits( 0 ), m_entryValuesBits( 0 ),
    m_active( true ), m_tabu( false ) {
    m_bounds ={};
    m_composition = { 0, 0, 0, 0, OffsetT() };
}

Pattern::Pattern( const Pattern& p ) {
    m_label =p.m_label;
    m_codeBits =p.m_codeBits;
    m_entryValuesBits = p.m_entryValuesBits;
    m_entryOffsetsBits = p.m_entryOffsetsBits;
    m_bounds =p.m_bounds;
    m_usage =p.m_usage;
    m_elements =p.m_elements;
    m_rowLength =p.m_rowLength;
    m_active = p.m_active;
    m_tabu = p.m_tabu;
    m_composition = p.m_composition;
}

Pattern::Pattern( const Matrix2D::ElementT& value, int rowLength ) 
    : m_usage( 0 ),
    m_label( 0 ),
    m_rowLength( rowLength ),
    m_codeBits( 0 ), m_entryOffsetsBits( 0 ), m_entryValuesBits( 0 ),
    m_active( true ), m_tabu( false ) {
    m_bounds ={ 0,0,0,0,1,1 };
    m_composition = { 0, 0, 0, 0, OffsetT() };
    m_elements.push_back( { OffsetT(0,0,rowLength), value } );
}

Pattern::Pattern( const Pattern& p1, const Pattern& p2, const OffsetT& offs )
    : m_usage( 0 ),
    m_label( 0 ),
    m_codeBits( 0 ), m_entryOffsetsBits( 0 ), m_entryValuesBits( 0 ),
    m_active( true ), m_tabu( false ) {
    m_rowLength = p1.rowLength();
    unionAdd( p1, p2, offs );
    recomputeBounds();
    m_composition = { &p1, &p2, 0, 0, offs };
}

Pattern::Pattern( const Pattern& p1, const Variant& v1, const Pattern& p2, const Variant& v2, const OffsetT& offs )
    : m_usage( 0 ),
    m_label( 0 ),
    m_codeBits( 0 ), m_entryOffsetsBits( 0 ), m_entryValuesBits( 0 ),
    m_active( true ), m_tabu( false ) {
    m_rowLength = p1.rowLength();
    Pattern p1v( p1 );
    v1.apply( p1v );
    Pattern p2v( p2 );
    v2.apply( p2v );
    unionAdd( p1v, p2v, offs );
    recomputeBounds();
    m_composition = { &p1, &p2, &v1, &v2, offs };
}

Pattern::~Pattern() {}

double 
Pattern::codeLength( int usage, int totalInstances, int modelSize ) {
    if( !usage )
        return 0.0;
    //return -log2( ((double)usage) / ((double)totalInstances) );
    return  - lgamma( (double)usage + pseudoCount ) / log(2)
            + lgamma( pseudoCount ) / log(2);

    //fprintf( stderr, "codeLength %d,%d,%d = %f\n", usage, totalInstances, modelSize, l );
}

double 
Pattern::updateCodeLength( int totalInstances, int modelSize ) {
//    for( auto&& elem : m_elements ) {
//    }
    return (m_codeBits = codeLength( m_usage, totalInstances, modelSize ));
}

double 
Pattern::entryOffsetsLength( int patternWidth, int patternHeight, int size ) {
    return /*size * log2( patternWidth * patternHeight ) 
         +*/ /*uintCodeLength( patternWidth )
         + uintCodeLength( patternHeight )
         + uintCodeLength( binom( (patternWidth*patternHeight), size ) )
         + log2(patternWidth * patternHeight); */       // empty cells take one bit
        +0;// + uintCodeLength( size );
}

double 
Pattern::updateEntryLength( const MassFunction& dist ) {
 //   return (m_entryBits = entryLength( m_bounds.width, m_bounds.height, base, size() ));

    m_entryOffsetsBits = entryOffsetsLength( m_bounds.width, m_bounds.height, size() );
    
    m_entryValuesBits =0.0;

    m_entryValuesBits = log2( dist.uniqueElements() ) * size();
    /*MassFunction f;
    for( auto&& elem : m_elements ) {
   //     m_entryValuesBits += -log2( dist.p( elem.value ) );
        f.increment( elem.value );
    }

    m_entryValuesBits += lgamma( (double)f.totalElements() + pseudoCount * (double)f.uniqueElements() ) / log(2)
                       - lgamma( pseudoCount * (double)f.uniqueElements() ) / log(2); 
    for( auto&& elem : f.elements() ) {
        m_entryValuesBits += -lgamma( (double)elem.second + pseudoCount ) / log(2)
                             +lgamma( pseudoCount ) / log(2);
    }*/

    return m_entryOffsetsBits + m_entryValuesBits;
   
#if 0
    double bits =uintCodeLength( size() );
    ElementT prev = { OffsetT(0,0), 0 };
    for( auto&& elem : m_elements ) {
        int col = std::abs( elem.offset.col()/* - prev.offset.col()*/ );
        int row = std::abs( elem.offset.row()/* - prev.offset.row()*/ );
        int value = std::abs( elem.value - prev.value );
        bits += 1.0 + uintCodeLength( col );
        bits += 1.0 + uintCodeLength( row );
        bits += 1.0 + uintCodeLength( value );
        prev = elem;
    }
    return bits;
#endif
}

void 
Pattern::setRowLength( int l ) {
    m_rowLength =l;
    for( auto&& elem : m_elements ) {
        elem.offset.setRowLength( l );
    }
}

void 
Pattern::recomputeBounds() {
    const ElementT& first =*m_elements.begin();
    m_bounds ={
        first.offset.row(),
        first.offset.row(),
        first.offset.col(),
        first.offset.col(),
        0,0 };
    for( auto&& elem : m_elements ) {
        m_bounds.rowMin = std::min( m_bounds.rowMin, elem.offset.row() );
        m_bounds.rowMax = std::max( m_bounds.rowMax, elem.offset.row() );
        m_bounds.colMin = std::min( m_bounds.colMin, elem.offset.col() );
        m_bounds.colMax = std::max( m_bounds.colMax, elem.offset.col() );
    }
    m_bounds.computeDimensions();
}

/** Returns true if at least one element of @p + @offs is adjacent to an element of this pattern.
 *  Note that @offs is assumed to be 'positive' (i.e. translates @p either right and/or down)
 */
bool 
Pattern::isAdjacent( const Pattern& p, const OffsetT& offs ) const {
  /*  for( auto it = m_elements.rbegin(); it != m_elements.rend(); it++ ) {
        const ElementT& elem =*it;
        for( auto elem2 : p.m_elements ) {
            OffsetT o = elem2.offset.translate( offs );
            if( o.row() - elem.offset.row() < 2 && o.col() - elem.offset.col() < 2 )
                return true;
        }
    }*/

    if( m_bounds.colMax+1 >= offs.col() + p.m_bounds.colMin
     && m_bounds.colMin-1 <= offs.col() + p.m_bounds.colMin
     && m_bounds.rowMax+1 >= offs.row() + p.m_bounds.rowMin )
        return true;
    return false;
}

/** Returns true if @offs lies within the bounding box returned by bounds() */
bool 
Pattern::isInside( const OffsetT& offs ) const {
    return offs.row() /*+ m_bounds.rowMin*/ <= m_bounds.rowMax
        && offs.col() /*+ m_bounds.colMin*/ <= m_bounds.colMax
        && offs.col() >= m_bounds.colMin;
}

/** Returns true if this pattern is in canonical form */
bool
Pattern::isCanonical() const {
    ElementT elem0 =*elements().begin();
    if( !elem0.offset.isZero() ) return false;
    int position = elem0.offset.position();

    for( auto&& elem : elements() ) {
        if( elem.offset.position() < position ) return false;
        position = elem.offset.position();
    }
    return true;
}

bool 
Pattern::apply( Matrix2D* mat, const Coord2D& pivot, bool flag ) {

    for( auto&& elem : elements() ) {
        Coord2D c =elem.offset.abs( pivot );

        if( flag ) { 
            mat->setFlagged( c, true ); 
            continue; 
        }

        if( mat->isFlagged( c ) ) return false;

        if( mat->value( c ) != elem.value )
            return false;
    }

    return true;
}

void
Pattern::debugPrint() const {
    printf( "Pattern #%d, bounds {%d, %d, %d, %d, %d, %d}\n", label(),
            bounds().rowMin, bounds().rowMax, bounds().colMin, bounds().colMax,
            bounds().width, bounds().height );

    const int n = bounds().width;
    const int m = bounds().height;
    char matrix[m][n];
    memset( matrix, '.',  m*n );

    for( auto&& elem : elements() ) {
        OffsetT of = elem.offset;
        matrix[of.row()-bounds().rowMin][of.col()-bounds().colMin] = '*';
    }

    for( int i =0; i < m; i++ ) {
        for( int j =0; j < n; j++ ) {
            printf( "%c", matrix[i][j] );
        }
        printf( "\n" );
    }
}

void 
Pattern::unionAdd( const Pattern& p1, const Pattern& p2, const OffsetT& offs ) {
    auto it1 = p1.elements().begin();
    auto it2 = p2.elements().begin();
    while( !(it1 == p1.elements().end() && it2 == p2.elements().end() ) ) {
        if( it1 == p1.elements().end() ){
            m_elements.push_back( { (*it2).offset.translate( offs ), (*it2).value } );
            it2++;
        }
        else if( it2 == p2.elements().end() ) {
            m_elements.push_back( *it1 );
            it1++;
        }
        else {
            if( (*it1).offset <= (*it2).offset.translate( offs ) ) {
                m_elements.push_back( *it1 );
                it1++;
            }
            else {
                m_elements.push_back( { (*it2).offset.translate( offs ), (*it2).value } );
                it2++;
            }
        }
    }

    assert( elements().size() == p1.elements().size() + p2.elements().size() );
}

VOUW_NAMESPACE_END

