/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "pattern.h"
#include "region.h"

VOUW_NAMESPACE_BEGIN

class Matrix2D;
class EquivalenceSet;
class CodeTable;
struct Candidate;

class Encoder {
    public:
        Encoder( EquivalenceSet* = new EquivalenceSet() );
        Encoder( Matrix2D* mat, EquivalenceSet* = new EquivalenceSet() );
        Encoder( Matrix2D* mat, CodeTable* ct, EquivalenceSet* = new EquivalenceSet() );
        ~Encoder();

        bool isValid() const;

        void setFromMatrix( Matrix2D* mat );
        void setFromMatrixUsing( Matrix2D* mat, CodeTable* ct );

        EquivalenceSet* equivalenceSet();
        void setEquivalenceSet( EquivalenceSet* ) ;

        void clear();

        bool encodeStep();
        int encode();
        Matrix2D* decode();
        
        bool isEncoded() const { return m_isEncoded; }

        Matrix2D* matrix() const { return m_mat; }
        CodeTable* codeTable() const { return m_ct; }
        const RegionList* instanceSet() const { return &m_encoded; }

        double uncompressedSize() const { return m_priorBits; }
        double compressedSize() const { return m_encodedBits; }
        double ratio() const { return m_encodedBits / m_priorBits; }       

    protected:
        double updateCodeLengths();

        EquivalenceSet* m_es;
        Matrix2D* m_mat;
        CodeTable* m_ct;
        RegionList m_encoded;
        double m_priorBits;
        double m_encodedBits;
        bool m_isEncoded;
        int m_lastLabel;
        
    private:
        Encoder( const Encoder& ) {}
        double computeGain( const Candidate*, int usage );
        void mergePatterns( const Candidate* );
        void prunePattern( Pattern* );

};

VOUW_NAMESPACE_END