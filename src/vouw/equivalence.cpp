/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/equivalence.h>
#include <vouw/pattern.h>
#include <vouw/matrix.h>

VOUW_NAMESPACE_BEGIN

/* The default equivalence is simply strict isomorphism */
/*bool 
EquivalenceSet::isEquivalent( const Pattern* p1, const Pattern* p2 ) const {
    const Pattern::ListT& list1 = p1->elements();
    const Pattern::ListT& list2 = p2->elements();
    return std::equal( list1.begin(), list1.end(), list2.begin() );
}*/

EquivalenceSet::EquivalenceSet() : m_null( false ), m_true( true ) {}

/* The default equivalence simply does a test for a strict isomorphic occurence */
Variant* 
EquivalenceSet::makeVariant( const Pattern& p, const Matrix2D* mat, const Coord2D& pivot ) {

   for( auto&& elem : p.elements() ) {
        if( !mat->checkBounds( elem.offset ) )
            return &m_null;
        if( mat->value( elem.offset.abs( pivot ) ) != elem.value )
            return &m_null;
   }
   return &m_true;
}

Variant* 
EquivalenceSet::makeVariant( const Pattern& p_union, 
             const Pattern& p1, 
             const Pattern& p2, 
             const Variant& v1, 
             const Variant& v2,
             const Pattern::OffsetT& offset ) {

    return &m_null;
}

Variant*
EquivalenceSet::makeNullVariant() {
    return &m_null;
}

/* The null-variant */
/*Variant 
EquivalenceSet::makeVariant( const Pattern*, const Pattern*) const {
    return Variant();
}*/

VOUW_NAMESPACE_END

