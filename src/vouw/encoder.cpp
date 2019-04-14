/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
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
        m_ct(0) {
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
    return m_es && m_mat && m_ct && m_instlist.totalCount();
}

void Encoder::setFromMatrix( Matrix2D* mat, bool useTabu ) {
    //std::map<Matrix2D::ElementT,std::pair<Pattern*,Variant>> smap; // Singleton equivalence mapping

    clear();
    m_ct = new CodeTable( mat );
    m_instlist.setMatrixSize( mat->width(), mat->height(), mat->base() );
    m_instlist.reserve( mat->width() * mat->height() );
    m_instmat.setRowLength( mat->width() );
    m_mat =mat;
    m_massfunc = &mat->distribution();
    
    Matrix2D::ElementT tabuElem;

    if( useTabu ) {
        /* If tabu is enabled, we first find the value with the highest frequency */
        MassFunction::CountT freq =0;
        for( auto pair : m_massfunc->elements() ) {
            if( pair.second > freq ) {
                freq =pair.second;
                tabuElem =pair.first;
            }
        }
        /* We set the corresponding pattern as tabu and remove the value from the mass function */
        //m_smap[elem].first->setActive( false );
        //m_smap[elem].first->setTabu( true );
        //m_massfunc.setCount( elem, 0 );
        printf( "Singleton with value %d is set as tabu.\n", tabuElem );
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
                    p->setLabel( m_lastLabel++ );
                    m_ct->push_back( p );
                    v = m_es->makeNullVariant();
                }

                // Add mapping to the cache
                m_smap[elem] = PatternVariantT(p,v);
            }

            p->usage()++;
            if( useTabu && elem == tabuElem ) {
                p->setTabu( true );
                m_instlist.tabuCount()++;
            }
            else {
                m_instlist.push_back( new Instance( p, c, v, false ) );
                m_instmat.place( m_instlist.back() );
            }


        }
    }
    std::cout << "Added " << m_ct->countIfActive() << " patterns in " << m_instlist.totalCount() << " instances." << std::endl;


    m_priorBits =updateCodeLengths();
}

void Encoder::setFromMatrixUsing( Matrix2D* mat, CodeTable* ct ) {

}

void Encoder::setEquivalenceSet( EquivalenceSet* es ) {
    if( m_es ) delete m_es;
    m_es = es;
}

void Encoder::clear() {
    m_mat =0;
    if( m_ct )
        delete m_ct;
    m_instlist.deleteAll();
    m_instmat.clear();
    m_smap.clear();
    m_massfunc = NULL;

    m_priorBits =0.0;
    m_isEncoded =false;
    m_lastLabel =0;
    m_decompositions =0;
    m_iteration =0;
}

