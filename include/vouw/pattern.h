/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "matrix.h"
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

            inline void computeDimensions() { 
                width = (colMax - colMin) + 1; height =(rowMax - rowMin) + 1;
            }
        };
        struct CompositionT {
            const Pattern *p1, *p2;
            const Variant *v1, *v2;
            OffsetT offset;

            bool isValid() const { return p1 && p2; }
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

        void setActive( bool b ) { m_active =b; }
        bool isActive() const { return m_active; }

        static double codeLength( int usage, int totalInstances );
        double updateCodeLength( int totalInstances );
        double codeLength() const { return m_codeBits; }
        static double bitsPerOffset( int patternWidth, int patternHeight, int base );
        double updateBitsPerOffset( int base );
        double bitsPerOffset() const { return m_bitsPerOffset; }

        inline BoundsT bounds() const { return m_bounds; }
        void recomputeBounds();
        inline int squareSize() const { return m_bounds.width * m_bounds.height; }

        bool isAdjacent( /*const Pattern& p,*/const OffsetT& offs ) const;
        bool isInside( const OffsetT& offs ) const;

        ListT& elements() { return m_elements; }
        const ListT& elements() const { return m_elements; }

        const CompositionT& composition() const { return m_composition; }

        void debugPrint() const;

    private:
        void unionAdd( const Pattern& p1, const Pattern& p2, const OffsetT& );
        ListT m_elements;
        BoundsT m_bounds;
        int m_usage;
        int m_label;
        double m_codeBits;
        double m_bitsPerOffset;
        int m_rowLength;
        bool m_active;
        CompositionT m_composition;
};

inline bool pattern_is_active( const Pattern* p ) { return p->isActive(); }

/*class Composition {
    public:
        Composition( const Pattern* p1, const Pattern* p2, const Variant& v1, const Variant& v2, const Pattern::OffsetT& offset );
        Composition();
        ~Composition() {}

        Pattern* p1() { return m_p1; }
        const Pattern* p1() const { return m_p1; }
        Pattern* p2() { return m_p2; }
        const Pattern* p2() const { return m_p2; }

        const Pattern::OffsetT& offset() const { return m_offset; }

        const Variant& v1() const { return m_v1; }
        const Variant& v2() const { return m_v2; }

    private:
        Pattern *m_p1, *m_p2;
        Variant m_v1, m_v2;
        Pattern::OffsetT m_offset;
};*/

VOUW_NAMESPACE_END
