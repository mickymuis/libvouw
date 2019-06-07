/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include <vouw/configuration.h>
#include <vouw/pattern.h>

VOUW_NAMESPACE_BEGIN

Configuration::Configuration( const Pattern& p  ) {
    setFromPattern( p );
}

void
Configuration::setFromPattern( const Pattern& p ) {
    m_data.clear();
    m_data.resize( p.bounds().width * p.bounds().height );
    int w = m_width = p.bounds().width;
    for( const auto & elem : p.elements() ) {
        int key = (elem.offset.row() - p.bounds().rowMin) * w 
            + (elem.offset.col() - p.bounds().colMin);
        m_data[key] =true;
    }
}

bool 
Configuration::operator==( const Configuration& rhs ) const {
    return m_width == rhs.m_width && m_data == rhs.m_data;
}

VOUW_NAMESPACE_END

