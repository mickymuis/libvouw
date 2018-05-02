/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "pattern.h"
#include <vector>

VOUW_NAMESPACE_BEGIN

class Matrix2D;

class Variant {
    public:
        Variant( bool valid = false ) : m_valid( valid ) {}
        virtual ~Variant() {}

        virtual void apply( Pattern& ) const {}

        bool isValid() const { return m_valid; }
        void setValid( bool b ) { m_valid =b; }

        virtual int hash() const { return m_valid; }

    private:
        bool m_valid;
};

class EquivalenceSet {
    public:
        EquivalenceSet();
        virtual ~EquivalenceSet() {}

        virtual Variant* makeVariant( const Pattern& p, const Matrix2D* mat, const Coord2D& pivot );
        virtual Variant* makeVariant( const Pattern& p_union, 
                                     const Pattern& p1, 
                                     const Pattern& p2, 
                                     const Variant& v1, 
                                     const Variant& v2,
                                     const Pattern::OffsetT& offset );
        virtual Variant* makeNullVariant();
    private:
        Variant m_null;
        Variant m_true;

};

VOUW_NAMESPACE_END
