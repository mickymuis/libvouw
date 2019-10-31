/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include <vouw/encoder.h>
#include <vouw/matrix.h>
#include <vouw/codetable.h>
#include <vouw/equivalence.h>
#include <map>
#include <unordered_map>
#include <bitset>
#include <functional>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>
#include <set>

/* Chrono library used to measure execution time of various functions */
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TimeVarT;

#define duration(a) std::chrono::duration_cast<std::chrono::milliseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()
/* End Chrono part */

VOUW_NAMESPACE_BEGIN

int
overlapCoeff( const Coord2D& piv1, const Coord2D& piv2, const Pattern::BoundsT& b ) {
    int r = /*b.height-1 +*/ (piv2.row() - piv1.row());
    int c = b.width + (piv2.col() - piv1.col());
    int w = 2 * b.width + 1;
    //int h = 2 * b.height;
    return c + r*w;
}

/* class Encoder implementation */

Encoder::Encoder( EquivalenceSet* es ) : 
        m_es( es ),
        m_mat(0),
        m_ct(0),
        m_local( NoLocalSearch ),
        m_heuristic( Best1 ) {
    clear();
}

Encoder::Encoder( Matrix2D* mat, EquivalenceSet* es ) : m_es( es ), m_ct(0) {
    clear();
    setFromMatrix( mat );
}

Encoder::Encoder( Matrix2D* mat, CodeTable* ct, EquivalenceSet* es ) : m_es( es ) {
    clear();
    setFromMatrixUsing( mat, ct );
}

Encoder::~Encoder() {
    clear();
    if( m_es )
        delete m_es;
}

bool
Encoder::isValid() const {
    return m_es && m_mat && m_ct && totalCount();
}

void Encoder::setFromMatrix( Matrix2D* mat, bool useTabu ) {
    clear();
    m_ct = new CodeTable( mat );
    m_instvec.setMatrixSize( mat->width(), mat->height(), mat->base() );
    m_instvec.reserve( mat->width() * mat->height() );
    m_instmat.setRowLength( mat->width() );
    m_mat =mat;
    const MassFunction *massfunc = &mat->distribution();
    
    Matrix2D::ElementT tabuElem;

    if( useTabu ) {
        /* If tabu is enabled, we first find the value with the highest frequency */
        MassFunction::CountT freq =0;
        for( auto pair : massfunc->elements() ) {
            if( pair.second > freq ) {
                freq =pair.second;
                tabuElem =pair.first;
            }
        }
        /* We set the corresponding pattern as tabu and remove the value from the mass function */
        //m_smap[elem].first->setActive( false );
        //m_smap[elem].first->setTabu( true );
        //m_massfunc.setCount( elem, 0 );
        fprintf( stderr, "Singleton with value %d is set as tabu.\n", tabuElem );
    }

    for( int i =0; i < m_mat->height(); i++ ) {
        for( int j =0; j < m_mat->width(); j++ ) {
            Coord2D c = m_mat->makeCoord( i,j );
            Matrix2D::ElementT elem = m_mat->value( c );
            Pattern* p =0;
            Variant* v =0;
            // Try to find if an existing value->pattern mapping exists
            auto it =m_smap.find( elem );
            if( it != m_smap.end() ) { 
                p = (it->second).first; // pattern
                v = (it->second).second; // variant
            }
            else {
                // Use the EquivalenceSet to see if we can use 
                // a variant of an existing pattern
                for( auto&& pair : m_smap ) {
                    v = m_es->makeVariant( *pair.second.first, m_mat, c );
                    if( v->isValid() ) {
                        p =pair.second.first;
                        break;
                    }
                }
                if( !p ) {
                    // We need a new pattern, add it to the code table
                    p = new Pattern( elem, m_mat->width() );
                    addPattern( p );
                    v = m_es->makeNullVariant();
                }

                // Add mapping to the cache
                m_smap[elem] = PatternVariantT(p,v);
            }

            p->usage()++;
            if( useTabu && elem == tabuElem ) {
                p->setTabu( true );
                m_tabuCount++;
            }
            else {
                m_instvec.emplace_back( p, c, v );
                m_instmat.place( m_instvec.size()-1, m_instvec.back() );
                m_instanceCount++;
            }


        }
    }
    std::cerr << "Added " << m_ct->countIfActive() << " patterns in " << totalCount() << " instances." << std::endl;


    m_priorBits =updateCodeLengths();
}

void Encoder::setFromMatrixUsing( Matrix2D* mat, CodeTable* ct ) {

}

void Encoder::setEquivalenceSet( EquivalenceSet* es ) {
    if( m_es ) delete m_es;
    m_es = es;
}

void Encoder::clear() {
    m_mat =nullptr;
    if( m_ct ) {
        delete m_ct;
        m_ct =nullptr;
    }
    m_instvec.clear();
    m_instmat.clear();
    m_smap.clear();
    m_configvec.clear();
    m_errormap.clear();

    m_tabuCount =0;
    m_instanceCount =0;

    m_priorBits =0.0;
    m_isEncoded =false;
    m_lastLabel =0;
    m_decompositions =0;
    m_iteration =0;
}

