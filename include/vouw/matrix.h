/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include <cinttypes>

VOUW_NAMESPACE_BEGIN

#define VOUW_UNODE32_FLAGGED (1U << 31)

class Coord2D {
    public:
        Coord2D( int row, int col, int rowLength =0 ) : m_row(row), m_col(col), m_rowLength(rowLength) {}
        Coord2D() : m_row(0), m_col(0) {}

        void setRow( int row) { m_row =row; }
        int row() const { return m_row; }

        void setCol( int col) { m_col =col; }
        int col() const { return m_col; }

        bool isZero() { return m_row == 0 && m_col == 0; }

        void setRowLength( int rowLength ) { m_rowLength = rowLength; } 
        int rowLength() const { return m_rowLength; }

        virtual int position() const;

    private:
        int m_row, m_col, m_rowLength;

};

bool operator==( const Coord2D&, const Coord2D& );
bool operator!=( const Coord2D&, const Coord2D& );
bool operator<=( const Coord2D&, const Coord2D& );
bool operator<( const Coord2D&, const Coord2D& );
bool operator>=( const Coord2D&, const Coord2D& );
bool operator>( const Coord2D&, const Coord2D& );

class Matrix2D {
    public:
        typedef uint32_t ElementT;
        Matrix2D( unsigned int width, unsigned int height, unsigned int base );
        Matrix2D( const Matrix2D& );
        ~Matrix2D();

        Coord2D makeCoord( int row, int col );

        unsigned int width() const { return m_width; }
        unsigned int height() const { return m_height; }
        unsigned int count() const { return m_width * m_height; }
        unsigned int base() const { return m_base; }

        void clear();

        ElementT* data();
        const ElementT* data() const;
        ElementT* rowPtr( unsigned int row );
        const ElementT* rowPtr( unsigned int row ) const;

        //ElementT& value( Coord2D );
        ElementT value( Coord2D ) const;
        void setValue( Coord2D, const ElementT&);

        void setFlagged( const Coord2D&, bool );
        bool isFlagged( const Coord2D& ) const;
        void unflagAll();

        bool checkBounds( const Coord2D& ) const;

        bool operator==( const Matrix2D& );

    private:
        unsigned int m_width, m_height, m_base;
        ElementT* m_buffer;
};

VOUW_NAMESPACE_END

