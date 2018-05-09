/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include <vector>
#include <algorithm>
#include <vouw/equivalence.h>
#include "matrix.h"

VOUW_NAMESPACE_BEGIN

class Pattern;
class Variant;
class Region;

inline bool region_is_masked( const Region& );

class RegionList : public std::vector<Region> {
    public:
        RegionList( int matWidth =0, int matHeight =0, int matBase =0 );
        RegionList( const Matrix2D* mat );
        ~RegionList();

        void clearBitmasks();
        void unflagAll();
        inline void eraseIfFlagged( RegionList::iterator begin, RegionList::iterator end ) { erase( std::remove_if( begin, end, region_is_masked ), end ); }

        void updateCodeLengths();

        inline double totalLength() const { return m_bits; }
        inline double bitsPerPivot() const { return m_stdBitsPerPivot; }
        static double bitsPerPivot( std::size_t pivotCount );
//        double bitsPerVariant() const { return m_stdBitsPerVariant; }

        void setMatrixSize( int width, int height, int base );
        inline int matrixWidth() const { return m_width; }
        inline int matrixHeight() const { return m_height; }

        void decompose( const Region& );

    private:
        void decomposeRecursive( const Region& );
        int m_width, m_height, m_base, m_nodeCount;
        double m_bits;
        double m_stdBitsPerPivot;
//        double m_stdBitsPerVariant;
};

class Region {
    public:
        typedef std::vector<bool> BitmaskT;
        Region( Pattern* pattern, const Coord2D& pivot, const Variant* variant, bool flag =false );
        Region();
        ~Region();

        void apply( Matrix2D* );

        inline void setPattern( Pattern* p ) { m_pattern =p; }
        inline Pattern* pattern() const { return m_pattern; }

        inline void setPivot( const Coord2D& c ) { m_pivot =c; }
        inline Coord2D pivot() const { return m_pivot; }

        inline void setVariant( const Variant* v ) { m_variant =v; }
        inline const Variant* variant() const { return m_variant; }

        inline void setFlagged( bool b ) const { m_flagged =b; }
        inline bool isFlagged( DirT dir = DirAll ) const { return m_flagged; }

        inline void bitmaskGrow( int size ) const { if( size > m_bitmask.size() ) m_bitmask.resize( size ); }
        inline BitmaskT& bitmask() const { return m_bitmask; }

        inline void setVariantBits( double b ) { m_variantBits =b; }
        inline double& variantBits() { return m_variantBits; }
        inline double variantBits() const { return m_variantBits; }

    private:

        Pattern* m_pattern;
        Coord2D m_pivot;

        friend class RegionList;
        mutable const Variant* m_variant;
        mutable BitmaskT m_bitmask;
        mutable bool m_flagged;
        mutable double m_variantBits;
};

bool operator<(const Region&, const Region& );

inline bool region_is_masked( const Region& r ) {
    return r.isFlagged();
}

VOUW_NAMESPACE_END