bool Encoder::encodeStep() { 
    m_iteration ++;
    fprintf( stderr, "\n *** Iteration %d, finding candidates ... ", m_iteration );

    TimeVarT t1 = timeNow();

    rebuildCandidateMap();

    TimeVarT t2 = timeNow();
    std::cerr << "Elapsed time: " << duration( t2-t1 ) << " ms."<< std::endl;
    fprintf( stderr, "Computing gain... " );

    // We keep track of the modelsize during each iteration, because it is expensive to recompute
    int modelSize = m_ct->countIfActive();

    // We estimate the gain for each candidate
    CandidateGainVectorT gainvec;
    for( auto&& pair : m_candidates ) {
        if( pair.second <= 1 ) continue;
        const Candidate& c = pair.first;

        double gain = computeGain( &c, pair.second, modelSize );
        if( gain > 0.0 || m_iteration == 1) {
            gainvec.push_back( CandidateGainT( c, gain ) );
        }
    }

    std::cerr << "Retained " << gainvec.size() << " candidates with positive gain." << std::endl;

    std::sort( gainvec.begin(), gainvec.end(), cg_gain_gt );
  
   /* int bestUsage =0;
    double bestGain =-std::numeric_limits<double>::infinity();
    Candidate bestC;

    if( gainvec.size() ) {
        bestC = gainvec.back().first;
        bestUsage = m_candidates[bestC];
        bestGain = gainvec.back().second;
    }*/

    TimeVarT t3 = timeNow();
    std::cerr << "Elapsed time: " << duration( t3-t2 ) << " ms."<< std::endl;
    //printf( "Estimated gain %f, estimated usage: %d\n", bestGain, bestUsage );


    double totalGain =0.0; int totalMerge =0;
    const int maxMerge = m_heuristic == Best1 ? 1 : gainvec.size();
    std::vector<Pattern*> usedps; // We need indepedent candidates, i.e. disjunct sets of patterns

    for( auto&& cg : gainvec ) {

        if( std::find( usedps.begin(), usedps.end(), cg.first.p1 ) == usedps.end() &&
            std::find( usedps.begin(), usedps.end(), cg.first.p2 ) == usedps.end() ) {
           
            if( m_iteration == 1 && totalMerge > 0 && cg.second < 0 ) break;

            bool ff =false; // Flood fill
            totalGain += processCandidate( cg, ff, modelSize );

            if( ff ) break;

            usedps.push_back( cg.first.p1 );
            usedps.push_back( cg.first.p2 );
            if( ++totalMerge == maxMerge )
                break;
        }
    }


    TimeVarT t4 = timeNow();
    std::cerr << "Merged " << totalMerge << " patterns. Elapsed time: " << duration( t4-t3 ) << " ms."<< std::endl;

    
    /* This part is for statistics only
     * It outputs the number of patterns vs the number of configurations */
    /*int patterns =0, configs =0;
    std::set<ConfigIDT> cfgset;
    for( auto && p : *m_ct ) {
        if( p->isActive() == false || p->size() == 1 || p->isTabu() ) continue;
        if( cfgset.count( p->configuration() ) == 0 ) {
            cfgset.insert( cfgset.end(), p->configuration() );
            configs++;
        }
        patterns++;
    }
    printf( "%d, %d\n", patterns, configs );*/
    /* End statistical part */


    if( totalGain <= 0.0 && m_iteration != 1 ) {
        std::cerr << "No compression gain." << std::endl;

        // Try additional decomposion before giving up
       /* bool prune =false;
        for( auto p : *m_ct ) {
             prune = prunePattern( p, false ) || prune;
        }
        if( prune ) { 
            std::sort( m_instvec.begin(), m_instvec.end() );
            return true;
        }*/

        m_isEncoded =true;
        rebuildInstanceMatrix();

      /*  for( auto&& p : *m_ct ) {
            if( p->isActive() )
                printf( "Pattern #%5d\t usage %d, %dx%d, codeword length %f\n",
                    p->label(), p->usage(), p->bounds().width, p->bounds().height, p->codeLength() );
        }*/
        fprintf( stderr, "Total number of succesfull decompositions: %d\n", m_decompositions );

        return false;
    }
    

    /* Run prunePattern with decomposition */
    /*prunePattern( bestC.p1, false );
    if( bestC.p1 != bestC.p2 )
        prunePattern( bestC.p2, false );*/

    if( m_iteration % 1000 == 0 ) {
        std::cerr << "Rebuilding instance matrix..." << std::endl;
        rebuildInstanceMatrix();
    }
    
    TimeVarT t5 = timeNow();
    std::cerr << "Elapsed time: " << duration( t5-t4 ) << " ms."<< std::endl;
    std::cerr << "Iteration elapsed time: " << duration( t5-t1 ) << " ms."<< std::endl;

    std::cerr << std::flush;
    std::cout << std::flush;
    return true;
}

int 
Encoder::encode() {
    int steps =0;
    
    TimeVarT t = timeNow();

    while( encodeStep() ) steps++;
    //m_mat->unflagAll();

    TimeVarT t2 = timeNow();

    std::cerr << "Total elapsed time: " << duration( t2-t ) << " ms." << std::endl << std::flush;

    return steps;
}

