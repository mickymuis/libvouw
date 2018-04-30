/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/encoder.h>
#include <vouw/matrix.h>
#include <vouw/codetable.h>
#include <vouw/equivalence.h>
#include <map>
#include <unordered_map>
#include <functional>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cassert>

VOUW_NAMESPACE_BEGIN

/* Will move at some point */

struct Candidate {
    Pattern* p1, *p2;
    Variant* v1, *v2;
    Pattern::OffsetT offset;
};

bool operator==( const Candidate& c1, const Candidate& c2 ) {
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
        hash = hash * HASH_FACTOR + std::hash<int>()(c.v1->hash());
        hash = hash * HASH_FACTOR + std::hash<int>()(c.v2->hash());
        return hash;
    }
};

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
    return m_es && m_mat && m_ct && m_encoded.size();
}

void Encoder::setFromMatrix( Matrix2D* mat ) {
    //std::map<Matrix2D::ElementT,std::pair<Pattern*,Variant>> smap; // Singleton equivalence mapping

    clear();
    m_ct = new CodeTable( mat );
    m_encoded.setMatrixSize( mat->width(), mat->height(), mat->base() );
    m_mat =mat;

    for( int i =0; i < m_mat->height(); i++ ) {
        for( int j =0; j < m_mat->width(); j++ ) {
            Coord2D c = m_mat->makeCoord( i,j );
            Matrix2D::ElementT elem = m_mat->value( c );
            Pattern* p =0;
            Variant v;
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
                    if( v.isValid() ) {
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
                m_smap[elem] = std::pair<Pattern*,Variant>(p,v);
            }

            p->usage()++;
            m_encoded.emplace_back( p, c, v, false );
        }
    }
    std::cout << "Added " << m_ct->size() << " patterns in " << m_encoded.size() << " regions." << std::endl;
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
    m_encoded.clear();
    m_smap.clear();

    m_priorBits =0.0;
    m_isEncoded =false;
    m_lastLabel =0;
}

bool Encoder::encodeStep() { 
    std::unordered_map<Candidate,int,CandidateHash> candidates;

    int progress =0, total = m_encoded.size();
    std::cerr << "Finding candidates ... ";

    //for( auto&& r1 : m_encoded ) {
    for( int i =0; i < m_encoded.size(); i++ ) {
        const Region& r1 = m_encoded[i];
       // if( r1.isMasked() ) continue;
       // r1.setMasked( true );

        Pattern* p1 =r1.pattern();

        if( progress++ % (total/10+1) == 0 )
          std::cerr << progress*100/total << "% ";

        for( int j =i+1; j < m_encoded.size(); j++ ) {
            const Region& r2 = m_encoded[j];
//        for( auto&& r2 : m_encoded ) {
         //   if( r2.isMasked() ) continue;

            //if( r2.pivot() < r1.pivot() ) break;
            if( r2.pivot().row() > r1.pivot().row() + p1->bounds().height ) break;
            
            Pattern* p2 =r2.pattern();

            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );

            if( p1 == p2 && r1.isMasked( offset.direction() ) ) continue;
            if( !p1->isAdjacent( *p2, offset ) ) continue;

            // Increment the usage count of this particular combination
            Candidate c = { p1, p2, (Variant*)&r1.variant(), (Variant*)&r2.variant(), offset };
            candidates[c]++;

            //fprintf( stderr, "%d + %d (%d,%d) @ (%d,%d)\n", p1->label(),p2->label(),offset.row(),offset.col(),r1.pivot().row(),r1.pivot().col());
            if( p1 == p2 )
                r2.setMasked( true, offset.direction() );
        
        }
    }

    std::cerr << std::endl << candidates.size() << " canditates found. Bucket count: " << candidates.bucket_count() << std::endl;

    m_encoded.unmaskAll();

    int bestUsage =0;
    double bestGain =0.0;
    Candidate bestC;

    for( auto&& pair : candidates ) {
        const Candidate& c = pair.first;
        double gain = computeGain( &c, pair.second );
        if( gain >= bestGain ) {
            bestGain =gain;
            bestC =c;
            bestUsage =pair.second;
        }
    }

    printf( "Estimated gain %f, estimated usage: %d\n", bestGain, bestUsage );
    
    if( bestGain <= 0.0 ) {
        std::cout << "No compression gain." << std::endl;
        m_isEncoded =true;

        for( auto&& p : *m_ct ) {
            printf( "Pattern #%5d\t usage %d, %dx%d, codeword length %f\n",
                    p->label(), p->usage(), p->bounds().width, p->bounds().height, p->codeLength() );
        }
        return false;
    }
    printf( "Merging '%d' and '%d' (%d,%d)\n", 
            bestC.p1->label(), bestC.p2->label(), bestC.offset.row(), bestC.offset.col());


    mergePatterns( &bestC );

    prunePattern( bestC.p1 );
    if( bestC.p1 != bestC.p2 )
        prunePattern( bestC.p2 );

    double oldBits = m_encodedBits;

    updateCodeLengths();
    
    printf( "Actual gain: %f\n", oldBits - m_encodedBits );

    for( auto it = m_ct->begin(); it != m_ct->end(); ) {
        Pattern* p =*it;
        it++;
        prunePattern( p, false );
    }

    std::cerr << std::flush;
    std::cout << std::flush;
    return true;
}

