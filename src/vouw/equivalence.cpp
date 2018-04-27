/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/equivalence.h>

VOUW_NAMESPACE_BEGIN

/* The default equivalence is simply strict isomorphism */
/*bool 
EquivalenceSet::isEquivalent( const Pattern* p1, const Pattern* p2 ) const {
    const Pattern::ListT& list1 = p1->elements();
    const Pattern::ListT& list2 = p2->elements();
    return std::equal( list1.begin(), list1.end(), list2.begin() );
}*/

/* The default equivalence simply does a test for a strict isomorpihic occurence */
Variant 
EquivalenceSet::makeVariant( const Pattern& p, const Matrix2D* mat, const Coord2D& pivot ) {

   for( auto&& elem : p.elements() ) {
        if( !mat->checkBounds( elem.offset ) )
            return Variant(false);
        if( mat->value( elem.offset.abs( pivot ) ) != elem.value )
            return Variant(false);
   }
   return Variant(true);
}

Variant 
EquivalenceSet::makeVariant( const Pattern& p_union, 
             const Pattern& p1, 
             const Pattern& p2, 
             const Variant& v1, 
             const Variant& v2,
             const Pattern::OffsetT& offset ) {

    return Variant(false);
}

Variant
EquivalenceSet::makeNullVariant() {
    return Variant(true);
}

/* The null-variant */
/*Variant 
EquivalenceSet::makeVariant( const Pattern*, const Pattern*) const {
    return Variant();
}*/

VOUW_NAMESPACE_END