void
Encoder::reencode() {

    m_instvec.clear();
    m_instmat.clear();

    /* Iterate over all patterns in the CT, sorted descending by size */
    m_ct->sortBySizeDesc();
    for( Pattern* p : *m_ct ) {

        if( !p->isActive() ) continue;
        if( p->isTabu() ) continue;
        p->setActive( false );
        p->usage() =0;

        for( int i =-p->bounds().rowMin; i < m_mat->height() - p->bounds().rowMax; i++ ) {
            for( int j =-p->bounds().colMin; j < m_mat->width() - p->bounds().colMax; j++ ) {
                Coord2D c =m_mat->makeCoord( i, j );

                /* Test if p fits the matrix at pivot c */
                if( p->test( m_mat, c, true, false, true ) ) {
                    p->apply( m_mat, c, true, true, false );

                    p->usage()++;
                    p->setActive( true );
                    m_instvec.emplace_back( p, c, m_es->makeNullVariant() );
                }
            }
        }
    }
    rebuildInstanceMatrix( true );

    m_mat->unflagAll();
    updateCodeLengths();
}

Matrix2D* 
Encoder::decode() {

}

/* Private functions for class Encoder */

void
Encoder::rebuildCandidateMap() {
    int progress =0, total = totalCount();

    m_candidates.clear();
    m_overlapMask.resize( m_instvec.size() );
    //m_overlapMask.assign( m_instvec.size(), Instance::BitmaskT() );
    for( auto && mask : m_overlapMask ) {
        mask.assign( mask.size(), false );
    }
    if( m_instvec.size() != m_instanceMarker.size() ) {
        m_instanceMarker.resize( m_instvec.size() );
        m_instanceMarker.assign( m_instvec.size(), 1UL << 31 );
    }

    InstanceVector::IndexT odd =0;
    if( m_iteration % 2 != 0 )
        odd = 1UL << 30;

    for( int i =0; i < m_instvec.size(); i++ ) {
        Instance &r1 = m_instvec[i];
        if( r1.empty() ) continue;

        Pattern* p1 =r1.pattern();
        assert( p1->isActive() );
       // if( p1->isTabu() ) continue;

        if( progress++ % (total/10+1) == 0 )
          std::cerr << progress*100/total << "% ";

        int overlap_coeff =0; // Only if p1 == p2

        // Get the periphery of r1's pattern
        const Vouw::Pattern::PeripheryT& post =p1->periphery( Vouw::Pattern::PosteriorPeriphery );

        // Iterate over all instances in r1's periphery
        for( int j =0; j < post.size(); j++ ) {
            Vouw::Coord2D coord = post[j].abs( r1.pivot() );
            InstanceMatrix::IndexT idx = m_instmat[coord];
            if( idx == m_instmat.empty ) continue;

            if( m_instanceMarker[idx] == (i | odd) ) continue;
            m_instanceMarker[idx] =(i | odd);

            Instance& r2 = m_instvec[idx];
            if( r2.empty() ) continue; // No instance at this coord

           // if( r2.marker() == i ) continue;
           // r2.marker() =i; // Make sure we do not visit this instance again

            if( r2.pivot().row() < r1.pivot().row() ) continue; // Edge case in the periphery representation

            Pattern* p2 =r2.pattern();

            if( p2->isTabu() ) continue;

            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );
            if( p1 == p2 ) {
                overlap_coeff =overlapCoeff( r1.pivot(), r2.pivot(), p1->bounds() );

                /*r1.bitmaskGrow( overlap_coeff+1 );
                if( r1.bitmask()[overlap_coeff] ) continue;
                r2.bitmaskGrow( overlap_coeff+1 );
                r2.bitmask()[overlap_coeff] = true;*/
                if( m_overlapMask[i].size() < overlap_coeff+1 ) m_overlapMask[i].resize( overlap_coeff+1 );
                if( m_overlapMask[i][overlap_coeff] ) continue;
                if( m_overlapMask[idx].size() < overlap_coeff+1 ) m_overlapMask[idx].resize( overlap_coeff+1 );
                //m_overlapMask[idx].reserve( overlap_coeff+1 );
                m_overlapMask[idx][overlap_coeff] = true;
            }
            
            // Increment the usage count of this particular combination
            Candidate c = { p1, p2, (Variant*)r1.variant(), (Variant*)r2.variant(), offset };
            m_candidates[c]++;

        }
    }

    std::cerr << std::endl << m_candidates.size() << " canditates found. Bucket count: " << m_candidates.bucket_count() << std::endl;
  /*  for( auto && inst : m_instvec ) { 
        inst.marker() = -1;
        inst.bitmask().clear();
    }*/
}

