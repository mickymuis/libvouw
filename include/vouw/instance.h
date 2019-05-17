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

inline bool instance_is_null( const Instance& );

class InstanceVector : public std::vector<Instance> {
    public:
        typedef uint32_t IndexT;
        InstanceVector( int matWidth =0, int matHeight =0, int matBase =0 );
        InstanceVector( const Matrix2D* mat );
        ~InstanceVector();

        void clearBitmasks();
        //void unflagAll();
        inline void eraseIfEmpty( InstanceVector::iterator begin, InstanceVector::iterator end ) { erase( std::remove_if( begin, end, instance_is_null ), end ); }

        void updateCodeLengths( int modelSize );

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
//        double m_stdBitsPerVariant;
};

typedef std::vector<InstanceVector::IndexT> InstanceIndexVectorT;

class Instance {
    public:
        typedef std::vector<bool> BitmaskT;
        Instance( Pattern* pattern, const Coord2D& pivot, const Variant* variant );
        Instance();
        ~Instance();

        void apply( Matrix2D* );

        inline void setPattern( Pattern* p );
        inline Pattern* pattern() const { return m_pattern; }

        inline void setPivot( const Coord2D& c ) { m_pivot =c; }
        inline Coord2D pivot() const { return m_pivot; }

        inline void setVariant( const Variant* v ) { m_variant =v; }
        inline const Variant* variant() const { return m_variant; }

      //  inline void bitmaskGrow( int size ) const { if( size > m_bitmask.size() ) m_bitmask.resize( size ); }
      //  inline BitmaskT& bitmask() const { return m_bitmask; }

      //  inline int& marker() { return m_marker; }

        /*inline void setVariantBits( double b ) { m_variantBits =b; }
        inline double& variantBits() { return m_variantBits; }
        inline double variantBits() const { return m_variantBits; }*/

        inline void clear() { m_pattern = NULL; m_pivot = Coord2D(0,0); /*m_bitmask.clear();*/ m_variant =NULL; }
        inline bool empty() const { return m_pattern == NULL; }

    private:

        Pattern* m_pattern;
        Coord2D m_pivot;

        friend class InstanceVector;
        mutable const Variant* m_variant;
    //    mutable BitmaskT m_bitmask;
        //mutable double m_variantBits;
    //    mutable int m_marker;
};

bool operator<(const Instance&, const Instance& );

inline bool instance_less_than( const Instance* inst1, const Instance* inst2 ) {
    return (*inst1 < *inst2);
}

inline bool instance_is_null( const Instance& inst ) {
    return inst.empty();
}

VOUW_NAMESPACE_END

