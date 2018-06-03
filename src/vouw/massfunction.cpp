/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#include <vouw/massfunction.h>
#include <stdexcept>
#include <algorithm>

VOUW_NAMESPACE_BEGIN

MassFunction::MassFunction() :
    m_lower( 0 ),
    m_upper( 0 ),
    m_count( 0 ) {}

MassFunction::CountT 
MassFunction::count( const ElementT& n ) const {
    CountT c;
    try {
        c = m_elements.at( n );
    }
    catch( const std::out_of_range& oor ) {
        return 0;
    }
    return c;
}

double 
MassFunction::p( const ElementT& n ) const {
    CountT c = count( n );
    return (double)c/(double)m_count;
}

void 
MassFunction::setCount( const ElementT& n, const CountT& c ) {
    CountT old_c = count( n );
    if( c == 0 ) {
        if( old_c == 0 ) return;
        erase( n );
    } else {
        m_elements[n] = c;
        if( n > m_upper ) m_upper =n;
        if( m_count == 0 ) m_lower =n;
        else if( n < m_lower ) m_lower =n;
        m_count += c - old_c;
    }
}

void 
MassFunction::decrement( const ElementT& n ) {
    try {
        CountT c = m_elements.at( n );
        if( c != 0 ) {
            if( c-1 == 0 ) {
                erase( n );
            } else {
                m_elements.at( n )--;
                m_count--;
            }
        }
    }
    catch( const std::out_of_range& oor ) {
        return;
    }
}

void 
MassFunction::increment( const ElementT& n ) {
    m_elements[n]++;
    m_count++;
    if( n > m_upper ) m_upper =n;
    if( m_count == 0 ) m_lower =n;
    else if( n < m_lower ) m_lower =n;
}

void 
MassFunction::erase( const ElementT& n ) {
    CountT c = count( n );
    if( c == 0 ) return;
    m_count -= c;
    m_elements.erase( n );

    auto result =std::minmax_element( m_elements.begin(), m_elements.end() );
    m_lower = (*result.first).first;
    m_upper = (*result.second).first;
}

void
MassFunction::clear() {
    m_elements.clear();
    m_lower =m_upper =m_count =0;
}

VOUW_NAMESPACE_END