double
Encoder::processCandidate( const CandidateGainT& pair, bool& usedFloodFill, int& modelSize ) {


    const Candidate& cand = pair.first;
    double gain = pair.second;
    
    fprintf( stderr, "\tMerging '%4d' and '%4d' (%4d,%4d), predicted gain %.3f, ", 
            cand.p1->label(), cand.p2->label(), cand.offset.row(), cand.offset.col(), gain);

    InstanceIndexVectorT insts; // Vector of instances changed by the merge 

    mergePatterns( &cand, insts );
    modelSize++;
    modelSize -= (int)prunePattern( cand.p1, true );
    if( cand.p1 != cand.p2 )
        modelSize -= (int)prunePattern( cand.p2, true );
    
    // Debug only
    double oldBits = m_encodedBits;
    updateCodeLengths();
    fprintf( stderr, "actual gain: %.3f\n", oldBits - m_encodedBits );
    

    while( m_local == FloodFill && floodFill( insts, modelSize ) ) usedFloodFill =true;
//    if( floodFill( prime ) ) usedFloodFill = true;


/*
    if( std::abs((oldBits - m_encodedBits) - gain ) > 0.0001 ) {
        printf( "\n*** Computed gain doesn't match! Panic! ***\n\n" );
        cand.p1->debugPrint();
        if( cand.p1 != cand.p2 )
            cand.p2->debugPrint();
        for( auto&& p : *m_ct ) {
            printf( "Pattern #%5d\t usage %d, %dx%d, codeword length %f (%s)\n",
                p->label(), p->usage(), p->bounds().width, p->bounds().height, p->codeLength(), p->isActive() ? "active" : "inactive");
        }
        return false;
    }*/
    return oldBits - m_encodedBits;
}

void 
Encoder::mergePatterns( const Candidate* c, InstanceIndexVectorT& changelist ) {
    // Create the merged pattern
    Pattern* p_union = new Pattern( *c->p1, *c->v1, *c->p2, *c->v2, c->offset );
    addPattern( p_union );

    for( int i =0; i < m_instvec.size(); i++ ) {
        Instance &r1 = m_instvec[i];
        if( r1.empty() ) continue;

        Pattern* p1 =r1.pattern();
        if( p1 != c->p1 ) continue;
        
        // Get the periphery of r1's pattern
        const Vouw::Pattern::PeripheryT& post =p1->periphery( Vouw::Pattern::PosteriorPeriphery );

        // Iterate over all instances in r1's periphery
        for( auto && p_offset : post ) {
            Vouw::Coord2D coord = p_offset.abs( r1.pivot() );
            InstanceMatrix::IndexT idx =m_instmat[coord];
            if( idx == m_instmat.empty ) continue;
            Instance& r2 =m_instvec[idx];
            if( r2.empty() ) continue; // No instance at this coord

            Pattern* p2 =r2.pattern();
            if( p2 != c->p2 ) continue;

            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );

            if( offset != c->offset ) continue;
            
            Variant* v;
            if( r1.variant()->hash() != c->v1->hash() || r2.variant()->hash() != c->v2->hash() ) {
                v =m_es->makeVariant( *p_union, *p1, *p2, *r1.variant(), *r2.variant(), offset );
                if( !v->isValid() ) continue;
            } else {
                v =m_es->makeNullVariant();
            }

            Coord2D pivot =r1.pivot();

            r2.clear(); // Mark for deletion later on
            m_instvec[i] = Instance( p_union, pivot, v );
            m_instmat.place( i, m_instvec[i] );
            changelist.push_back( i );

            p_union->usage()++;
            p1->usage()--;
            p2->usage()--;
            m_instanceCount--;
            break;
        }
    }
}

void
Encoder::addPattern( Pattern* p ) {
    p->setLabel( m_lastLabel++ );
    m_ct->push_back( p );
    Configuration c( *p );
    auto it =std::find( m_configvec.begin(), m_configvec.end(), c ); 
    if( it == m_configvec.end() ) {
        p->setConfiguration( m_configvec.size() ); // Index of the new element
        m_configvec.push_back( c );
    } else {
        p->setConfiguration( it - m_configvec.begin() );
    }
}