int Encoder::encode() {
    int steps =0;
    while( encodeStep() ) steps++;
    return steps;
}

Matrix2D* Encoder::decode() {

}

/* Private functions for class Encoder */

double
Encoder::updateCodeLengths() {
    m_ct->updateCodeLengths( m_encoded.size() );
    m_encoded.updateCodeLengths();
    return (m_encodedBits =m_ct->totalLength() + m_encoded.totalLength());
}

/*
 * Calculate the gain in encoding size if patterns p1 and p2 were replaced by their union.
 * This union is assumed to have estimated usage p_usage,
 * which is assumed to be less or equal than the usages of p1 and p2.
 * The return value is the difference in encoding side in bits.
 */
double
Encoder::computeGain( const Candidate* c, int usage ) {

    // New pivots can be sized differently because their size depends on the number of regions
    double newBitsPerPivot = m_encoded.bitsPerPivot();//RegionList::bitsPerPivot( totalInstances );
    double bits = m_encoded.size() * (m_encoded.bitsPerPivot() - newBitsPerPivot);

    // Compute the total size of the new model
    const int num_patterns = c->p1 == c->p2 ? 1 : 2;
   /* int newModelSize = m_ct->size() + 1;
    if( num_patterns == 2 ) {
        newModelSize -= ((int)(c->p1->usage() - usage == 0 && c->p1->size() != 1) 
                      + (int)(c->p2->usage() - usage == 0 && c->p2->size() != 1));
    } else {
        newModelSize -= (int)(c->p1->usage() - 2*usage == 0 && c->p1->size() != 1);
    }*/

    //printf( "(%d,%d) new model size %d (was %d) ", c->offset.row(), c->offset.col(), newModelSize, m_ct->size() );

    const int totalInstances = m_encoded.size() - usage;
    // Recompute code table and region codewords based on the new model's size
    for( auto&& p : *m_ct ) {
        bits += p->codeLength() * (p->usage() + 1);
        bits -= Pattern::codeLength( p->usage(), totalInstances ) * (p->usage() + 1);
    }

   // const int  totalInstances = m_mat->count();

    for( int i =0; i < num_patterns; i++ ) {
        Pattern* p = (&c->p1)[i];
        
        double codeLength =Pattern::codeLength( p->usage(), totalInstances );
        // Step 1. remove the bits of the old regions and code table codeword completely
        bits += (codeLength + newBitsPerPivot /*c->p1->bitsPerVariant() +*/) * p->usage();
        bits += codeLength;
        
        // Step 2. Re-add the old patterns based on their new usage
        int p_usage = p->usage() - (usage * (num_patterns == 1 ? 2 : 1) );
        
        double newCodeLength =Pattern::codeLength( p_usage, totalInstances );

        if( p_usage > 0 ) {
            // Instance set part
            bits -= (newCodeLength + newBitsPerPivot /*+ c->p1->bitsPerVariant()*/) * p_usage;
            // Code table part
            bits -= newCodeLength;
        }
        else if( p->size() > 1 ) { 
            // Pattern p1 will be removed from the code table
            bits += p->bitsPerOffset() * p->size();
        }
    }

    // Step 3. add the length from the union pattern p
    const int width = std::max( 
            std::abs(c->offset.col()) + c->p2->bounds().width + std::abs(c->p1->bounds().colMin),
            c->p1->bounds().width );
                   
    const int height = std::max(
            std::abs(c->offset.row()) + c->p2->bounds().height + std::abs(c->p1->bounds().rowMin),
            c->p1->bounds().height );
    //printf( "square size %d\n", width*height );

    double p_codeLength = Pattern::codeLength( usage, totalInstances );
    // Instance set part
    bits -= (p_codeLength + newBitsPerPivot /*+ bitsPerVariant*/) * (usage);
    // Code table part
    bits -= Pattern::bitsPerOffset( width, height, m_mat->base() ) * (c->p1->size() + c->p2->size()) + p_codeLength;
    
    return bits;
}

