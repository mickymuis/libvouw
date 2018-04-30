/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
//#include <set>
#include <algorithm>
#include <vouw/equivalence.h>

VOUW_NAMESPACE_BEGIN

class Pattern;
class Variant;
class Region;
class Matrix2D;

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

    private:
        int m_width, m_height, m_base, m_nodeCount;
        double m_bits;
        double m_stdBitsPerPivot;
//        double m_stdBitsPerVariant;
};

class Region {
    public:
        Region( Pattern* pattern, const Coord2D& pivot, const Variant& variant, bool masked =false );
        Region();
        ~Region();

        void apply( Matrix2D* );

        void setPattern( Pattern* p ) { m_pattern =p; }
        Pattern* pattern() const { return m_pattern; }

        void setPivot( const Coord2D& c ) { m_pivot =c; }
        Coord2D pivot() const { return m_pivot; }

        void setVariant( const Variant& v ) { m_variant =v; }
        const Variant& variant() const { return m_variant; }
        Variant& variant() { return m_variant; }

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
        mutable Variant m_variant;
        mutable int m_mask;
        mutable double m_variantBits;
};

bool operator<(const Region&, const Region& );

inline bool region_is_masked( const Region& r ) {
    return r.isMasked();
}

VOUW_NAMESPACE_END

//#ifndef REGION_H
#if 0
#define REGION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vouw/matrix.h>
#include <vouw/pattern.h>
#include "list.h"

typedef struct {
    struct list_head list;
    pattern_t* pattern;
    vouw_coord_t pivot;
    int variant;
    bool masked;
} region_t;

region_t*
region_create( pattern_t* pattern, vouw_coord_t pivot );

void
region_apply( const region_t* region, vouw_matrix_t* m );

void
region_free( region_t* r );

void
region_list_free( region_t* );

void 
region_list_unmask( region_t* );

#ifdef __cplusplus
}
#endif

#endif

