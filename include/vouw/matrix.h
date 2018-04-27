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

#if 0
//#ifndef VOUW_MATRIX_H
#define VOUW_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define VOUW_UNODE32_FLAGGED (1U << 31)

/** Pricipal element type for VOUW input matrices. 
 * The 32nd bit is reserved for internal use. */
typedef uint32_t vouw_unode32_t;

typedef struct {
    int row;
    int col;
} vouw_coord_t;

typedef struct {
    vouw_unode32_t* cols;
    int size;

} vouw_row_t;

/** Dense, rectangular matrix of postive integers.
 * The @base field denotes the number of possible values for each element. */
typedef struct {
    int base;                   // Largest element value is base-1
    int height;                 // Number of rows
    int width;                  // Number of columns
    int count;                  // Total number of elements
    vouw_row_t* rows;           // Array of pointers to each row
    vouw_unode32_t* buffer;     // Pointer to storage buffer
} vouw_matrix_t;

vouw_matrix_t*
vouw_matrix_create( int width, int height, int base );

void
vouw_matrix_free( vouw_matrix_t* m );

void
vouw_matrix_clear( vouw_matrix_t* m );

vouw_unode32_t 
vouw_matrix_value( const vouw_matrix_t* m, vouw_coord_t c );

void
vouw_matrix_setValue( vouw_matrix_t* m, vouw_coord_t c, vouw_unode32_t value );

int 
vouw_matrix_rowLength( const vouw_matrix_t* m, int row );

bool
vouw_matrix_checkBounds( const vouw_matrix_t* m, vouw_coord_t c );

bool
vouw_matrix_isEqual( const vouw_matrix_t* m1, const vouw_matrix_t* m2 );

void
vouw_matrix_setFlagged( vouw_matrix_t* m, vouw_coord_t c, bool mask );

bool
vouw_matrix_isFlagged( const vouw_matrix_t* m, vouw_coord_t c );

void
vouw_matrix_unflagAll( vouw_matrix_t* m );

#ifdef __cplusplus
}
#endif

#endif
