/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once
#include "vouw.h"
#include "pattern.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

VOUW_NAMESPACE_BEGIN

struct Candidate {
    Pattern* p1, *p2;
    Variant* v1, *v2;
    Pattern::OffsetT offset;
};

inline bool operator==( const Candidate& c1, const Candidate& c2 ) {
    return (
            c1.p1 == c2.p1 &&
            c1.p2 == c2.p2 &&
            c1.v1->hash() == c2.v1->hash() &&
            c1.v2->hash() == c2.v2->hash() &&
            c1.offset == c2.offset );
}


// https://stackoverflow.com/a/34006336
#define HASH_SEED 1009
#define HASH_FACTOR 9176

struct CandidateHash {
    std::size_t operator()( const Candidate& c ) const {
        std::size_t hash = HASH_SEED;
        hash = hash * HASH_FACTOR + std::hash<int>()(c.offset.position());
        hash = hash * HASH_FACTOR + std::hash<Pattern*>()(c.p1);
        hash = hash * HASH_FACTOR + std::hash<Pattern*>()(c.p2);
        //hash = hash * HASH_FACTOR + std::hash<int>()(c.v1->hash());
        //hash = hash * HASH_FACTOR + std::hash<int>()(c.v2->hash());
        return hash;
    }
};

typedef std::unordered_map<Candidate,int,CandidateHash> CandidateMapT;
typedef std::pair<Candidate,double> CandidateGainT;
typedef std::vector<CandidateGainT> CandidateGainVectorT;

inline bool cg_gain_lt( const CandidateGainT& cg1, const CandidateGainT& cg2 ) {
    return cg1.second < cg2.second;
}

inline bool cg_pattern_size_gt( const CandidateGainT& cg1, const CandidateGainT& cg2 ) {
    int pmax1 = std::max( cg1.first.p1->size(), cg1.first.p2->size() );
    int pmax2 = std::max( cg2.first.p1->size(), cg2.first.p2->size() );
    if( pmax1 == pmax2 )
        return cg1.second < cg2.second;

    return pmax1 > pmax2;
}

VOUW_NAMESPACE_END