bool Encoder::encodeStep() { 
    //std::vector<bool> pattern_mask( m_lastLabel );
    m_iteration ++;

    int progress =0, total = m_instlist.totalCount();
    fprintf( stderr, "\n *** Iteration %d, finding candidates ... ", m_iteration );

    TimeVarT t1 = timeNow();

    m_candidates.clear();

    for( int i =0; i < m_instlist.size(); i++ ) {
        Instance *r1 = m_instlist[i];

        Pattern* p1 =r1->pattern();
        assert( p1->isActive() );
        if( p1->isTabu() ) continue;

        if( progress++ % (total/10+1) == 0 )
          std::cerr << progress*100/total << "% ";

        int overlap_coeff =0; // Only if p1 == p2

        // Get the periphery of r1's pattern
        const Vouw::Pattern::PeripheryT& post =p1->periphery( Vouw::Pattern::PosteriorPeriphery );

        // Iterate over all instances in r1's periphery
        //for( auto && p_offset : post ) {
        for( int j =0; j < post.size(); j++ ) {
            Vouw::Coord2D coord = post[j].abs( r1->pivot() );
            Instance* r2 = m_instmat[coord];
            if( r2 == NULL ) continue; // No instance at this coord

            if( r2->marker() == i ) continue;
            r2->marker() =i; // Make sure we do not visit this instance again

            if( r2->pivot().row() < r1->pivot().row() ) continue; // Edge case in the periphery representation

            Pattern* p2 =r2->pattern();

            if( p2->isTabu() ) continue;

            Pattern::OffsetT offset( r1->pivot(), r2->pivot() );
            if( p1 == p2 ) {
                overlap_coeff =overlapCoeff( r1->pivot(), r2->pivot(), p1->bounds() );

                r1->bitmaskGrow( overlap_coeff+1 );
                if( r1->bitmask()[overlap_coeff] ) continue;
                r2->bitmaskGrow( overlap_coeff+1 );
                r2->bitmask()[overlap_coeff] = true;
            }
            
            // Increment the usage count of this particular combination
            Candidate c = { p1, p2, (Variant*)r1->variant(), (Variant*)r2->variant(), offset };
            m_candidates[c]++;

        }
    }
    for( auto && inst : m_instlist ) 
        inst->marker() = -1;

    std::cerr << std::endl << m_candidates.size() << " canditates found. Bucket count: " << m_candidates.bucket_count() << std::endl;
    m_instlist.clearBitmasks();

    TimeVarT t2 = timeNow();
    std::cerr << "Elapsed time: " << duration( t2-t1 ) << " ms."<< std::endl;
    fprintf( stderr, "Computing gain... " );

    int modelSize = m_ct->countIfActive();
    int bestUsage =0;
    double bestGain =-std::numeric_limits<double>::infinity();
    Candidate bestC;

    for( auto&& pair : m_candidates ) {
       // if( pair.second < bestUsage-1 ) continue; // Extra greedy selection by usage
        if( pair.second <= 1 ) continue;
        const Candidate& c = pair.first;

        // Fractured pattern, merge it without further considerations
       /* if( c.p1->usage() == c.p2->usage() && c.p1->usage() == pair.second ) {
            bestC =c;
            bestUsage =pair.second;
            bestGain =computeGain( &c, pair.second, modelSize );
            break;
        }*/

        double gain = computeGain( &c, pair.second, modelSize );
        /*fprintf( stderr, "Candidate: %d + %d (%d,%d), usage %d, gain %f\n",
                c.p1->label(),
                c.p2->label(),
                c.offset.row(),
                c.offset.col(),
                pair.second,
                gain );*/
        if( gain >= bestGain ) {
            bestGain =gain;
            bestC =c;
            bestUsage =pair.second;
        }
    }

    TimeVarT t3 = timeNow();
    std::cerr << "Elapsed time: " << duration( t3-t2 ) << " ms."<< std::endl;
    printf( "Estimated gain %f, estimated usage: %d\n", bestGain, bestUsage );
    static int decompositions =0;
    
    if( m_iteration > 1 && bestGain <= 0.0 ) {
        std::cout << "No compression gain." << std::endl;

        bool prune =false;
        for( auto p : *m_ct ) {
             prune = prunePattern( p, false ) || prune;
        }
        if( prune ) return true;

        m_isEncoded =true;

        for( auto&& p : *m_ct ) {
            if( p->isActive() )
                printf( "Pattern #%5d\t usage %d, %dx%d, codeword length %f\n",
                    p->label(), p->usage(), p->bounds().width, p->bounds().height, p->codeLength() );
        }
        printf( "Total number of succesfull decompositions: %d\n", m_decompositions );

        return false;
    }
    
    fprintf( stderr, "Merging '%d' and '%d' (%d,%d)...", 
            bestC.p1->label(), bestC.p2->label(), bestC.offset.row(), bestC.offset.col());


    mergePatterns( &bestC );

    prunePattern( bestC.p1, true );
    if( bestC.p1 != bestC.p2 )
        prunePattern( bestC.p2, true );

    double oldBits = m_encodedBits;

    updateCodeLengths();
    
    TimeVarT t4 = timeNow();
    std::cerr << "Elapsed time: " << duration( t4-t3 ) << " ms."<< std::endl;
    printf( "Actual gain: %f\n", oldBits - m_encodedBits );
    fprintf( stderr, "Computing decompositions... " );

    if( std::abs((oldBits - m_encodedBits) - bestGain ) > 0.0001 ) {
        printf( "\n*** Computed gain doesn't match! Panic! ***\n\n" );
        computeGain( &bestC, bestUsage, true );
        bestC.p1->debugPrint();
        if( bestC.p1 != bestC.p2 )
            bestC.p2->debugPrint();
        for( auto&& p : *m_ct ) {
            printf( "Pattern #%5d\t usage %d, %dx%d, codeword length %f (%s)\n",
                p->label(), p->usage(), p->bounds().width, p->bounds().height, p->codeLength(), p->isActive() ? "active" : "inactive");
        }
        return false;
    }

    /* Run prunePattern with decomposition */
    /*prunePattern( bestC.p1, false );
    if( bestC.p1 != bestC.p2 )
        prunePattern( bestC.p2, false );*/
    
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
    m_mat->unflagAll();

    TimeVarT t2 = timeNow();

    std::cout << "Total elapsed time: " << duration( t2-t ) << " ms." << std::endl << std::flush;

    return steps;
}