/** Noisy flood fill */
bool
Encoder::noisyFloodFill( InstanceIndexVectorT& insts, int& modelSize ) {

    if( insts.empty() ) return false;

    Pattern *p1 =m_instvec[insts[0]].pattern();

    // Obtain the peripheries of p1
    Pattern::PeripheryT peri =p1->periphery( Pattern::AnteriorPeriphery );
    peri.insert( peri.end(), p1->periphery( Pattern::PosteriorPeriphery ).begin(), p1->periphery( Pattern::PosteriorPeriphery ).end() );

    int totalMerges =0;
    Pattern::OffsetT p_shift; // Value with which we shift each periphery offset, in case we need to swap pivots 

    for( auto p_offset : peri ) {
        // The offsets in the periphery may have changed as we may have shifted the pivot of the original pattern
        // We apply this translation to accomodate for these changes
        p_offset =p_offset.translate( p_shift );
        Pattern::OffsetT i_offset;      // The actual offset between p1's and p2's pivots
        Pattern *p2 = NULL, *p_union;   // Second pattern and the final union pattern
        Variant *v;                     // ...
        bool is_anterior =false;        // Are we looking in the anterior or posterior periphery?
        Candidate c;                    // Just a struct for holding p1,p2 and i_offset
        ConfigIDT cfg = -1;             // The configuration needs to be the same for all candidate patterns
        double gain =0.0;               // Gain computed for this merge
        using PmapT = std::map<Pattern*,int>;
        PmapT pmap;                     // We keep track of how many times each specific pattern is encountered

        // We need all instances to have the same neighboring pattern configuration/instance at the same offset
        for( auto i : insts ) {
            const Instance& r1 =m_instvec[i];
            Vouw::Coord2D coord = p_offset.abs( r1.pivot() );
            InstanceMatrix::IndexT j =m_instmat[coord];
            if( j == m_instmat.empty ) goto NO_MATCH;
            const Instance& r2 =m_instvec[j];
            if( r2.empty() ) goto NO_MATCH;
            if( r2.pattern() == p1 ) goto NO_MATCH;
            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );

            // Configurations and offsets need to match for all candidates
            if( cfg != -1 ) {
                if( cfg != r2.pattern()->configuration() || i_offset != offset ) goto NO_MATCH;
            } else {
                cfg = r2.pattern()->configuration();
                i_offset =offset;
            }

            // Add this specific pattern variety to the map
            pmap[r2.pattern()]++;
        }

        // First, we try to discover the most prevalent pattern
        { 
            auto best_pair = std::max_element(pmap.begin(), pmap.end(),
                [](const PmapT::value_type& p1, const PmapT::value_type& p2) {
                    return p1.second < p2.second; } );

            p2 =(*best_pair).first;
        }

        // Compute the additional cost of encoding errors for all patterns that do not match p2
        {
            for( auto pair : pmap ) {
                if( pair.first == p2 ) continue;
                int dist = errorCount( *p2, *pair.first );
                assert( dist != -1 );
                gain -= dist * pair.second * 
                    errorCodeLength( m_mat->width(), m_mat->height(), m_mat->distribution().uniqueElements() );
            }
            if( gain != 0.0 ) {
               // fprintf( stderr, "\tfloodFill: additional cost for error-encoding: %.3f\n", gain ); 
               // goto NO_MATCH; // TODO not implemented
            }
        }


        // Compute the expected gain for this 'candidate'
        c = { p1, p2, nullptr, nullptr, i_offset };
        gain +=computeGain( &c, insts.size(), modelSize );
        //fprintf( stderr, "\tfloodFill: expecting %.3f bits gain, ", gain );

        if( gain < 0.0 ) {
            //fprintf( stderr, "rejected.\n");
            goto NO_MATCH;
        }

        // We have a match, make a new pattern
        
        v =m_es->makeNullVariant();
        // Swap p1 and p2 if the offset is negative (to preserve pivot in row 0)
        is_anterior = (i_offset.row() < 0) || (i_offset.row() == 0 && i_offset.col() < 0);
        if( is_anterior ) {
            p_union =new Pattern( *p2, *v, *p1, *v, i_offset.negate() );
            p_shift =p_shift.translate( i_offset.negate() ); // Fix all periphery offset to come...
        }
        else
            p_union =new Pattern( *p1, *v, *p2, *v, i_offset );
        p1->usage() =0;
        p1->setActive( false );
        /*p2->usage() -=insts.size();
        if( p2->usage() == 0 ) {
            p2->setActive( false );
            modelSize--;
        }*/
        p_union->usage() = insts.size();

        addPattern( p_union );

        // Apply the merge
        for( auto & i : insts ) {
            Instance& r1 =m_instvec[i];
            Vouw::Coord2D coord = p_offset.abs( r1.pivot() );
            InstanceMatrix::IndexT i2 =m_instmat[coord];
            Instance& r2 =m_instvec[i2];
            
            Coord2D pivot = is_anterior ? r2.pivot() : r1.pivot();

            r2.pattern()->usage()--;
            if( r2.pattern()->usage() == 0 ) {
                r2.pattern()->setActive( false );
                modelSize--;
            }

            if( r2.pattern() != p2 ) {
                // Encode error
                errorMapDelta( m_errormap, *p2, *r2.pattern(), r2.pivot() );
            }

            if( is_anterior ) {
                r1.clear();
                i = i2;
            }
            else r2.clear(); // Mark for deletion later on
            m_instvec[i] = Instance( p_union, pivot, v );
            m_instmat.place( i, m_instvec[i] );

            m_instanceCount--;
        }
        totalMerges++;
        p1 =p_union;
        // Debug only
        {
            double oldBits = m_encodedBits;
            updateCodeLengths();
            //fprintf( stderr, "actual gain: %.3f\n", oldBits - m_encodedBits );
        }
NO_MATCH:;
    }

    //fprintf( stderr, "\tfloodFill: merged %d instances with %d adjacent instances (%d total merges).\n",
    //        insts.size(), totalMerges, insts.size()*totalMerges );

    assert( modelSize == m_ct->countIfActive() );

    return totalMerges != 0;
}


