/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include "vouw.h"
//#include "pattern.h"

#include <vector>
#include <cinttypes>

VOUW_NAMESPACE_BEGIN

class Pattern;

class Configuration {
    public:
        typedef std::vector<bool> DataT;
        Configuration( const Pattern& p );

        bool operator==( const Configuration& rhs ) const;

    private:
        DataT m_data;
        int m_width;
        void setFromPattern( const Pattern& p );
};

typedef std::vector<Configuration> ConfigVectorT;
typedef ConfigVectorT::size_type ConfigIDT;

VOUW_NAMESPACE_END
