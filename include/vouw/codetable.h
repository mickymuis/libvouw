/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include <list>
#include <algorithm>

VOUW_NAMESPACE_BEGIN

class Pattern;
class Matrix2D;
class MassFunction;

inline bool pattern_is_active( const Pattern* );

class CodeTable : public std::list<Pattern*> {
    public:
        CodeTable( int matWidth =0, int matHeight =0, int matBase =0 );
        CodeTable( const Matrix2D* mat );
        ~CodeTable();

        void updateCodeLengths( int totalInstances, const MassFunction& distribution );
        void sortBySizeDesc();

        inline int countIfActive() const { return std::count_if( begin(), end(), pattern_is_active ); }
        double totalLength() const { return m_bits; }
        //double bitsPerOffset() const { return m_stdBitsPerOffset; }

        void setMatrixSize( int width, int height, int base );
        int matrixWidth() const { return m_width; }
        int matrixHeight() const { return m_height; }

        //void sortByUsageDesc();
        //
    private:
        int m_width, m_height, m_base, m_nodeCount;
        double m_bits;
        double m_stdBitsPerOffset;

};

VOUW_NAMESPACE_END