void
Encoder::reencode() {

    m_instlist.clear();

    /* Iterate over all patterns in the CT, sorted descending by size */
    m_ct->sortBySizeDesc();
    for( Pattern* p : *m_ct ) {

        p->setActive( false );
        p->usage() =0;

        for( int j =0; j < m_mat->width(); j++ ) {
            for( int i =0; i < m_mat->height(); i++ ) {
                Coord2D c =m_mat->makeCoord( i, j );

                /* Test if p fits the matrix at pivot c */
                if( p->apply( m_mat, c, false ) ) {
                    p->apply( m_mat, c, true );

                    p->usage()++;
                    p->setActive( true );
                    m_instlist.push_back( new Instance( p, c, m_es->makeNullVariant(), false ) );
                    m_instmat.place( m_instlist.back() );
                }
            }
        }
    }
    std::sort( m_instlist.begin(), m_instlist.end(), instance_less_than );

    m_mat->unflagAll();
    updateCodeLengths();
}

Matrix2D* 
Encoder::decode() {

}

/* Private functions for class Encoder */

void 
Encoder::mergePatterns( const Candidate* c ) {
    // Create the merged pattern
    Pattern* p_union = new Pattern( *c->p1, *c->v1, *c->p2, *c->v2, c->offset );
    p_union->setLabel( m_lastLabel++ );
    m_ct->push_back( p_union );

    //assert( p_union->isCanonical() ); // Sanity check, creates a bit of overhead

    for( int i =0; i < m_instlist.size(); i++ ) {
        Instance *r1 = m_instlist[i];
        if( r1->isFlagged() ) continue;
        //r1->setFlagged( true );

        Pattern* p1 =r1->pattern();
        if( p1 != c->p1 ) continue;
        
        // Get the periphery of r1's pattern
        const Vouw::Pattern::PeripheryT& post =p1->periphery( Vouw::Pattern::PosteriorPeriphery );

        // Iterate over all instances in r1's periphery
        for( auto && p_offset : post ) {
            Vouw::Coord2D coord = p_offset.abs( r1->pivot() );
            Instance* r2 = m_instmat[coord];
            if( !r2 ) continue; // No instance at this coord
            if( r2->isFlagged() ) continue;

            Pattern* p2 =r2->pattern();
            if( p2 != c->p2 ) continue;

            Pattern::OffsetT offset( r1->pivot(), r2->pivot() );

            if( offset != c->offset ) continue;
            
            Variant* v;
            if( r1->variant()->hash() != c->v1->hash() || r2->variant()->hash() != c->v2->hash() ) {
                v =m_es->makeVariant( *p_union, *p1, *p2, *r1->variant(), *r2->variant(), offset );
                if( !v->isValid() ) continue;
            } else {
                v =m_es->makeNullVariant();
            }

            Coord2D pivot =r1->pivot();

            r2->setFlagged( true ); // Mark for deletion
            delete m_instlist[i];
            m_instlist[i] = new Instance( p_union, pivot, v, false );

            //m_instmat.remove( &r1 );
            //m_instmat.remove( r2 );
            m_instmat.place( m_instlist[i] );

            p_union->usage()++;
            p1->usage()--;
            p2->usage()--;
            break;
        }
    }
    
    m_instlist.deleteIfFlagged( m_instlist.begin(), m_instlist.end() );

    printf( "Actual usage: %d, actual dimensions %d x %d\n", p_union->usage(), p_union->bounds().width, p_union->bounds().height );
}

