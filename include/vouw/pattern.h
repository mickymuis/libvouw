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
class MassFunction;

/** The Pattern class is the elementary type for the CodeTable. It specifies a
  * non-overlapping set of value that correspond with the original input matrix.
  * Pattern stores its elements directly; therefore use a reference or a pointer
  * to pass object of the Pattern type, to avoid unnecessary copying.
  */
class Pattern {
    public:
        /** Defines offsets within the pattern */
        class OffsetT : public Coord2D { 
            public:
                OffsetT( int row =0, int col =0, int rowLength =0 );
                OffsetT( const Coord2D& pivot1, const Coord2D& pivot2 );

                Coord2D abs( const Coord2D& pivot ) const;
                OffsetT translate( const OffsetT& offs ) const;

                DirT direction() const;

                int position() const override;
        };
        /** Elementary type of the pattern, tuple of offset and value */
        struct ElementT {
            OffsetT offset;
            Matrix2D::ElementT value;
        };
        /** Container of pattern elements */
        typedef std::vector<ElementT> ListT;
        /** Defines the bouding box around the pattern and contains 
         * the total surface area a pattern spans. */
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
        /** When a pattern is constructed using one of the union constructors,
         * this type is used to store the original composition of the pattern */
        struct CompositionT {
            const Pattern *p1, *p2;
            const Variant *v1, *v2;
            OffsetT offset;

            bool isValid() const { return p1 && p2; }
        };
        typedef std::pair<Pattern*,Variant*> EquivalenceT;
        typedef std::vector<EquivalenceT> EquivalenceListT;

        /* Constructors */

        Pattern();
        Pattern( const Pattern& );
        Pattern( const Matrix2D::ElementT&, int rowLength );
        Pattern( const Pattern& p1, const Pattern& p2, const OffsetT& );
        Pattern( const Pattern& p1, const Variant& v1, const Pattern& p2, const Variant& v2, const OffsetT& );
        ~Pattern();

        /* General properties */

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

        void setTabu( bool b ) { m_tabu =b; }
        bool isTabu() const { return m_tabu; }

        /* Functions that are related to (computing) the code length */

        static double codeLength( int usage, int totalInstances, int modelSize );
        double updateCodeLength( int totalInstances, int modelSize );
        double codeLength() const { return m_codeBits; }
        static double entryOffsetsLength( int patternWidth, int patternHeight, int size );
        double updateEntryLength( const MassFunction& distribution );
        double entryLength() const { return m_entryOffsetsBits + m_entryValuesBits; }
        double entryOffsetsLength() const { return m_entryOffsetsBits; }
        double entryValuesLength() const { return m_entryValuesBits; }

        /* Functions for computing, obtaining and comparing the bounds of a pattern */

        inline BoundsT bounds() const { return m_bounds; }
        void recomputeBounds();
        inline int squareSize() const { return m_bounds.width * m_bounds.height; }
        bool isAdjacent( const Pattern& p, const OffsetT& offs ) const;
        bool isInside( const OffsetT& offs ) const;
        bool isCanonical() const;

        /* Functions that apply or test a pattern against a matrix */

        bool apply( Matrix2D* mat, const Coord2D& pivot, bool flag =false );

        /** Returns a reference to the list of elements */
        ListT& elements() { return m_elements; }
        const ListT& elements() const { return m_elements; }

        /** Returns a reference to the composition object if the pattern was constructed
          * using the union constructors. */
        const CompositionT& composition() const { return m_composition; }

        void debugPrint() const;

    private:
        void unionAdd( const Pattern& p1, const Pattern& p2, const OffsetT& );
        ListT m_elements;
        BoundsT m_bounds;
        int m_usage;
        int m_label;
        double m_codeBits;
        double m_entryOffsetsBits;
        double m_entryValuesBits;
        int m_rowLength;
        bool m_active;
        bool m_tabu;
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
