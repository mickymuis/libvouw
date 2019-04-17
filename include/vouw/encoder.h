/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "pattern.h"
#include "instance.h"
#include "instance_matrix.h"
#include "massfunction.h"
#include "candidate.h"
#include <map>

VOUW_NAMESPACE_BEGIN

class Matrix2D;
class EquivalenceSet;
class CodeTable;

class Encoder {
    public:
        Encoder( EquivalenceSet* = new EquivalenceSet() );
        Encoder( Matrix2D* mat, EquivalenceSet* = new EquivalenceSet() );
        Encoder( Matrix2D* mat, CodeTable* ct, EquivalenceSet* = new EquivalenceSet() );
        ~Encoder();

        bool isValid() const;

        void setFromMatrix( Matrix2D* mat, bool useTabu =true );
        void setFromMatrixUsing( Matrix2D* mat, CodeTable* ct );

        EquivalenceSet* equivalenceSet();
        void setEquivalenceSet( EquivalenceSet* ) ;

        void clear();

        bool encodeStep();
        int encode();
        void reencode();
        Matrix2D* decode();
        
        bool isEncoded() const { return m_isEncoded; }

        Matrix2D* matrix() const { return m_mat; }
        CodeTable* codeTable() const { return m_ct; }
        const InstanceVector* instanceSet() const { return &m_instvec; }
        double uncompressedSize() const { return m_priorBits; }
        int totalCount() const { return m_instanceCount + m_tabuCount; }

        double compressedSize() const { return m_encodedBits; }
        double ratio() const { return m_encodedBits / m_priorBits; }       

    private:
        Encoder( const Encoder& ) {}
        void rebuildCandidateMap();
        double updateCodeLengths();
        double computeCandidateEntryLength( const Candidate*, bool debugPrint =false );
        double computeGain( const Candidate*, int usage, int modelSize, bool debugPrint =false );
        double computePruningGain( const Pattern* p );
        double computeDecompositionGain( const Pattern* p, int modelSize, bool debugPrint =false );
        void mergePatterns( const Candidate* );
        bool prunePattern( Pattern*, bool onlyZeroPattern = true );
        void decompose( Instance& );
        void rebuildInstanceMatrix( bool sort = false );

        EquivalenceSet* m_es;
        Matrix2D* m_mat;
        CodeTable* m_ct;
        InstanceVector m_instvec;
        InstanceMatrix m_instmat;
        const MassFunction* m_massfunc;
        CandidateMapT m_candidates;
        std::vector<InstanceVector::IndexT> m_instanceMarker;
        std::vector<Instance::BitmaskT> m_overlapMask;

        typedef std::pair<Pattern*,Variant*> PatternVariantT;
        typedef std::map<Matrix2D::ElementT,PatternVariantT> SingletonEqvMapT;
        typedef std::map<Pattern*,int> PatternUsageMapT;
        SingletonEqvMapT m_smap; // Singleton equivalence mapping
        
        int m_tabuCount;
        int m_instanceCount;

        double m_priorBits;
        double m_encodedBits;
        bool m_isEncoded;
        int m_lastLabel;
        int m_decompositions;
        int m_iteration;
        
};

VOUW_NAMESPACE_END