/** Flood fill */
bool
Encoder::floodFill( InstanceIndexVectorT& insts, int& modelSize ) {

    if( insts.empty() ) return false;

    Pattern *p1 =m_instvec[insts[0]].pattern();

    // Obtain the peripheries of p1
    Pattern::PeripheryT peri =p1->periphery( Pattern::AnteriorPeriphery );
    peri.insert( peri.end(), p1->periphery( Pattern::PosteriorPeriphery ).begin(), p1->periphery( Pattern::PosteriorPeriphery ).end() );

    int totalMerges =0;
    Pattern::OffsetT p_shift; // Value with which we shift each periphery offset, in case we need to swap pivots 

    for( auto p_offset : peri ) {
        p_offset =p_offset.translate( p_shift );
        Pattern::OffsetT i_offset;
        Pattern *p2 = NULL, *p_union;
        Variant *v;
        bool is_anterior =false;
        Candidate c;
        double gain;

        // We need all instances to have the same neighboring pattern/instance at the same offset
        for( auto i : insts ) {
            const Instance& r1 =m_instvec[i];
            Vouw::Coord2D coord = p_offset.abs( r1.pivot() );
            InstanceMatrix::IndexT j =m_instmat[coord];
            if( j == m_instmat.empty ) goto NO_MATCH;
            const Instance& r2 =m_instvec[j];
            if( r2.empty() ) goto NO_MATCH;
            if( r2.pattern() == p1 ) goto NO_MATCH;
            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );

            if( p2 != NULL ) {
                if( p2 != r2.pattern() || i_offset != offset ) goto NO_MATCH;
            } else {
                p2 =r2.pattern();
                i_offset =offset;
            }
        }

        // Compute the expected gain for this 'candidate'
        c = { p1, p2, nullptr, nullptr, i_offset };
        gain =computeGain( &c, insts.size(), modelSize );
        //fprintf( stderr, "\tfloodfill: expecting %.3f bits gain, ", gain );

        if( gain < 0.0 ) {
            //fprintf( stderr, "rejected.\n");
            goto NO_MATCH;
        }

        // We have a match, make a new pattern
        
        v =m_es->makeNullVariant();
        // Swap p1 and p2 if the offset is negative (to preserve pivot in row 0)
        is_anterior = (i_offset.row() < 0) || (i_offset.row() == 0 && i_offset.col() < 0);
        if( is_anterior ) {
            p_union =new Pattern( *p2, *v, *p1, *v, i_offset.negate() );
            p_shift =p_shift.translate( i_offset.negate() ); // Fix all periphery offset to come...
        }
        else
            p_union =new Pattern( *p1, *v, *p2, *v, i_offset );
        p1->usage() =0;
        p1->setActive( false );
        p2->usage() -=insts.size();
        if( p2->usage() == 0 ) {
            p2->setActive( false );
            modelSize--;
        }
        p_union->usage() = insts.size();

        addPattern( p_union );

        // Apply the merge
        for( auto & i : insts ) {
            Instance& r1 =m_instvec[i];
            Vouw::Coord2D coord = p_offset.abs( r1.pivot() );
            InstanceMatrix::IndexT i2 =m_instmat[coord];
            Instance& r2 =m_instvec[i2];
            
            Coord2D pivot = is_anterior ? r2.pivot() : r1.pivot();

            if( is_anterior ) {
                r1.clear();
                i = i2;
            }
            else r2.clear(); // Mark for deletion later on
            m_instvec[i] = Instance( p_union, pivot, v );
            m_instmat.place( i, m_instvec[i] );

            m_instanceCount--;
        }
        totalMerges++;
        p1 =p_union;
        // Debug only
       /* {
            double oldBits = m_encodedBits;
            updateCodeLengths();
            fprintf( stderr, "actual gain: %.3f\n", oldBits - m_encodedBits );
        }*/
NO_MATCH:;
    }

    //fprintf( stderr, "\tfloodFill: merged %d instances with %d adjacent instances (%d total merges).\n",
    //        insts.size(), totalMerges, insts.size()*totalMerges );

    assert( modelSize == m_ct->countIfActive() );

    return totalMerges != 0;
}

double
Encoder::updateCodeLengths() {
    m_ct->updateCodeLengths( totalCount(), m_mat->distribution() );
    return (m_encodedBits =m_ct->totalLength());
}

double
Encoder::computeCandidateEntryLength( const Candidate* c, bool debugPrint ) {
    Pattern::BoundsT bounds = {
        std::min( c->p1->bounds().rowMin, c->p2->bounds().rowMin + c->offset.row() ),
        std::max( c->p1->bounds().rowMax, c->p2->bounds().rowMax + c->offset.row() ),
        std::min( c->p1->bounds().colMin, c->p2->bounds().colMin + c->offset.col() ),
        std::max( c->p1->bounds().colMax, c->p2->bounds().colMax + c->offset.col() ),
        0, 0 };
    bounds.computeDimensions();

    double bits = c->p1->entryValuesLength() + c->p2->entryValuesLength();
    bits += Pattern::entryOffsetsLength( bounds.width, 
                                         bounds.height, 
                                         c->p1->size() + c->p2->size(), 
                                         m_mat->width(), m_mat->height() );

//    printf( "New pattern bounds wxh: %d x %d\n", bounds.width, bounds.height );
    return bits;
}

/*
 * Calculate the gain in encoding size if patterns p1 and p2 were replaced by their union.
 * This union is assumed to have estimated usage p_usage,
 * which is assumed to be less or equal than the usages of p1 and p2.
 * The return value is the difference in encoding side in bits.
 */
