/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
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
class Instance;

inline bool instance_delete_if_masked( Instance* );

class InstanceList : public std::vector<Instance*> {
    public:
        InstanceList( int matWidth =0, int matHeight =0, int matBase =0 );
        InstanceList( const Matrix2D* mat );
        ~InstanceList();

        void clearBitmasks();
        void unflagAll();
        inline void deleteIfFlagged( InstanceList::iterator begin, InstanceList::iterator end ) { erase( std::remove_if( begin, end, instance_delete_if_masked ), end ); }

        void updateCodeLengths( int modelSize );

        inline int  tabuCount() const { return m_tabuCount; }
        inline int& tabuCount() { return m_tabuCount; }
        inline void setTabuCount( int c ) { m_tabuCount = c; }
        inline int totalCount() const { return size() + m_tabuCount; }

        inline double totalCodeLength() const { return m_bits; }
        inline double bitsPerPivot() const { return m_stdBitsPerPivot; }
        static double bitsPerPivot( std::size_t pivotCount );
//        double bitsPerVariant() const { return m_stdBitsPerVariant; }

        void setMatrixSize( int width, int height, int base );
        inline int matrixWidth() const { return m_width; }
        inline int matrixHeight() const { return m_height; }

        void decompose( const Instance& );

        void deleteAll();

    private:
        void decomposeRecursive( const Instance& );
        int m_width, m_height, m_base, m_nodeCount;
        double m_bits;
        double m_stdBitsPerPivot;
        int m_tabuCount;
//        double m_stdBitsPerVariant;
};

class Instance {
    public:
        typedef std::vector<bool> BitmaskT;
        Instance( Pattern* pattern, const Coord2D& pivot, const Variant* variant, bool flag =false );
        Instance();
        ~Instance();

        void apply( Matrix2D* );

        inline void setPattern( Pattern* p );
        inline Pattern* pattern() const { return m_pattern; }

        inline void setPivot( const Coord2D& c ) { m_pivot =c; }
        inline Coord2D pivot() const { return m_pivot; }

        inline void setVariant( const Variant* v ) { m_variant =v; }
        inline const Variant* variant() const { return m_variant; }

        inline void setFlagged( bool b ) const { m_flagged =b; }
        inline bool isFlagged( DirT dir = DirAll ) const { return m_flagged; }

        inline void bitmaskGrow( int size ) const { if( size > m_bitmask.size() ) m_bitmask.resize( size ); }
        inline BitmaskT& bitmask() const { return m_bitmask; }

        inline int& marker() { return m_marker; }

        inline void setVariantBits( double b ) { m_variantBits =b; }
        inline double& variantBits() { return m_variantBits; }
        inline double variantBits() const { return m_variantBits; }

    private:

        Pattern* m_pattern;
        Coord2D m_pivot;

        friend class InstanceList;
        mutable const Variant* m_variant;
        mutable BitmaskT m_bitmask;
        mutable bool m_flagged;
        mutable double m_variantBits;
        mutable int m_marker;
};

bool operator<(const Instance&, const Instance& );

inline bool instance_less_than( const Instance* inst1, const Instance* inst2 ) {
    return (*inst1 < *inst2);
}

inline bool instance_delete_if_masked( Instance* inst ) {
    if( inst->isFlagged() ) {
        delete inst;
        return true;
    }
    return false;
}

VOUW_NAMESPACE_END

