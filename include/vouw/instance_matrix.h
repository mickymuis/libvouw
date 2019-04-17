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
        typedef unsigned int KeyT;
        typedef InstanceVector::IndexT IndexT;
        typedef std::unordered_map<KeyT, IndexT> MapT;

        static IndexT empty;

        InstanceMatrix();
        InstanceMatrix( int rowLength );
        ~InstanceMatrix();

        IndexT at( const Coord2D& );
        IndexT at( int row, int col );

        IndexT operator[]( const Coord2D& );
        const IndexT operator[]( const Coord2D& ) const;

        void place( IndexT idx, const Instance&, const Coord2D& pivot );
        void place( IndexT idx, const Instance& );

        void remove( const Instance& );

        void setRowLength( int rowlength ) { m_rowLength =rowlength; }
        int rowLength() const { return m_rowLength; }

        void clear();

        size_t occupancy() const { return m_map.size(); }
      //  size_t count() const { return m_count; }

    private:
        inline KeyT key( const Coord2D& ) const;
        inline KeyT key( int row, int col ) const;
        MapT m_map;
        int m_rowLength;
        //size_t m_count;


};

VOUW_NAMESPACE_END