double
Encoder::computeGain( const Candidate* c, int usage, int modelSize, bool debugPrint ) {

    double bits = 0.0;

    // Compute the total size of the new model
    const int num_patterns = c->p1 == c->p2 ? 1 : 2;

    int newModelSize = modelSize + 1;
    if( num_patterns == 2 ) {
        newModelSize -= ((int)(c->p1->usage() - usage == 0 /*&& c->p1->size() != 1*/) 
                      + (int)(c->p2->usage() - usage == 0 /*&& c->p2->size() != 1*/));
    } else {
        newModelSize -= (int)(c->p1->usage() - 2*usage == 0 /*&& c->p1->size() != 1*/);
    }

    //printf( "(%d,%d) new model size %d (was %d) ", c->offset.row(), c->offset.col(), newModelSize, m_ct->countIfActive() );

    // The new size of the instance set is determined by the predicted usage of the union pattern
    const int totalInstances = totalCount() - usage;

    // Temporary bias to account for the encoding of null-variants
    //bits += usage;

    // For the instance set to decode correctly, its cardinality need to be known in advance
    //bits += uintCodeLength( totalCount() ) - uintCodeLength( totalInstances );
    //bits += uintCodeLength( binom( m_mat->width() * m_mat->height(), totalCount() ) )
    //     -  uintCodeLength( binom( m_mat->width() * m_mat->height(), totalInstances ) );
    // Idem for the model
    bits += uintCodeLength( modelSize ) - uintCodeLength( newModelSize );

    // Recompute code table and instance codewords based on the new model's size
    // We use the fact that -log(a/b) = log(b)-log(a)
    // Here we subtract the log(b) component and replace it with log(b+k)
    //const int f =totalCount()+m_ct->countIfActive();
    //bits += f * (log2( totalCount() ) - log2( totalInstances ));
    bits += (lgamma( (double)totalCount() + pseudoCount * (double)modelSize ) / log(2) - lgamma( pseudoCount * (double)modelSize ) / log(2)) 
            -(lgamma( (double)totalInstances + pseudoCount * (double)newModelSize ) / log(2) - lgamma( pseudoCount * (double)newModelSize ) / log(2)); 

 

   // const int  totalInstances = m_mat->count();

    for( int i =0; i < num_patterns; i++ ) {
        Pattern* p = (&c->p1)[i];
        
        double codeLength =Pattern::codeLength( p->usage(), totalInstances, newModelSize );
        // Step 1. remove the bits of the old instances completely
        bits += codeLength;
        
        // Step 2. Re-add the old patterns based on their new usage
        int p_usage = p->usage() - (usage * (num_patterns == 1 ? 2 : 1) );
        
        double newCodeLength =Pattern::codeLength( p_usage, totalInstances, newModelSize );

        if( p_usage > 0 ) {
            // Instance set part
            bits -= newCodeLength;
        }
        else /*if( p->size() > 1 )*/ { 
            // Pattern p1 will be removed from the code table
            bits += p->entryLength();
        }
/*        else {
            // Singleton patterns will remain in the code table
            bits -= newCodeLength;
        }*/
    }

    // Step 3. add the length from the union pattern p
    double p_codeLength = Pattern::codeLength( usage, totalInstances, newModelSize );
    // Instance set part
    bits -= p_codeLength;
    // Code table part
    bits -= computeCandidateEntryLength( c, debugPrint );

/*    Pattern p_union( *c->p1, *c->v1, *c->p2, *c->v2, c->offset );
    p_union.setUsage( usage );
    bits -= (p_union.updateCodeLength( totalInstances ) + newBitsPerPivot) * usage;
    bits -= p_union.updateEntryLength( m_mat->base() ) + p_union.codeLength();*/
    
    
    return bits;
}

