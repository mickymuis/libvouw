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

        void unmaskAll();
        inline void eraseIfMasked( RegionList::iterator begin, RegionList::iterator end ) { erase( std::remove_if( begin, end, region_is_masked ), end ); }

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
        Region( Pattern* pattern, const Coord2D& pivot, const Variant* variant, bool masked =false );
        Region();
        ~Region();

        void apply( Matrix2D* );

        void setPattern( Pattern* p ) { m_pattern =p; }
        Pattern* pattern() const { return m_pattern; }

        void setPivot( const Coord2D& c ) { m_pivot =c; }
        Coord2D pivot() const { return m_pivot; }

        void setVariant( const Variant* v ) { m_variant =v; }
        const Variant* variant() const { return m_variant; }

        void setMasked( bool b, DirT dir = DirAll ) const;
        bool isMasked( DirT dir = DirAll ) const { return m_mask & dir; }
        int& mask() { return m_mask; }

        void setVariantBits( double b ) { m_variantBits =b; }
        double& variantBits() { return m_variantBits; }
        double variantBits() const { return m_variantBits; }

    private:

        Pattern* m_pattern;
        Coord2D m_pivot;

        friend class RegionList;
        mutable const Variant* m_variant;
        mutable int m_mask;
        mutable double m_variantBits;
};

bool operator<(const Region&, const Region& );

inline bool region_is_masked( const Region& r ) {
    return r.isMasked();
}

VOUW_NAMESPACE_END

