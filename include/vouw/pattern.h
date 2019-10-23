/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "matrix.h"
#include "configuration.h"
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
                OffsetT negate() const;

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
         *  this type is used to store the original composition of the pattern */
        struct CompositionT {
            const Pattern *p1, *p2;
            const Variant *v1, *v2;
            OffsetT offset;

            bool isValid() const { return p1 && p2; }
        };
        /** The pattern's periphery is the set of the immediate adjacent elements 
         *  that surround a pattern in the data. We distinguish between the elements
         *  that come before it (anterior) and those that come after (posterior). */
        enum PeripheryPosition {
            AnteriorPeriphery, PosteriorPeriphery
        };
        typedef std::vector<OffsetT> PeripheryT;

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
        static double entryOffsetsLength( int patternWidth, int patternHeight, int size, int matrixWidth, int matrixHeight );
        double updateEntryLength( const MassFunction& distribution, int matrixWidth, int matrixHeight );
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

        /* Functions that interact with the pattern's periphery */
        const PeripheryT& periphery( PeripheryPosition p = AnteriorPeriphery ) const;

        /** Returns a reference to the list of elements */
        ListT& elements() { return m_elements; }
        const ListT& elements() const { return m_elements; }

        /** Returns a reference to the composition object if the pattern was constructed
          * using the union constructors. */
        const CompositionT& composition() const { return m_composition; }

        /** Set the user-defined configuration-id for this pattern*/
        void setConfiguration( const ConfigIDT& id ) { m_config =id; }
        /** Returns the used-defined configuration-id for this pattern */
        ConfigIDT configuration() const { return m_config; }

        void debugPrint() const;

    private:
        void unionAdd( const Pattern& p1, const Pattern& p2, const OffsetT& );
        void recomputePeriphery();
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
        PeripheryT m_periphery[2];
        ConfigIDT m_config;
};

inline bool pattern_is_active( const Pattern* p ) { return p->isActive(); }
inline bool pattern_is_active_gte( const Pattern* p, int minSize ) { return p->isActive() && p->size() >= minSize; }

VOUW_NAMESPACE_END