double 
Encoder::computeDecompositionGain( const Pattern* p, int modelSize, bool debugPrint ) {

    // We use a recursive stateless lambda to compute the patterns
    // that make the composition of @p
    PatternUsageMapT decomp;
    static bool (*decompose)(const Pattern*, PatternUsageMapT&, int&) =
        [](const Pattern* p, PatternUsageMapT& new_usage, int& n)->bool {
            const Pattern::CompositionT& comp = p->composition();
            if( !comp.p1 || !comp.p2 ) return false;

            const Pattern* p_new[2] = { comp.p1, comp.p2 };
            for( int i =0; i < 2; i++ ) {
                if( p_new[i]->isActive() ) {
                    new_usage[(Pattern*)p_new[i]]++;
                    n++;
                } else {
                    if (!decompose( p_new[i], new_usage, n ) ) return false;
                }
            }
            return true;
        };

    int n =0;
    if( !decompose( p, decomp, n ) || !n ) return 0.0;

    // New pivots can be sized differently because their size depends on the number of instances
    const int newModelSize = modelSize -1;
    const int totalInstances = totalCount() + p->usage() * (n-1);
    double bits = 0.0;
    
    // For the instance set to decode correctly, its cardinality need to be known in advance
    //bits += uintCodeLength( totalCount() ) - uintCodeLength( totalInstances );
    // Idem for the model
    bits += uintCodeLength( modelSize ) - uintCodeLength( newModelSize );

    // Recompute code table and instance codewords based on the new model's size
    // We use the fact that -log(a/b) = log(b)-log(a)
    // Here we subtract the log(b) component and replace it with log(b+k)
    const int f =totalCount()+m_ct->countIfActive();
    bits += (lgamma( (double)totalCount() + pseudoCount * (double)modelSize ) / log(2) - lgamma( pseudoCount * (double)modelSize ) / log(2)) 
            -(lgamma( (double)totalInstances + pseudoCount * (double)newModelSize ) / log(2) - lgamma( pseudoCount * (double)newModelSize ) / log(2)); 

    // Step 1. remove the bits of the instances and code table codeword of @p completely
    double codeLength =Pattern::codeLength( p->usage(), totalInstances, newModelSize );
    // Instance set part
    bits += codeLength;
    // Code table part
    bits += p->entryLength();
    
    // Step 2. Replace the instances of @p by its decomposition
    for( auto&& pair : decomp ) {
        Pattern* ps = pair.first;
        int newUsage = ps->usage() + p->usage() * pair.second;
        double oldCodeLength = Pattern::codeLength( ps->usage(), totalInstances, modelSize );
        double newCodeLength = Pattern::codeLength( newUsage, totalInstances, newModelSize );

        if( debugPrint )
            fprintf( stderr, "--- #%d usage +%d * %d\n", ps->label(), pair.second, p->usage() );

        // Add the new instances
        bits += oldCodeLength;
        bits -= newCodeLength;
    }
    return bits;
}

bool
Encoder::prunePattern( Pattern* p, bool onlyZeroPattern ) {
    //if( p->size() == 1 ) return;
    if( p->usage() == 0 ) {
        //m_ct->remove( p );
        //delete p;
        p->setActive( false );
        return false || onlyZeroPattern;
    }
    if( onlyZeroPattern || p->size() == 1 || p->isActive() == false ) return false;

    int modelSize = m_ct->countIfActive();
    double g =computeDecompositionGain( p, modelSize );
    if( g > 0.0 ) {
        fprintf( stderr, "The decompositon of %d would result in %f bits gain.\n", p->label(), g );
        for( auto&& r : m_instvec ) {
            if( !r.empty() && r.pattern() == p ) {
                decompose( r );
                r.clear();
                ((Pattern*)p)->usage()--;
            }
        }
        //m_instvec.eraseIfNull( m_instvec.begin(), m_instvec.end() );
        p->setActive( false );
        m_decompositions++;
        
        double oldBits = m_encodedBits;
        double oldIBits = m_instvec.totalCodeLength();
        double oldCBits = m_ct->totalLength();
        updateCodeLengths();
        printf( "Actual decomposition gain: %f (instance set %f, code table %f)\n", 
                oldBits - m_encodedBits, oldIBits - m_instvec.totalCodeLength() , oldCBits - m_ct->totalLength() );
        if( std::abs( g - (oldBits-m_encodedBits) ) > 0.0001 ) {
            printf( "\n*** Computed decomposition gain doesn't match! Panic! ***\n\n" );
        }
        return true;
    }

    return false;
}

/** Decomposes instance @r into instances r1 and r2, defined by the composition on @r.pattern()
 *  If r1 and/or r2 contain an inactive pattern, each is decomposed recursively.
 *  The final instances are covering @r completely and are pushed to the back of the list.
 *  @r is not removed from the list and this function leaves the list in an unsorted state.
 */
void
Encoder::decompose( Instance& inst ) {

    const Pattern* p =inst.pattern();
    const Pattern::CompositionT& comp = p->composition();
    if( !comp.isValid() ) return;

    Coord2D pivot2 = comp.offset.abs( inst.pivot() );

    m_instanceCount++;

    Instance inst1( (Pattern*)comp.p1, inst.pivot(), comp.v1 );
    if( !comp.p1->isActive() ) {
        decompose( inst1 );
    } else {
        ((Pattern*)comp.p1)->usage()++;
        m_instvec.push_back( inst1 );
        //m_instmat.place( inst1 );
    }

    Instance inst2( (Pattern*)comp.p2, pivot2, comp.v2 );
    if( !comp.p2->isActive() ) {
        decompose( inst2 );
    } else {
        ((Pattern*)comp.p2)->usage()++;
        m_instvec.push_back( inst2 );
        //m_instmat.place( inst2 );
    }
}

void 
Encoder::rebuildInstanceMatrix( bool sort ) {

    // Free up space by compacting the instance vector
    m_instvec.eraseIfEmpty( m_instvec.begin(), m_instvec.end() );

    // The instance vector needs to be sorted at all time, 
    // if random insertion has taken place it needs to be resorted
    if( sort ) {
        std::sort( m_instvec.begin(), m_instvec.end() );
    }

    // Now the unfortunate part, repopulate the matrix with the altered indices from the array
    for( int i =0; i < m_instvec.size(); i++ ) {
        m_instmat.place( i, m_instvec[i] );
    }
    m_instanceCount = m_instvec.size(); // This rarely equals, but now it does

}

VOUW_NAMESPACE_END
