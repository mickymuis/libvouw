/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "matrix.h"
//#include "equivalence.h"
#include <vector>

VOUW_NAMESPACE_BEGIN

class Variant;

class Pattern {
    public:
        class OffsetT : public Coord2D { 
            public:
                OffsetT( int row =0, int col =0, int rowLength =0 );
                OffsetT( const Coord2D& pivot1, const Coord2D& pivot2 );

                Coord2D abs( const Coord2D& pivot ) const;
                OffsetT translate( const OffsetT& offs ) const;

                DirT direction() const;

                int position() const override;
        };
        struct ElementT {
            OffsetT offset;
            Matrix2D::ElementT value;
        };
        typedef std::vector<ElementT> ListT;
        struct BoundsT {
            int rowMin;
            int rowMax;
            int colMin;
            int colMax;
            int width, height;
        };

        Pattern();
        Pattern( const Pattern& );
        Pattern( const Matrix2D::ElementT&, int rowLength );
        Pattern( const Pattern& p1, const Pattern& p2, const OffsetT& );
        Pattern( const Pattern& p1, const Variant& v1, const Pattern& p2, const Variant& v2, const OffsetT& );
        ~Pattern();

        void setUsage( int u ) { m_usage =u; }
        int usage() const { return m_usage; }
        int& usage() { return m_usage; }

        int size() const { return m_elements.size(); }

        void setRowLength( int l );
        int rowLength() const { return m_rowLength; }

        void setLabel( int i ) { m_label =i; }
        int label() const { return m_label; }

        static double codeLength( int usage, int totalInstances );
        double updateCodeLength( int totalInstances );
        double codeLength() const { return m_codeBits; }
        static double bitsPerOffset( int patternWidth, int patternHeight, int base );
        double updateBitsPerOffset( int base );
        double bitsPerOffset() const { return m_bitsPerOffset; }

        inline BoundsT bounds() const { return m_bounds; }
        void recomputeBounds();
        inline int squareSize() const { return m_bounds.width * m_bounds.height; }

        bool isAdjacent( const Pattern& p, const OffsetT& offs ) const;

        ListT& elements() { return m_elements; }
        const ListT& elements() const { return m_elements; }

    private:
        void unionAdd( const Pattern& p1, const Pattern& p2, const OffsetT& );
        ListT m_elements;
        BoundsT m_bounds;
        int m_usage;
        int m_label;
        double m_codeBits;
        double m_bitsPerOffset;
        int m_rowLength;
};

VOUW_NAMESPACE_END

#if 0
//#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include <vouw/matrix.h>
#include <stdbool.h>

typedef struct {
    int row;
    int col;
    vouw_unode32_t value;
} pattern_offset_t;

vouw_coord_t
pattern_offset_abs( vouw_coord_t c, pattern_offset_t offs );

pattern_offset_t
pattern_offset( vouw_coord_t pivot1, vouw_coord_t pivot2 );

typedef struct {
    int rowMin;
    int rowMax;
    int colMin;
    int colMax;
    int width, height;
} pattern_bounds_t;

typedef struct {
    struct list_head list;
    pattern_offset_t* offsets;
    unsigned int usage;
    unsigned int size;
    double codeLength;
    char label; // for debug printingi
    pattern_bounds_t bounds;
} pattern_t;


pattern_t*
pattern_createSingle( int value );

pattern_t*
pattern_createUnion( const pattern_t* p1, const pattern_t* p2, pattern_offset_t p2_offset );

pattern_t*
pattern_createVariantUnion( const pattern_t* p1, const pattern_t* p2, int variant, pattern_offset_t p2_offset, int base );

pattern_t*
pattern_createCopy( const pattern_t* src );

void
pattern_free( pattern_t* p );

void
pattern_updateCodeLength( pattern_t* p, unsigned int totalNodeCount );

double 
pattern_computeCodeLength( const pattern_t* p, unsigned int totalNodeCount );

bool
pattern_isMatch( const pattern_t* p, const vouw_matrix_t* m, vouw_coord_t pivot, int* variant );

pattern_bounds_t
pattern_computeBounds( const pattern_t* p );

void
pattern_setMatrixValues( const pattern_t* p, vouw_coord_t pivot, vouw_matrix_t* m, vouw_unode32_t value );

void
pattern_list_free( pattern_t* list );

void 
pattern_list_setLabels( pattern_t* list );

double
pattern_list_updateCodeLength( pattern_t* list, unsigned int totalNodeCount );

void
pattern_list_sortByUsageDesc( pattern_t* head );

void
pattern_list_sortBySizeDesc( pattern_t* head );

#ifdef __cplusplus
}
#endif

#endif