double
Encoder::updateCodeLengths() {
    assert( m_massfunc );
    m_ct->updateCodeLengths( m_instlist.totalCount(), *m_massfunc );
    //m_instlist.updateCodeLengths( m_ct->countIfActive() );
    return (m_encodedBits =m_ct->totalLength() /*+ m_instlist.totalLength()*/);
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
    const int totalInstances = m_instlist.totalCount() - usage;

    // For the instance set to decode correctly, its cardinality need to be known in advance
    //bits += uintCodeLength( m_instlist.totalCount() ) - uintCodeLength( totalInstances );
    //bits += uintCodeLength( binom( m_mat->width() * m_mat->height(), m_instlist.totalCount() ) )
    //     -  uintCodeLength( binom( m_mat->width() * m_mat->height(), totalInstances ) );
    // Idem for the model
    bits += uintCodeLength( modelSize ) - uintCodeLength( newModelSize );

    // Recompute code table and instance codewords based on the new model's size
    // We use the fact that -log(a/b) = log(b)-log(a)
    // Here we subtract the log(b) component and replace it with log(b+k)
    //const int f =m_instlist.totalCount()+m_ct->countIfActive();
    //bits += f * (log2( m_instlist.totalCount() ) - log2( totalInstances ));
    bits += (lgamma( (double)m_instlist.totalCount() + pseudoCount * (double)modelSize ) / log(2) - lgamma( pseudoCount * (double)modelSize ) / log(2)) 
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
    const int totalInstances = m_instlist.totalCount() + p->usage() * (n-1);
    double bits = 0.0;
    
    // For the instance set to decode correctly, its cardinality need to be known in advance
    //bits += uintCodeLength( m_instlist.totalCount() ) - uintCodeLength( totalInstances );
    // Idem for the model
    bits += uintCodeLength( modelSize ) - uintCodeLength( newModelSize );

    // Recompute code table and instance codewords based on the new model's size
    // We use the fact that -log(a/b) = log(b)-log(a)
    // Here we subtract the log(b) component and replace it with log(b+k)
    const int f =m_instlist.totalCount()+m_ct->countIfActive();
    bits += (lgamma( (double)m_instlist.totalCount() + pseudoCount * (double)modelSize ) / log(2) - lgamma( pseudoCount * (double)modelSize ) / log(2)) 
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
        return false;
    }
    if( onlyZeroPattern || p->size() == 1 || p->isActive() == false ) return false;

    int modelSize = m_ct->countIfActive();
    double g =computeDecompositionGain( p, modelSize );
    if( g > 0.0 ) {
        fprintf( stderr, "The decompositon of %d would result in %f bits gain.\n", p->label(), g );
        for( auto&& r : m_instlist ) {
            if( r->pattern() == p ) {
                r->setFlagged( true );
                decompose( r );
                ((Pattern*)p)->usage()--;
            }
        }
        m_instlist.deleteIfFlagged( m_instlist.begin(), m_instlist.end() );
        p->setActive( false );
        m_decompositions++;
        double oldBits = m_encodedBits;
        double oldIBits = m_instlist.totalCodeLength();
        double oldCBits = m_ct->totalLength();
        updateCodeLengths();
        printf( "Actual decomposition gain: %f (instance set %f, code table %f)\n", 
                oldBits - m_encodedBits, oldIBits - m_instlist.totalCodeLength() , oldCBits - m_ct->totalLength() );
        if( std::abs( g - (oldBits-m_encodedBits) ) > 0.0001 ) {
            printf( "\n*** Computed decomposition gain doesn't match! Panic! ***\n\n" );
        }
        /* Decompositions may have changed the order of the instance set */
        std::sort( m_instlist.begin(), m_instlist.end(), instance_less_than );

        return true;
    }

    return false;
}

void
Encoder::decompose( Instance* inst ) {

    const Pattern* p =inst->pattern();
    const Pattern::CompositionT& comp = p->composition();
    if( !comp.isValid() ) return;

    Coord2D pivot2 = comp.offset.abs( inst->pivot() );

    Instance *inst1 = new Instance( (Pattern*)comp.p1, inst->pivot(), comp.v1 );
    if( !comp.p1->isActive() ) {
        decompose( inst1 );
        delete inst1;
    } else {
        ((Pattern*)comp.p1)->usage()++;
        m_instlist.push_back( inst1 );
        m_instmat.place( inst1 );
    }

    Instance *inst2 = new Instance( (Pattern*)comp.p2, pivot2, comp.v2 );
    if( !comp.p2->isActive() ) {
        decompose( inst2 );
        delete inst2;
    } else {
        ((Pattern*)comp.p2)->usage()++;
        m_instlist.push_back( inst2 );
        m_instmat.place( inst2 );
    }
}

VOUW_NAMESPACE_END
