/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "instance.h"
#include <unordered_map>

VOUW_NAMESPACE_BEGIN

class InstanceMatrix {
    public: 
        typedef std::unordered_map<unsigned int, Instance*> MapT;

        InstanceMatrix();
        InstanceMatrix( int rowLength );
        ~InstanceMatrix();

        Instance* at( const Coord2D& );
        Instance* at( const Coord2D&, bool& isPivot );
        Instance* at( int row, int col );
        Instance* at( int row, int col, bool& isPivot );

        Instance* operator[]( const Coord2D& );
        const Instance* operator[]( const Coord2D& ) const;

        void place( Instance* );
        void place( Instance*, const Coord2D& pivot );

        void remove( Instance* );
        void remove( const Coord2D& );

        void setRowLength( int rowlength ) { m_rowLength =rowlength; }
        int rowLength() const { return m_rowLength; }

        void clear();

        size_t occupancy() const { return m_map.size(); }
      //  size_t count() const { return m_count; }

    private:
        inline unsigned int key( const Coord2D& ) const;
        inline unsigned int key( int row, int col ) const;
        MapT m_map;
        int m_rowLength;
        //size_t m_count;


};

VOUW_NAMESPACE_END
