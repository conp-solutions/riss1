/*
    riss
    Copyright (C) 2011 Norbert Manthey

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/ 
#if 0

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "blocked.h"
#include "common.h"
#include "doublelook.h"
#include "memory.h"
#include "parser.h"
#include "tree.h"




#define BCE






#define	OCCURENCE_GRAPH		1
#define CONFLICT_GRAPH		2
#define	RESOLUTION_GRAPH	3
#define SUBSUMPTION_GRAPH	4

#define INITIAL_ORDER		nrofclauses
#define INITIAL_LOW		nrofclauses
#define INITIAL_PARENT		-1

#define PRINT_COMPONENTS


#define PRINT_EDGES

int *candidates, nrofcandidates;
int *MarkArray;
int **literal_occ, **literal_lut;
int binary_blocked = 0, nary_blocked = 0; 
int removed_blocked_clauses = 0;	
int *clause_order_stamp, *clause_order_array, *clause_parent, *clause_low;
int current_order;

int *graph_subsume, graph_subsume_stamp = 0;

void allocate_local_datastructures( );
void free_local_datastructures( );

void iHTE();
void hidden_literal_addition( );
int hidden_tautology_elimination( );
void hidden_blocked_clause_elimination( );


int depth_first_graph_rec( const int clsidx, const int parent, const int graph_flag );
void determine_clause_low( const int clsidx, const int graph_flag );

int seive_blocked_binaries( )
{
	int i, j, k, lit1, lit2, imp, *bImp, *tImp, _freevar, *_freevarsArray;
	int blocked;
        struct treeNode _treeNode, *_treeArray;

	reset_doublelook_pointers();

	currentTimeStamp = 0;

	for( _freevarsArray = freevarsArray, i = freevars; i > 0; i-- )
	{
	    _freevar = *(_freevarsArray++);
	    UNFIX( _freevar );
	}	

	if( treebased_lookahead() == UNSAT ) return UNSAT;

	_treeArray = treeArray;
        for( i = tree_elements-1; i >= 0; i-- )
        {
	    _treeNode = *(_treeArray++);

            currentTimeStamp += _treeNode.gap;

            if( DL_treelook( _treeNode.literal ) == UNSAT )    return UNSAT;

            bImp = BIMP_START( _treeNode.literal );
            for( j = BIMP_ELEMENTS; --j; )
	    {
		imp = *(bImp++);
		blocked = 1;

		printf("checking blocked %i v %i\n", -_treeNode.literal, imp );

        	tImp = TernaryImp[ -imp ];
		for( k = TernaryImpSize[ -imp ]; k > 0; k-- )
        	{
		    lit1 = *(tImp++);
		    printf("lit %i %i %i\n", lit1, IS_FIXED(lit1), FIXED_ON_COMPLEMENT(lit1) );
		    lit2 = *(tImp++);
		    printf("lit %i %i %i\n", lit2, IS_FIXED(lit2), FIXED_ON_COMPLEMENT(lit2) );

		    printf("imp %i v %i v %i\n", -imp, lit1, lit2 );

		    if( !(IS_FIXED(lit1) && !FIXED_ON_COMPLEMENT(lit1)) &&
		        !(IS_FIXED(lit2) && !FIXED_ON_COMPLEMENT(lit2)) )
			blocked = 0;

		    if( blocked == 0 ) break;
		}
		if( blocked ) printf("found blocked %i v %i\n", -_treeNode.literal, imp );
	    }
            currentTimeStamp -= _treeNode.gap;
        }
	
	return SAT;
}

void print_graph_data( )
{
	int i;

	clause_order_stamp = (int*) malloc( sizeof(int) * nrofclauses );
	clause_order_array = (int*) malloc( sizeof(int) * nrofclauses );
	clause_parent      = (int*) malloc( sizeof(int) * nrofclauses );
	clause_low         = (int*) malloc( sizeof(int) * nrofclauses );

	allocate_local_datastructures();

	graph_subsume = (int*) malloc( sizeof(int) * (2*nrofvars + 1) );
	for( i = 0; i <= 2*nrofvars; i++ ) 
	    graph_subsume[ i ] = 0;
	
	graph_subsume += nrofvars;







	printf("p edge %i %i \n", nrofclauses, count_edges( SUBSUMPTION_GRAPH ) );

	graph_subsume -= nrofvars;
	free( graph_subsume );

	free_local_datastructures();

	free( clause_order_stamp );
	free( clause_order_array );
	free( clause_parent );
	free( clause_low );
}

int count_edges( const int graph_flag )
{
    int i, j, lit, clsidx, clschk, nrofedges = 0;

    if( graph_flag <= CONFLICT_GRAPH )
	for( i = 0; i < nrofclauses; i++ )
	    clause_parent[ i ] = -1;	

    for( clsidx = 0; clsidx < nrofclauses; clsidx++ )
    {
	assert( Clength[ clsidx ] > 0 );

	if( graph_flag >= RESOLUTION_GRAPH )
	    mark_complement_of_literals( clsidx );
	else if( graph_flag == OCCURENCE_GRAPH )
	    clause_parent[ clsidx ] = clsidx;
	
	for( i = 0; i < Clength[ clsidx ]; i++ )
	{
	    lit = -Cv[ clsidx ][ i ];
	    for( j = 1; j <= literal_occ[ lit ][ 0 ]; j++ )
	    {
		clschk = literal_occ[ lit ][ j ];
		if( graph_flag <= CONFLICT_GRAPH )
		{
		    if( clause_parent[ clschk ] != clsidx )
		    {	clause_parent[ clschk ] = clsidx;
#ifdef PRINT_EDGES
		        if( clschk < clsidx )
			    printf("e %i %i\n", clschk + 1, clsidx + 1 );
#endif
			nrofedges++; 		    	    }
		}
		else if( count_clashing_literals( clschk, clsidx ) == 1 )
		{
		    if( graph_flag == SUBSUMPTION_GRAPH )
			if( is_subsumed( clschk, clsidx ) )
			    continue;
#ifdef PRINT_EDGES
		    if( clschk < clsidx )
			printf("e %i %i\n", clschk + 1, clsidx + 1 );
#endif
		    nrofedges++;
		}
	    }
	    if( graph_flag == OCCURENCE_GRAPH )
	    {
	    	for( j = 1; j <= literal_occ[ -lit ][ 0 ]; j++ )
	    	{
		    clschk = literal_occ[ -lit ][ j ];
		    if( clause_parent[ clschk ] != clsidx )
		    {	clause_parent[ clschk ] = clsidx;
#ifdef PRINT_EDGES
		        if( clschk < clsidx )
			    printf("e %i %i\n", clschk + 1, clsidx + 1 );
#endif
			nrofedges++;				}
		}
	    }
	}
    }
    assert( (nrofedges % 2) == 0);
    return nrofedges / 2;
}

int count_components( const int graph_flag )
{
	int i, tmp, nrofcomponents = 0;	

	current_order = 0;

	for( i = 0; i < nrofclauses; i++ )
	{
	    clause_order_stamp[ i ] = INITIAL_ORDER;
	    clause_low        [ i ] = INITIAL_LOW;
	}

	for( i = 0; i < nrofclauses; i++ )
	{
	    if( clause_order_stamp[ i ] == INITIAL_ORDER )
	    {
		nrofcomponents++;
		if( depth_first_graph_rec( i, INITIAL_PARENT, graph_flag ) >= 2 )
	     	    printf("c found ARTICULATION clause %i\n", i);
	    }
	}

	for( i = nrofclauses - 1; i >= 0; i-- )
	{
	     tmp = clause_order_array[ i ];
	     determine_clause_low( tmp, graph_flag );
	     if( (clause_low[ tmp ] != 0) &&
	         (clause_low[ tmp ] == clause_order_stamp[ clause_parent[ tmp ] ]) && 
		 (clause_parent[ clause_parent[ tmp ] ] != INITIAL_PARENT) )
	     	    printf("c found ARTICULATION clause %i\n", clause_parent[ tmp ] );
	}

	printf("c number of components = %i\n", nrofcomponents );

	print_components( nrofcomponents );

	return nrofcomponents;
}

int depth_first_graph_rec( const int clsidx, const int parent, const int graph_flag )
{
	int i, j, lit, clschk, counter = 0;

	clause_order_array[ current_order ] = clsidx;
	clause_order_stamp[ clsidx ] = current_order;
	clause_parent     [ clsidx ] = parent; 
	clause_low        [ clsidx ] = current_order;
	current_order++;

	mark_complement_of_literals( clsidx );

	for( i = 0; i < Clength[ clsidx ]; i++ )
	{
	    lit = -Cv[ clsidx ][ i ];
	    for( j = 1; j <= literal_occ[ lit ][ 0 ]; j++ )
	    {
		clschk = literal_occ[ lit ][ j ];
		if( clause_order_stamp[ clschk ] == INITIAL_ORDER )
		{
		    if( graph_flag <= CONFLICT_GRAPH )
		    {
			counter++;
			depth_first_graph_rec( clschk, clsidx, graph_flag );
		    }
		    else if( count_clashing_literals( clschk, clsidx ) == 1 )
		    {
			if( graph_flag == SUBSUMPTION_GRAPH )
			    if( is_subsumed( clschk, clsidx ) ) continue;

			counter++;
			depth_first_graph_rec( clschk, clsidx, graph_flag );
			mark_complement_of_literals( clsidx );
		    }
		}
	    }
	}
	return counter;
}

void determine_clause_low( const int clsidx, const int graph_flag )
{
	int i, j, lit, clschk;

	mark_complement_of_literals( clsidx );
	
	for( i = 0; i < Clength[ clsidx ]; i++ )
	{
	    lit = -Cv[ clsidx ][ i ];
	    for( j = 1; j <= literal_occ[ lit ][ 0 ]; j++ )
	    {
		clschk = literal_occ[ lit ][ j ];
		if( clause_low[ clsidx ] > clause_low[ clschk ] )
		{
		    if( graph_flag >= RESOLUTION_GRAPH )
		    {
			if( count_clashing_literals( clschk, clsidx ) > 1 )
			    continue;			

			if( graph_flag == SUBSUMPTION_GRAPH )
			    if( is_subsumed( clschk, clsidx ) )
			        continue;
		    }
		    clause_low[ clsidx ] = clause_low[ clschk ];
		}
	    }
	}
}

void mark_complement_of_literals( int clsidx )
{
	int i;

#ifdef GTEDEBUG
	printf("marking %i: ", clsidx );
#endif
	for( i = 0; i < Clength[ clsidx ]; i++ )
	{
#ifdef GTEDEBUG
	    printf("%i ", Cv[clsidx][i] );
#endif
	    MarkArray[ -Cv[ clsidx ][ i ] ] = clsidx;
	}
#ifdef GTEDEBUG
	printf("\n");
#endif
}

int count_clashing_literals( int clsidx, int mark )
{
	int i, counter = 0;

	for( i = 0; i < Clength[ clsidx ]; i++ )
	    if( MarkArray[ Cv[ clsidx ][ i ] ] == mark )
		counter++;

	assert( counter > 0 );
	
	return counter;
}

inline int check_subsumed( const int clsidx )
{
	int i, j, h, subsumed, current;

	for( i = 0; i < Clength[ clsidx ]; i++ )
	    for( j = 1; j <= literal_occ[ Cv[ clsidx ][ i ] ][ 0 ]; j++ )
	    {
	    	subsumed = 1;
		current = literal_occ[ Cv[ clsidx ][ i ] ][ j ];
	    	for( h = 0; h < Clength[ current ]; h++ )
		    if( graph_subsume[ Cv[current][h] ] != graph_subsume_stamp )
		    {
		    	subsumed = 0;
		    	break;
		    } 
	    	if( subsumed ) return 1;
	    }
	return 0;
}

int is_subsumed( const int clsidx1, const int clsidx2 )
{
	int i;

	graph_subsume_stamp++;

	for( i = 0; i < Clength[ clsidx1 ]; i++ )
	    graph_subsume[ Cv[ clsidx1 ][ i ] ] = graph_subsume_stamp;

	for( i = 0; i < Clength[ clsidx2 ]; i++ )
	    if( graph_subsume[ -Cv[ clsidx2 ][ i ] ] == graph_subsume_stamp )
	    	graph_subsume[ -Cv[ clsidx2 ][ i ] ] = 0; 
	    else
	    	graph_subsume[ Cv[ clsidx2 ][ i ] ] = graph_subsume_stamp; 

	if( check_subsumed( clsidx1 ) == 1 ) 
	    return 1;
	return check_subsumed( clsidx2 );
}

void add_blocked_binaries( )
{
	int i, j, idx = 0, *resolution_counter, *resolution_index, nrofadditions = 0;

	MALLOC_OFFSET( resolution_counter, int, nrofvars,  0 );
	MALLOC_OFFSET( resolution_index  , int, nrofvars, -1 );

	for( idx = 0; idx < nrofclauses; idx++ )
	    for( j = 0; j < Clength[ idx ] ; j++ )
	    {
		resolution_counter[ Cv[ idx ][ j ] ]++;
		resolution_index  [ Cv[ idx ][ j ] ] = idx;
	    }

	for( i = -nrofvars; i <= nrofvars; i++ )
	{
	    if( resolution_counter[ i ] == 1 )
	    {
		idx = resolution_index[ i ];
		for( j = 0; j < Clength[ idx ]; j++ )
		    if( Cv[ idx ][ j ] != i )
		    {
			resolution_counter[ -Cv[ idx ][ j ] ]++;
			resolution_index  [ -Cv[ idx ][ j ] ] = idx;
			resolution_counter[ -i ]++;
			resolution_index  [ -i ] = idx;
			nrofadditions++;
		    }
	    }
	}

	printf("c nrofadditions = %i\n", nrofadditions ); 

	Cv      = (int**) realloc( Cv,      sizeof(int*) *  ( nrofclauses + nrofadditions ) );
	Clength = (int* ) realloc( Clength, sizeof(int ) *  ( nrofclauses + nrofadditions ) );


	for( i = -nrofvars; i <= nrofvars; i++ )
	{
	    if( resolution_counter[ i ] == 1 )
	    {
		idx = resolution_index[ i ];
		for( j = 0; j < Clength[ idx ]; j++ )
		    if( Cv[ idx ][ j ] != i )
		    {
			Cv     [ nrofclauses ] = (int*)malloc( sizeof(int) * 2);
			Clength[ nrofclauses ] = 2;
			Cv [ nrofclauses][ 0 ] = -i;
			Cv [ nrofclauses][ 1 ] = -Cv[ idx ][ j ];
			nrofclauses++;
		    }
	    }
	}

	FREE_OFFSET( resolution_counter );
	FREE_OFFSET( resolution_index   );
}

void add_horn_clauses()
{
	int h,i,j,k,l;
	int horn_stamp = 0;

	int *clause = (int*) malloc(sizeof(int) * nrofvars );
	int length, orig_length, pos, clsidx;

	allocate_local_datastructures();
#ifdef CLA
	int coverstamp = 0;
	int *intersection;
	intersection = (int*) malloc( sizeof(int) * (2*nrofvars+1) );
	for ( i = 0; i <= 2*nrofvars; i++ )
	    intersection[ i ] = coverstamp;
	intersection += nrofvars;
#endif
	for( i = 1; i <= nrofvars; i++ )
	{
		int *clause_set = literal_occ[ -i ];
		int  set_size   = clause_set[ 0 ];

		for( h = 1; h <= set_size; h++ )
		{

		  length  = 0; pos     = 0;
		  clsidx = clause_set[ h ];
		  for( k = 0; k < Clength[ clsidx ]; k++ )
		  {
			if( Cv[ clsidx ][ k ] == -i )    clause[ length ] = i;
			else if( Cv[ clsidx ][ k ] < 0 ) clause[ length ] = Cv[ clsidx ][ k ];
			else
			{
				pos++;
				if( pos > 1) goto next_horn_clause;
				clause[ length ] = -Cv[ clsidx ][ k ];
			}
			length++;		
                  }

		  if( pos == 1 )
		  {
		     horn_stamp++;
		     for( j = 0; j < length; j++ ) MarkArray[ -clause[ j ] ] = horn_stamp;
		     orig_length = length;
#ifdef CLA
		     int last_cover_index = length; j = 0;
		     while( j != last_cover_index )
		     {
		         if( j == length ) j = 0;
#else
		     for( j = 0; j < length; j++ )
		     {
#endif
			int lit = clause[ j ];
			int first_non_taut = -1;

		        for( k = 1; k <= literal_occ[ -lit ][ 0 ]; k++ )
		        {
		            int newcls = literal_occ[ -lit ][ k ];

		            if( Clength[ newcls ] > 0 ) 
		 	       if( count_clashing_literals( newcls, horn_stamp ) == 1 ) {
#ifndef CLA
				    goto next_horn_literal;
#else
				    int intersection_size = 0;
				    if( first_non_taut == -1 )
				    {
					coverstamp++;
					first_non_taut = newcls;
		  			for( l = 0; l < Clength[ newcls ]; l++ )
					{
					    int cov_lit = Cv[ newcls ][ l ];
					    if( cov_lit == -lit ) continue;

					    intersection[ cov_lit ] = coverstamp;
					    intersection_size++;
					}
				    }
				    else
				    {
		  			for( l = 0; l < Clength[ newcls ]; l++ )
					{
					    int cov_lit = Cv[ newcls ][ l ];
					    if( intersection[ cov_lit ] != coverstamp ) continue;
					    
					    intersection[ cov_lit ] = coverstamp + 1;
					    intersection_size++;
					}
					coverstamp++;
				    }
				    if( intersection_size == 0 ) goto next_horn_literal;
#endif
				}
			}
#ifdef CLA
		        if( first_non_taut != -1 )
		        {
			    for( l = 0; l < Clength[ first_non_taut ]; l++ )
			    {
			        int cov_lit = Cv[ first_non_taut ][ l ];

			        if( (intersection[ cov_lit ] == coverstamp) && (MarkArray[ -cov_lit ] != horn_stamp) )
			        {
				    MarkArray[ -cov_lit ] = horn_stamp;
				    clause   [ length++ ] = cov_lit;
				    last_cover_index = j;
				    if( j == 0 ) last_cover_index = length;
			        }
			    }
			    goto next_horn_literal;
		        }
		        else
#endif			
			{
			    printf("[%i] ", lit );
			    goto horn_blocked_clause;
			}
			next_horn_literal:;
#ifdef CLA
			j++;
#endif
		     }

		     goto next_horn_clause;
		     horn_blocked_clause:;
		     printf("found new blocked horn clause");
		     for( j = 0; j < orig_length; j++ ) printf(" %i", clause[ j ] );
		     printf("\n");
		  }
		  next_horn_clause:;

		}
	}
}

void iHTE()
{
        while ( hidden_tautology_elimination( ) )
        {
                find_and_propagate_unary_clauses();
                
#ifdef TIMEOUT
            if( (int) clock() > CLOCKS_PER_SEC * TIMEOUT )
                return;
#endif
        }
}

void iteratively_remove_blocked_clauses( )
{
#ifdef HTE
	while ( hidden_tautology_elimination( ) )
	{
		find_and_propagate_unary_clauses();
		
#ifdef TIMEOUT
	    if( (int) clock() > CLOCKS_PER_SEC * TIMEOUT ) return;
#endif
	}
#endif
#ifdef BCE
	blocked_clause_elimination( 0 );
  #ifdef CLA
	blocked_clause_elimination( 1 );
  #endif
#endif
#ifdef HBCE
	hidden_blocked_clause_elimination( );
#endif
#ifdef HLA
	hidden_literal_addition( );
#endif

}


int hidden_tautology_elimination( )
{
	int i, j, k, nrval, failed_flag, unit_flag = 0;
	int *bin_occ, **bin_lut, *bin_table, bin_total = 0;
	int *bin_que, *head, *tail, hla_size;

	allocate_local_datastructures();

	bin_occ = (int * ) malloc( sizeof(int ) * (2*nrofvars + 1) );
	bin_que = (int * ) malloc( sizeof(int ) * (2*nrofvars + 1) );
	bin_lut = (int **) malloc( sizeof(int*) * (2*nrofvars + 1) );

	for( i = 0; i <= 2*nrofvars; i++ ) bin_occ[ i ] = 0; 
	bin_occ += nrofvars;
	bin_lut += nrofvars;

	for( i = 0; i < nrofclauses; i++ )
	   if( Clength[ i ] == 2 )
	   {
		bin_total += 2;
		bin_occ[ Cv[ i ][ 0 ] ]++;
		bin_occ[ Cv[ i ][ 1 ] ]++;
	   }

	bin_table = (int*) malloc( sizeof(int) * bin_total );

	bin_total = 0;
	for( i = -nrofvars; i <= nrofvars; i++ )
	{
	    bin_lut[ i ] = bin_table + bin_total;
	    bin_total += bin_occ[ i ];
	    bin_occ[ i ] = 0;
	}

	for( i = 0; i < nrofclauses; i++ )
	   if( Clength[ i ] == 2 )
	   {
		bin_lut[ Cv[i][0] ][ bin_occ[ Cv[i][0] ]++ ] = -Cv[i][1];
		bin_lut[ Cv[i][1] ][ bin_occ[ Cv[i][1] ]++ ] = -Cv[i][0];
	   }	

	for( i = -nrofvars; i <= nrofvars; i++ ) MarkArray[ i ] = 0;

	for( i = -nrofvars; i <= nrofvars; i++ )
	{
	    if( i == 0 ) continue;

#ifdef TIMEOUT
	    if( (int) clock() > CLOCKS_PER_SEC * TIMEOUT ) break;
#endif
	    failed_flag = 0;
	    hla_size    = 1;

	    MarkArray[ i ] = i;

	    for( j = 0; j < bin_occ[ i ]; j++ )
	    {
		int imp = bin_lut[ i ][ j ];
		if( MarkArray[ imp ] == i ) continue;

	        head = bin_que;
	        tail = bin_que;

	        *(head++) = imp;	
	        MarkArray[ imp ] = i;
		hla_size++;

	        while( tail < head )
	        {
		    int lit = *(tail++);

		    for( k = 0; k < bin_occ[ lit ]; k++ )
		    {
			int _lit = bin_lut[ lit ][ k ];



			if( MarkArray[ _lit ] != i )
			{
			    if( MarkArray[ -_lit ] == i ) 
			    {
				failed_flag = 1;
				goto hla_failed_literal;
			    }

			    MarkArray[ _lit ] = i;
			    *(head++) = _lit;
			    hla_size++;
			}
		    }
		}
		MarkArray[ imp ] = 0;
	    }



	     for( k = 1; k <= literal_occ[ i ][ 0 ]; k++ )
	     {  
		int clsidx = literal_occ[ i ][k];
		if( Clength[ clsidx ] == 2 )
		{
		    for( j = 0; j < Clength[ clsidx ]; j++ )
			if( MarkArray[ -Cv[ clsidx ][j] ] == i )
			{
			    int s;
			    for( s = 0; s < bin_occ[ Cv[clsidx][0] ]; s++ ) 
				if( bin_lut[ Cv[clsidx][0] ][ s ] == -Cv[clsidx][1] ) 
					bin_lut[ Cv[clsidx][0] ][ s ] = 0;

			    for( s = 0; s < bin_occ[ Cv[clsidx][1] ]; s++ ) 
				if( bin_lut[ Cv[clsidx][1] ][ s ] == -Cv[clsidx][0] ) 
					bin_lut[ Cv[clsidx][1] ][ s ] = 0;



			     Clength[ clsidx ] = 0;
			}
		} 
	    }

	    for( j = 0; j < bin_occ[ i ]; j++ )
		MarkArray[ bin_lut[i][j] ] = i;

	    if( unit_flag == 0 )
	      for( k = 1; k <= literal_occ[ i ][ 0 ]; k++ )
	      {
		int clsidx = literal_occ[ i ][k];
		if( Clength[ clsidx ] > 2 )
		{
		    for( j = 0; j < Clength[ clsidx ]; j++ )
			if( MarkArray[ -Cv[ clsidx ][j] ] == i )
			{
		    	     printf("c found hidden tautology, will remove %i with size %i\n", clsidx, Clength[ clsidx ] );
			     Clength[ clsidx ] = 0;
			}
			else if( Cv[ clsidx ][ j ] != i && MarkArray[ Cv[ clsidx ][ j ] ] == i )
			{
		    	     printf("c found hidden literal, will remove %i (%i) from clause %i of size %i\n", Cv[ clsidx ][ j ], i, clsidx, Clength[ clsidx ] );
			     Cv[ clsidx ][ j-- ] = Cv[ clsidx ][ --Clength[ clsidx ] ];
			}
		} 
	      }

	    hla_failed_literal:;
		
	    if( failed_flag )
	    {
		for( k = 1; k <= literal_occ[ i ][ 0 ]; k++ )
		{
		    int clsidx = literal_occ[i][k]; int *cls = Cv[clsidx];
				
		    if( Clength[ clsidx ] == 2 )
		    {
			int s;
			for( s = 0; s < bin_occ[ cls[0] ]; s++ ) if( bin_lut[ cls[0] ][ s ] == -cls[1] ) bin_lut[ cls[0] ][ s ] = 0;
			for( s = 0; s < bin_occ[ cls[1] ]; s++ ) if( bin_lut[ cls[1] ][ s ] == -cls[0] ) bin_lut[ cls[1] ][ s ] = 0;
		    }


		    Clength[ clsidx ] = 1;
		    cls[ 0 ] = i;

		    unit_flag = 1;
		}
	    }
	}

	bin_occ -= nrofvars;
	bin_lut -= nrofvars;

	free( bin_occ );
	free( bin_lut );
	free( bin_que );
	free( bin_table );

	free_local_datastructures();
	compactCNF( );

	return unit_flag;
}

void hidden_blocked_clause_elimination( )
{
	int i, j, k;
	int *c;
	int *bin_occ, **bin_lut, *bin_table, bin_total = 0;
	int *bin_que, *_que, *tmp;
	int eliminated = 0, _eliminated = 0, hidden_flag;

	allocate_local_datastructures();

	bin_occ = (int * ) malloc( sizeof(int ) * (2*nrofvars + 1) );
	bin_que = (int * ) malloc( sizeof(int ) * (2*nrofvars + 1) );
	bin_lut = (int **) malloc( sizeof(int*) * (2*nrofvars + 1) );

	for( i = 0; i <= 2*nrofvars; i++ ) bin_occ[ i ] = 0; 
	bin_occ += nrofvars;
	bin_lut += nrofvars;

	for( i = 0; i < nrofclauses; i++ )
	   if( Clength[ i ] == 2 )
	   {
		bin_total += 2;
		bin_occ[ Cv[ i ][ 0 ] ]++;
		bin_occ[ Cv[ i ][ 1 ] ]++;
	   }

	bin_table = (int*) malloc( sizeof(int) * bin_total );

	bin_total = 0;
	for( i = -nrofvars; i <= nrofvars; i++ )
	{
	    bin_lut[ i ] = bin_table + bin_total;
	    bin_total += bin_occ[ i ];
	    bin_occ[ i ] = 0;
	} 

	for( i = 0; i < nrofclauses; i++ )
	   if( Clength[ i ] == 2 )
	   {
		bin_lut[ Cv[i][0] ][ bin_occ[ Cv[i][0] ]++ ] = -Cv[i][1];
		bin_lut[ Cv[i][1] ][ bin_occ[ Cv[i][1] ]++ ] = -Cv[i][0];
	   }	

    do
    {
	hidden_flag = (_eliminated == eliminated);

	_eliminated = eliminated;

	for( i = -nrofvars; i <= nrofvars; i++ ) MarkArray[ i ] = -1;

	for( i = 0; i < nrofclauses; i++ )
	   if( Clength[ i ] >= 2 )
	   {
#ifdef TIMEOUT
	    if( (int) clock() > CLOCKS_PER_SEC * TIMEOUT ) goto end_HBCE;
#endif
		c = Cv[i];

		_que = bin_que;
		tmp  = bin_que;
		for( j = 0; j < Clength[ i ]; j++ )
		{
		    *(_que++) = Cv[ i ][ j ];
		    MarkArray[ -Cv[i][j] ] = i;
		}

              if( hidden_flag )  
		while( tmp < _que )
		{
		    int lit = *(tmp++);
		    for( j = 0; j < bin_occ[ lit ]; j++ )
		    {
			int _lit = bin_lut[ lit ][ j ];

			if( (Clength[i] == 2) && (lit == Cv[i][0]) && (_lit == -Cv[i][1]) ) continue;
			if( (Clength[i] == 2) && (lit == Cv[i][1]) && (_lit == -Cv[i][0]) ) continue;

			if( MarkArray[ -_lit ] != i )
			{
			    if( MarkArray[ _lit ] == i ) 
			    {
				printf("c found a tautological clause %i\n", i );

				if( Clength[ i ] == 2 )
				{
				    int s;
				    for( s = 0; s < bin_occ[ c[0] ]; s++ ) 
					if( bin_lut[ c[0] ][ s ] == -c[1] ) 
					    bin_lut[ c[0] ][ s ] =   c[0];

				    for( s = 0; s < bin_occ[ c[1] ]; s++ ) 
					if( bin_lut[ c[1] ][ s ] == -c[0] ) 
					    bin_lut[ c[1] ][ s ] =   c[1];
				}

				Clength[ i ] = 0;
				
				_que = bin_que;
				goto next_clause;
			    }
			    MarkArray[ -_lit ] = i;
			    *(_que++) = _lit;
			}
		    }
		}

		for( j = 0; j < Clength[ i ]; j++ )
		{
		    int lit = Cv[i][j];

		    for( k = 1; k <= literal_occ[ -lit ][ 0 ]; k++ )
		    {
		      int clsidx = literal_occ[-lit][k];
		      if( Clength[ clsidx ] > 0 ) 
		 	if( count_clashing_literals( clsidx, i ) == 1 )
			    goto next_clause;
		    }

		    if( Clength[ i ] == 2 )
		    {
		        int s;
		        for( s = 0; s < bin_occ[ c[0] ]; s++ ) 
			    if( bin_lut[ c[0] ][ s ] == -c[1] ) 
				bin_lut[ c[0] ][ s ] =   c[0];

			for( s = 0; s < bin_occ[ c[1] ]; s++ ) 
			    if( bin_lut[ c[1] ][ s ] == -c[0] ) 
				bin_lut[ c[1] ][ s ] =   c[1];
		    }
		    Clength[ i ] = 0;
		    eliminated++;
		    goto next_clause;
		}
		next_clause:;
	   }
    }
    while( (hidden_flag == 0) || (eliminated != _eliminated) );

        end_HBCE:;

	bin_occ -= nrofvars;
	bin_lut -= nrofvars;

	free( bin_occ );
	free( bin_lut );
	free( bin_que );
	free( bin_table );

	free_local_datastructures();
	compactCNF( );

}

void blocked_clause_elimination( int flag )
{
	int i, j, k, l;
	int eliminated = 0, _eliminated;
	int last_eliminated = -1;
	int *toCheck;
	int blocking_literal;

	toCheck = (int*) malloc (sizeof(int) * nrofclauses);
	for( i = 0; i < nrofclauses; i++ ) toCheck[ i ] = 1;

#ifdef CCE
	int coverstamp = 0;
	int *intersection, intersection_size, tmpidx;
	int *_clause, _length; 
	int last_cover_index;

	intersection = (int*) malloc( sizeof(int) * (2*nrofvars+1) );
	for ( i = 0; i <= 2*nrofvars; i++ )
	    intersection[ i ] = coverstamp;
	intersection += nrofvars;

	_clause = (int*) malloc( sizeof(int) * (nrofvars+1) );
#endif
	allocate_local_datastructures();

	do
	{
	  _eliminated = eliminated;

	  for( i = -nrofvars; i <= nrofvars; i++ ) MarkArray[ i ] = -1;

	  for( i = 0; i < nrofclauses; i++ )
	  {
	    if( last_eliminated == i ) goto end_BCE;

	    if( toCheck[ i ] == 0 ) continue;
	    toCheck[ i ] = 0;

	    if( Clength[ i ] > 0 )
	    {
#ifdef TIMEOUT
	    if( (int) clock() > CLOCKS_PER_SEC * TIMEOUT ) goto end_BCE;
#endif

		mark_complement_of_literals( i );
#ifdef CCE
		_length = 0;
		for( j = 0; j < Clength[ i ]; j++ ) _clause[ _length++ ] = Cv[i][j];

		last_cover_index = _length; j = 0;
		while( j != last_cover_index )
		{
		    if( j == _length ) j = 0;

		    int lit = _clause[ j ];
		    tmpidx = -1;
#else

		for( j = 0; j < Clength[ i ]; j++ )
		{
		    int lit = Cv[i][j];
#endif


		    for( k = 1; k <= literal_occ[ -lit ][ 0 ]; k++ )
		    {
		      int clsidx = literal_occ[ -lit ][ k ];

		      if( Clength[ clsidx ] > 0 ) 
		 	if( count_clashing_literals( clsidx, i ) == 1 )
#ifdef CCE
			{
			     if( tmpidx == -1 )  
			     {
				coverstamp++;
				tmpidx = clsidx;

				intersection_size = 0;
				for( l = 0; l < Clength[ clsidx ]; l++ )
				{
				   int _lit = Cv[ clsidx ][ l ];
				   if( _lit == -lit ) continue;


				   {
					intersection[ _lit ] = coverstamp;
					intersection_size++;
				   }
				}
				if( intersection_size == 0 ) goto next_literal;
			     }
			     else
			     {
				intersection_size = 0;
				for( l = 0; l < Clength[ clsidx ]; l++ )
				{
				   int _lit = Cv[ clsidx ][ l ];
				   if( intersection[ _lit ] == coverstamp )
				   {
					intersection[ _lit ]++;
					intersection_size++;
				   }
				}
				coverstamp++;

				if( intersection_size == 0 ) goto next_literal;
			     }				
			}
#else
			goto next_literal;
#endif
			blocking_literal = lit;
		    }

#ifdef CCE
		    if( tmpidx != -1 )
		    {
			for( l = 0; l < Clength[ tmpidx ]; l++ )
			{
			    int _lit = Cv[ tmpidx ][ l ];
#ifdef CLE
			    if( (intersection[ _lit ] == coverstamp) && (MarkArray[ -_lit ] == i) )
			    {
				int m;
				printf("!!!!!!!!!!!!!!!!!!!!!!!!\n");

				MarkArray[ -_lit ] = -1;
				Cv[ tmpidx ][ l ] = Cv[ tmpidx ][ --Clength[ tmpidx ] ];
				for( m = 1; m <= literal_occ[ lit ][ 0 ]; m++ )
				    if( literal_occ[ lit ][ m ] == tmpidx )
				    {
					literal_occ[ lit ][ m ] = literal_occ[ lit ][ literal_occ[ lit ][ 0 ]-- ];
				    }
			    }else
#endif
			    if( (intersection[ _lit ] == coverstamp) && (MarkArray[ -_lit ] != i) )
			    {
				MarkArray[ -_lit ] = i;
				_clause [ _length++ ] = _lit;
				last_cover_index = j;
				if( j == 0 ) last_cover_index = _length;

			    }
			}
			goto next_literal;
		    }
		    else
#endif
		    {

			for( l = 0; l < Clength[ i ]; l++ )
			{
			    int m;
			    int lit = Cv[ i ][ l ]; 
			    if( lit != blocking_literal )
				for( m = 1; m <= literal_occ[ -lit ][ 0 ]; m++ ) toCheck[ literal_occ[ -lit ][ m ] ] = 1;
			}

		        Clength[ i ] = 0;
		        eliminated++;
			last_eliminated = i;
			goto next_clause;
		    }
		    next_literal:;
#ifdef CCE
		    j++;
#endif
		}


#ifdef CCE
#ifdef CLA
		if( flag && Clength[i] > 0 && _length > Clength[i] )
		{ 

		    Cv[i] = (int*) realloc( Cv[i], sizeof(int) * _length );

		    for( j = 0; j < _length; j++ )  
		    {
			int lit = _clause[j];
			Cv[i][j] = lit;

			if( j >= Clength[ i ] )
			{
			    int resize = literal_occ[ lit ][ 0 ] + 2;
			    literal_occ[ lit ] = (int*) realloc( literal_occ[ lit ], sizeof(int) * resize );
			    literal_occ[ lit ][ ++literal_occ[ lit ][ 0 ] ] = i;
			}

		    }
		    Clength[i] = _length;
		}
#endif
#endif
		next_clause:;
	    }
	  }
	}
	while( eliminated != _eliminated );

	end_BCE:;

	free(toCheck);
	free_local_datastructures();
	compactCNF( );
}

void print_components( const int nrofcomponents )
{
	int i, offset = 0, counter;
#ifdef PRINT_COMPONENTS
	int j, current;
#endif
        for( i = 1; i <= nrofcomponents; i++ )
	{
	    counter = 0;
	    do
	    {
#ifdef PRINT_COMPONENTS
		current = clause_order_array[ counter + offset ];
		for( j = 0; j < Clength[ current ]; j++ )
		    printf("%i ", Cv[ current ][ j ] );
		printf("0\n");
#endif
		counter++;
	    }
	    while( (counter + offset < nrofclauses) && (clause_parent[ clause_order_array[ counter + offset ] ] != INITIAL_PARENT) );

	    offset += counter;
	    printf("c component %i has %i clauses\n", i, counter); 
	}
}


void allocate_local_datastructures( )
{
	int i;

	MarkArray = (int*) malloc( sizeof(int) * (2*nrofvars + 1) );
	for( i = 0; i <= (2*nrofvars); i++ )
		MarkArray[ i ] = -1;

	MarkArray += nrofvars;	

	allocateVc( &literal_occ, &literal_lut );
	literal_occ += nrofvars;
}

void free_local_datastructures( )
{
	literal_occ -= nrofvars;
	freeVc( literal_occ, literal_lut );

	MarkArray -= nrofvars;
	free( MarkArray );
}

void compute_sign_balance( )
{
	int i, j, active = 0, *_occurence; 
	double sign_bias = 0, pos, neg;

	_occurence = (int*) malloc( sizeof(int) * (2*nrofvars+1) );
	_occurence += nrofvars;

	for( i = 1; i <= nrofvars; i++ )
	{
	    _occurence[  i ] = 0;
	    _occurence[ -i ] = 0;
	}

	for( i = 0 ; i < nrofclauses; i++ )
	    for( j = 0; j < Clength[ i ]; j++ )
		_occurence[ Cv[i][j] ]++;

	for( i = 0; i <= nrofvars; i++ )
	{
	    pos = (double) _occurence[  i ];
	    neg = (double) _occurence[ -i ];

	    if( (pos > 0) && (neg > 0) )
	    {
		active++;
		sign_bias +=  (pos / neg) + (neg / pos);
	    }
	}

	sign_bias = (sign_bias / active) - 2;

	printf("c stat :: sign balance is biased by %.3f\n", sign_bias );

	_occurence -= nrofvars;	
	free( _occurence );
}

#endif