double
Encoder::computePruningGain( const Pattern* p ) {

    // New pivots can be sized differently because their size depends on the number of regions
    const int totalInstances = m_encoded.size() + p->usage() * (p->size() - 1);
    double newBitsPerPivot = m_encoded.bitsPerPivot();//RegionList::bitsPerPivot( totalInstances );
    double bits = m_encoded.size() * (m_encoded.bitsPerPivot() - newBitsPerPivot);

    // Compute the total size of the new model
    int newModelSize = m_ct->size() - 1;

    // Recompute code table and region codewords based on the new model's size
    for( auto&& p0 : *m_ct ) {
        bits += p0->codeLength() * (p0->usage() + 1);
        bits -= Pattern::codeLength( p0->usage(), totalInstances ) * (p0->usage() + 1);
    }

    // Step 1. remove the bits of the regions and code table codeword of @p completely
    double codeLength =Pattern::codeLength( p->usage(), totalInstances );
    // Instance set part
    bits += (codeLength + newBitsPerPivot /*c->p1->bitsPerVariant() +*/) * p->usage();
    // Code table part
    bits += codeLength + p->bitsPerOffset() * p->size();
    
    // Step 2. Replace the instances of @p by singleton patterns
    std::map<Pattern*, int> singleton; // Mapping of increased number of instances to singleton patterns
    for( auto&& elem : p->elements() ) {
        Pattern* ps = m_smap[elem.value].first; // Singleton pattern matching elem
        assert( ps );
        singleton[ps]++; // Increment usage
    }

    // Step 3. Update the singleton patterns depending on their new usages
    for( auto&& pair : singleton ) {
        Pattern* ps = pair.first;
        int newUsage = ps->usage() + p->usage() * pair.second;
        double newCodeLength = Pattern::codeLength( newUsage, totalInstances );

        // Update the code table
        bits += ps->codeLength() - newCodeLength;

        // Add the new instances
        bits += (ps->codeLength() + newBitsPerPivot) * ps->usage();
        bits -= (newCodeLength + newBitsPerPivot) * newUsage;
    }
    return bits;
}

