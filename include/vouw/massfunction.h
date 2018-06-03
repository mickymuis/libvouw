/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include "matrix.h"
#include <unordered_map>

VOUW_NAMESPACE_BEGIN

/** Models a finite probability mass function on integers that can be dynamically updated.
 *  Under the hood a hashmap is utilized to store the absolute frequencies of the given integers.
 */
class MassFunction {
    public:
        typedef unsigned int CountT;
        typedef Matrix2D::ElementT ElementT;
        typedef std::unordered_map<ElementT,CountT> MapT;
        MassFunction();

        CountT count( const ElementT& ) const;
        double p( const ElementT& ) const;

        void setCount( const ElementT&, const CountT& );
        void decrement( const ElementT& );
        void increment( const ElementT& );
        void erase( const ElementT& );
        void clear();

        ElementT lowerBound() const { return m_lower; }
        ElementT upperBound() const { return m_upper; }
        ElementT range() const { return m_count ? m_upper - m_lower + 1 : 0; }
        CountT totalElements() const { return m_count; }
        CountT uniqueElements() const { return m_elements.size(); }

        MapT& elements() { return m_elements; }
        const MapT& elements() const { return m_elements; }

    private:
        MapT m_elements;
        ElementT m_lower;
        ElementT m_upper;
        CountT m_count;
};

VOUW_NAMESPACE_END
