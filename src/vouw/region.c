/*
 * VOUW - Generating, encoding and pattern-mining of Reduce-Fold Cellular Automata
 *
 * Micky Faas <micky@edukitty.org> 
 * Leiden Institute for Advanced Computer Science
 */

#include <vouw/region.h>
#include <stdlib.h>

region_t*
region_create( pattern_t* pattern, vouw_coord_t pivot ) {
    region_t* r = (region_t*)malloc( sizeof( region_t ) );
    r->pivot =pivot;
    r->pattern = pattern;
    r->masked = false;
    INIT_LIST_HEAD( &(r->list ) );
    return r;
}

void
region_apply( const region_t* region, vouw_matrix_t* m ) {
    pattern_t* p =region->pattern;
    for( int i =0; i < p->size; i++ ) {
        // For each offset, compute its location on the automaton
        vouw_coord_t c = pattern_offset_abs( region->pivot, p->offsets[i] );
        // Set the buffer's value at c
        vouw_matrix_setValue( m, c, (p->offsets[i].value + region->variant) % m->base );
    }
}

void
region_free( region_t* r ) {
    free( r );
}

void
region_list_free( region_t* r ) {
    struct list_head* tmp,* pos;
    list_for_each_safe( pos, tmp, &(r->list) ) {
        region_t* entry = list_entry( pos, region_t, list );
        list_del( pos );
        
        region_free( entry );
    }
    free( r );
}

void 
region_list_unmask( region_t* r ) {
    struct list_head* pos;
    list_for_each( pos, &(r->list) ) {
        region_t* entry = list_entry( pos, region_t, list );
        entry->masked =false;
    }
}