void 
Encoder::mergePatterns( const Candidate* c ) {
    // Create the merged pattern
    Pattern* p_union = new Pattern( *c->p1, c->v1, *c->p2, c->v2, c->offset );
    p_union->setLabel( m_lastLabel++ );
    m_ct->push_back( p_union );

    //for( auto it1 = m_encoded.begin(); it1 != m_encoded.end(); it1++ ) {
    for( int i =0; i < m_encoded.size(); i++ ) {
        const Region& r1 = m_encoded[i];//*it1;
        if( r1.isMasked() ) continue;
        //r1.setMasked( true );

        Pattern* p1 =r1.pattern();
        if( p1 != c->p1 ) continue;

//        for( auto it2 = m_encoded.begin(); it2 != m_encoded.end(); it2++ ) {
        for( int j=i+1; j < m_encoded.size(); j++ ) {
            const Region& r2 = m_encoded[j];//*it2;
            if( r2.isMasked() ) continue;

            //if( r2.pivot() < r1.pivot() ) continue;
            //if( r2.pivot().row() > r1.pivot().row() + p1->bounds().height ) break;
            
            Pattern* p2 =r2.pattern();

            if( p2 != c->p2 ) continue;

            Pattern::OffsetT offset( r1.pivot(), r2.pivot() );

            if( offset != c->offset ) continue;
            
            //fprintf( stderr, "%d + %d (%d,%d) @ (%d,%d)\n", p1->label(),p2->label(),offset.row(),offset.col(),r1.pivot().row(),r1.pivot().col());
            Variant v;
            if( r1.variant().hash() != c->v1->hash() || r2.variant().hash() != c->v2->hash() ) {
                v =m_es->makeVariant( *p_union, *p1, *p2, r1.variant(), r2.variant(), offset );
                if( !v.isValid() ) continue;
            } else {
                v =m_es->makeNullVariant();
            }

            Coord2D pivot =r1.pivot();

            //m_encoded.erase( m_encoded.begin() + j );
            m_encoded[j].setMasked( true ); // Mark for deletion
            m_encoded[i] = Region( p_union, pivot, v, false );


/*            auto temp = ++it1;
            if( temp == it2 ) {
                m_encoded.erase( it1 );
                it1 = ++temp;
                m_encoded.erase( it2 );
                it2 = ++temp;
            } else {
                m_encoded.erase( it1++ );
                m_encoded.erase( it2++ );
            }*/
            
            //m_encoded.emplace_back( p_union, pivot, v, true );
            p_union->usage()++;
            p1->usage()--;
            p2->usage()--;
            break;
        }
    }

    m_encoded.eraseIfMasked( m_encoded.begin(), m_encoded.end() );
    m_encoded.unmaskAll();

    printf( "Actual usage: %d\n", p_union->usage() );
    
}

void
Encoder::prunePattern( Pattern* p, bool onlySingleton ) {
    if( p->size() == 1 ) return;
    if( p->usage() == 0 ) {
        m_ct->remove( p );
        delete p;
    }
    if( onlySingleton ) return;

    // The complex case for non-zero usage patterns
    // Calculate gain first to see if we have to do anything
    double gain =computePruningGain( p );
    if( gain < 0.0 ) return;
    
    fprintf( stderr, "Pruning pattern %d would result in a gain of %f bits.\n", p->label(), gain );
    
    // Find every region in which @p occurs and replace it with singletons
    const std::size_t currentInstances =m_encoded.size();
    for( int i =0; i < currentInstances; i++ ) {
        const Region& r =m_encoded[i];
        if( r.pattern() != p ) continue;

        Coord2D origin =r.pivot();
        bool first =true;
        for( auto&& elem : p->elements() ) {
            PatternVariantT pvt = m_smap[elem.value];
            assert( pvt.first );
            Coord2D pivot = elem.offset.abs( origin );

            if( first ) {
                m_encoded[i] = Region( pvt.first, pivot, pvt.second, false );
            } else {
                m_encoded.emplace_back( pvt.first, pivot, pvt.second, false );
            }
            pvt.first->usage()++;
            first =false;
        }
    }

    // The instance set needs to be resorted
    std::sort( m_encoded.begin(), m_encoded.end() );
    m_ct->remove( p );
    delete p;

    double oldBits = m_encodedBits;

    updateCodeLengths();
    
    fprintf( stderr, "Actual pruning gain: %f\n", oldBits - m_encodedBits );

}

VOUW_NAMESPACE_END
