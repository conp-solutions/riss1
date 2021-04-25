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


#include "unitpropagation/csp_propagation.h"


#include <iostream>

CspPropagation::CspPropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, const uint32_t max_size, const StringMap& commandline):
	implication_closure ( true),
	search_sat( false ),
	long_conflicts ( false),
	swap_watches ( true),
	keep_order_prop ( true),
	keep_order_remo ( true),
	bfs ( true),
	unit_fifo(true),
	lastPhase ( true),
	reinitFormulaFirst(true),
	prefetched_items ( 3),
	doCsp( true ),
	minXorSize( 5)
{
	#ifdef PROPAGATION_METRICS
		processed_clauses = 0;
		clauses_size_sum = 0;
		processed_literals = 0;
		calls_propagate = 0;
		calls_propagate_lit1 = 0;
		calls_propagate_dual = 0;
	#endif

	
	this->var_cnt = search.var_cnt;
	this->max_size = max_size;
	set_parameter( commandline );

	unseen_ind = 0;

	VEC_CREATE( lit_t, impliedLits );
	VEC_CREATE( reasonStruct, reasons );

	
	VEC_CREATE_SPACE( VEC_TYPE( watch_t ),  watch, max_index( var_cnt ) );
	
	if( doCsp ){
		VEC_CREATE_SPACE( VEC_TYPE(CL_REF),  csp, var_cnt + 1 );
	}
	
	
	VEC_CREATE_SPACE( VEC_TYPE( dual_watch_t ),  implication, max_index( var_cnt ) );

	
	seen_implications = boolarray_create( var_cnt + 1 );

	unit_queue = RINGBUFFER_CREATE(lit_t, var_cnt+1);
	
	if( clause_set != 0 ) 
	{
		uint32_t avglen = VEC_SIZE( CL_REF, (*clause_set) ) / (var_cnt + 1);
		for( uint32_t i = 0; i< max_index( var_cnt ); ++i )
		{
			
			VEC_TYPE( watch_t ) tmp;
			VEC_CREATE_SPACE(watch_t, tmp, avglen + 4 );
			VEC_PUSH_BACK(VEC_TYPE( watch_t ),   watch, tmp );

			
			VEC_TYPE( dual_watch_t ) tmp2;
			VEC_CREATE( dual_watch_t, tmp2);
			VEC_PUSH_BACK( VEC_TYPE( dual_watch_t ), implication, tmp2 );
		}
		
		if( doCsp ){
			
			VEC_CREATE_SPACE( VEC_TYPE(CL_REF), csp, var_cnt+1 );
			for( uint32_t i = 0; i<= var_cnt; ++i ){
				VEC_TYPE( CL_REF ) tmp;
				VEC_CREATE(CL_REF, tmp);
				VEC_PUSH_BACK(VEC_TYPE( CL_REF ), csp, tmp );
			}
		}

		
		for( uint32_t j = 1; j <= var_cnt; ++j ) if( assi_get_polarity( search.assi, j ) != UNDEF )
			 enqueue_literal( search, lit( j, assi_get_polarity( search.assi, j ) ), reasonStruct() );

		VEC_TYPE( CL_REF ) tmp;
		VEC_CREATE( CL_REF, tmp );
		reinit(search, (*clause_set), tmp, var_cnt );
		VEC_DESTROY( CL_REF, tmp );
	}
	
	
	 inserted_variable = boolarray_create( var_cnt + 1 );
	 
	
	MON_REG( "propClLen" , DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "WLlen"     , DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "binWLlen"  , DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "breakWL"   , DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "breakbinWL", DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "blockedSat", DataMonitor::HISTOGRAM, 'u' ); 
	MON_REG( "watchedSat", DataMonitor::HISTOGRAM, 'u' ); 


	 
}

CspPropagation::~CspPropagation()
{
		
		for(uint32_t i = 0; i<VEC_SIZE(VEC_TYPE(watch_t),   watch ); ++i )
		{
			VEC_DESTROY(watch_t,   watch[i] );
		}
		VEC_DESTROY(VEC_TYPE(watch_t),   watch );

		RINGBUFFER_DESTROY(lit_t,   unit_queue );
		
		
		for(uint32_t i = 0; i<VEC_SIZE( VEC_TYPE( dual_watch_t ), implication ); ++i )
		{
			VEC_DESTROY( dual_watch_t,   implication[i] );
		}
		VEC_DESTROY( VEC_TYPE( dual_watch_t ), implication );
		
		boolarray_destroy(  inserted_variable );
		boolarray_destroy(  seen_implications );
		
	#ifdef PROPAGATION_METRICS
		std::cerr << "c kind_of_data    clss size.sum size.avg literals prop prop_bin prop_long" << std::endl;
		std::cerr << "c dup_metric_data "
							<< processed_clauses << " "
							<< clauses_size_sum << " "
							<< (double)clauses_size_sum / (double)processed_clauses << " "
							<< processed_literals << " "
							<< calls_propagate << " "
							<< calls_propagate_lit1 << " "
							<< calls_propagate_dual << std::endl;
	#endif
}

void CspPropagation::enqueue_decision(searchData& search, lit_t decision_literal )
{
	
	DO( std::cerr << "enqueue decision "; )
	
	 enqueue_literal( search, decision_literal, reasonStruct() );
}
			
			
void CspPropagation::propagate(searchData& search, VEC_TYPE( conflictStruct  )& conflict_clauses )
{
	lit_t literal;						
	conflictStruct conflict;	
	conflictStruct shortC;	

	#ifdef PROPAGATION_METRICS
		calls_propagate++;
	#endif

	VEC_CLEAR( lit_t, impliedLits );
	VEC_CLEAR( CL_REF, reasons );
	
	while ( !RINGBUFFER_IS_EMPTY(lit_t, unit_queue ) ){
		
		literal = dequeue_literal();
		
		if( shortC.empty() ) shortC = propagate_implications( search,literal);
		else propagate_implications( search,literal);
		
		if( !long_conflicts  && !shortC.empty() ) break;
		
		conflict = conflict = propagate_literal(search,literal);
		if( !conflict.empty() ) break;
		else{
			conflict = propagate_csp(search,literal);
			if( !conflict.empty() ) break;
		}

	}

	
	if ( !conflict.empty() ) {
		VEC_PUSH_BACK( conflictStruct, conflict_clauses, conflict );
	} else {
		if ( !shortC.empty() ) {
			VEC_PUSH_BACK( conflictStruct, conflict_clauses, shortC );
		}
	}

	RINGBUFFER_CLEAR(lit_t, unit_queue );
	return;
}

bool CspPropagation::clauses_added(searchData& search, VEC_TYPE(CL_REF)& clauses){
	bool fail = true;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i ){
		fail = fail && clause_added( search, clauses[i]);
	}
	return fail;
}


bool CspPropagation::clause_added(searchData& search, CL_REF clause)
{
	assert( clause != 0 );
	assert( CL_SIZE( gsa.get(clause) )> 0 );
	
	
	bool not_unsatisfied = true;
	
	
	CLAUSE& cl = gsa.get(clause);
	
	if( CL_SIZE(cl) == 1 ){
		lit_t literal = CL_GET_LIT( cl, 0 );
		
		bool ret = enqueue_literal( search, literal, reasonStruct(clause) );
		
		search.VAR_LEVEL( var(literal) ) = 0;
		return ret;
	}
	
	uint32_t watched_literals = 0;
	
	
	uint32_t i = 0;
	
	for( ; i < CL_SIZE(cl) && watched_literals<2; ++i )
	{
		const lit_t lit1 = CL_GET_LIT( cl, i );
		if( !assi_is_unsat( search.assi, lit1) )
		{
			
			CL_SWAP_LITS(cl, i, watched_literals);
			watched_literals++;	
		}
	}
	
	if( watched_literals == 1 ){
		
			if( assi_is_undef( search.assi, var( CL_GET_LIT( cl, 0 ) ) ) ){
				enqueue_literal(search, CL_GET_LIT( cl, 0 ) , reasonStruct(clause) );	
				
			}
	} else {
		if( watched_literals == 0 ){
			
			not_unsatisfied = false;
			cerr << "c added unsatisfied clause" << endl;
		}
	}
	
	
	if( CL_SIZE(cl) > 2 ){
		watch_t watchMe;
		watchMe.clause = clause;
#ifdef BLOCKING_LIT
		watchMe.lit1 = CL_GET_LIT( cl, 0 );
#endif
		 watch_clause(search, watchMe, CL_GET_LIT(cl, 0) );
		 watch_clause(search, watchMe, CL_GET_LIT(cl, 1) );
	} else {
		
		 watch_dual( search, clause );
	}
	
	
	return not_unsatisfied;
}

void CspPropagation::clauses_removed(searchData& search, VEC_TYPE(CL_REF)& clauses )
{
	for( uint32_t cind = 0; cind < VEC_SIZE( CL_REF, clauses ); ++cind )
	{
		
		CLAUSE& cl = gsa.get( clauses[cind] );
		if( CL_SIZE(cl) == 2 ){
			for(uint32_t i = 0; i<2; ++i){
#ifndef USE_IMPLICIT_BINARY
				for(uint32_t ind = 0; ind < VEC_SIZE( dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ] ); ind ++ ){
					if( (implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ][ind]).clause == clauses[cind] ){
						
						if(  keep_order_remo ) VEC_ERASE(dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ], ind );
						else VEC_ERASE_NO_ORDER(dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ], ind );
						break;
					}
				}
#else
				
				const lit_t other = CL_GET_LIT( cl, i )	 ^ CL_GET_LIT( cl, 1 ) ^ CL_GET_LIT( cl, 0 );
				for(uint32_t ind = 0; ind < VEC_SIZE( dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ] ); ind ++ ){
					if( (implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ][ind]).other == other ){
						
						if(  keep_order_remo ) VEC_ERASE(dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ], ind );
						else VEC_ERASE_NO_ORDER(dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, i ) ) ) ], ind );
						break;
					}
				}
#endif
			}
			continue;
		}

		DO( std::cerr << "c UP remove cls " << std::hex << clauses[cind] << std::dec << std::endl; )
		
		
		for(uint32_t i = 0; i<2; ++i)
		{
			
			
			lit_t watched_literal = inv_lit( CL_GET_LIT(cl,i) );
			
			VEC_TYPE( watch_t )& current_watchlist =  watch[ index( watched_literal ) ];
			
			for(uint32_t ind = 0; ind < VEC_SIZE(watch_t,  current_watchlist ); ind++)
			{
				if( clauses[cind] == current_watchlist[ind].clause )
				{
					
					if(  keep_order_remo ) VEC_ERASE(watch_t, current_watchlist, ind );
					else VEC_ERASE_NO_ORDER(watch_t, current_watchlist, ind );
					
					break;
				}
			}
		}
	}
}

void CspPropagation::backtrack(searchData& search, int32_t level , VEC_TYPE(var_t)& undefed_literals)
{
	search.current_level = level;
	
	RINGBUFFER_CLEAR( lit_t, unit_queue );
	
	int32_t max_level = search.VAR_LEVEL( var( STACK_TOP(lit_t, search.trail) ) );
	
	
	
	
	
	if( !lastPhase && max_level != 0){
		while( STACK_SIZE(lit_t, search.trail ) && search.VAR_LEVEL( var( STACK_TOP(lit_t, search.trail) ) ) >= max_level ){
			
			const var_t& variable = var( STACK_TOP(lit_t, search.trail) );
			search.VAR_REASON( variable ) = reasonStruct();
			search.VAR_LEVEL( variable ) = -1;
		
			
			boolarray_set( seen_implications, variable, false);
		
			
			assi_undef_variable( search.assi, variable );
			assi_undef_variable_backup( search.assi, variable );

			
			VEC_PUSH_BACK( lit_t, undefed_literals, variable );
	
			
			STACK_POP(lit_t,  search.trail );
		}
	}
	
	while( STACK_SIZE(lit_t, search.trail ) && search.VAR_LEVEL( var( STACK_TOP(lit_t, search.trail) ) ) > level )
	{
		
		const var_t& variable = var( STACK_TOP(lit_t, search.trail) );
		search.VAR_REASON( variable ) = reasonStruct();
		search.VAR_LEVEL( variable ) = -1;
		
		
		boolarray_set( seen_implications, variable, false);
		
		
		assi_undef_variable_backup( search.assi, variable );

		
		VEC_PUSH_BACK( lit_t, undefed_literals, variable );
	
		
		STACK_POP(lit_t,  search.trail );	
	}
	
}



void CspPropagation::watch_clause(searchData& search, watch_t clause, lit_t watched_literal )
{
	VEC_PUSH_BACK(watch_t,   watch[ index( inv_lit(watched_literal) ) ], clause);
}


void CspPropagation::watch_csp( CL_REF clause, lit_t literal ){
	
	assert ( var( CL_GET_LIT( gsa.get(clause), 0 ) ) == var(literal) || var( CL_GET_LIT( gsa.get(clause), 1 ) ) == var(literal) );
	VEC_PUSH_BACK(CL_REF, csp[ var(literal) ], clause);
}

void CspPropagation::watch_dual(searchData& search, CL_REF clause )
{
	assert( clause != 0 );
	assert( CL_SIZE( gsa.get(clause) ) == 2 );
	  
	CLAUSE& cl = gsa.get(clause);
	dual_watch_t dual_watch;

#ifndef USE_IMPLICIT_BINARY
	dual_watch.clause = clause;
#endif
	dual_watch.other = CL_GET_LIT( cl, 1 );
	VEC_PUSH_BACK( dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, 0 ) ) ) ], dual_watch );


	dual_watch.other = CL_GET_LIT( cl, 0 );
	VEC_PUSH_BACK( dual_watch_t, implication[ index( inv_lit( CL_GET_LIT( cl, 1 ) ) ) ], dual_watch );

}

conflictStruct CspPropagation::propagate_implications(searchData& search, lit_t literal)
{
	
	if( boolarray_get(  seen_implications, var(literal) ) ) return conflictStruct();
	#ifdef PROPAGATION_METRICS
		calls_propagate_dual ++;
	#endif
	uint32_t queue_size = RINGBUFFER_SIZE( lit_t,  unit_queue );
	unseen_ind = ( unseen_ind > queue_size ) ? queue_size : unseen_ind;
	conflictStruct clause_to_return;
	
	while( 1 ){
		
		VEC_TYPE( dual_watch_t )& implications =  implication[ index( literal ) ];

		
		if( ! boolarray_get(  seen_implications, var(literal)) ){
			boolarray_set(  seen_implications, var(literal), true);
			
			const uint32_t lsize = VEC_SIZE( dual_watch, implications );
			MON_EVENT( "binWLlen", &lsize );
			for( uint32_t i = 0; i < lsize; ++i )
			{
				const lit_t other = implications[i].other;
				MON_EVENT( "propClLen", (void*)2 );

				
				if( assi_variable_equal( search.assi, var( other ), pol( other ) ) ) continue;
				#ifndef USE_IMPLICIT_BINARY
				assert( implications[i].clause != 0 );
				#endif

				#ifndef USE_IMPLICIT_BINARY
				if(! enqueue_literal(search, other, reasonStruct(implications[i].clause) ) ){
				#else	
				if(! enqueue_literal(search, other, reasonStruct( inv_lit(literal) , true ) ) ){
				#endif
					MON_EVENT( "breakbinWL", &i );
					#ifndef USE_IMPLICIT_BINARY
					assert( other == CL_GET_LIT( gsa.get(implications[i].clause), 0 ) || other == CL_GET_LIT( gsa.get(implications[i].clause), 1 ) );
					assert( inv_lit(literal) == CL_GET_LIT( gsa.get(implications[i].clause), 0 ) || inv_lit(literal) == CL_GET_LIT( gsa.get(implications[i].clause), 1 ) );
					#endif
					if( !long_conflicts ) return conflictStruct( inv_lit(literal), other);
					else clause_to_return = conflictStruct( inv_lit(literal), other);
				} else {
					
					queue_size ++;
				}
			}
		
		}

		
		if( ! implication_closure || unseen_ind == queue_size ) break;
		
		
		literal = queue_get_literal(unseen_ind);
		
		
		unseen_ind ++;
		
	}	
	
	
	return clause_to_return;
}

bool CspPropagation::enqueue_literal(searchData& search,  lit_t literal, const reasonStruct& reason ){
	
	if( assi_variable_equal( search.assi, var(literal), pol( inv_lit(literal) ) ) ){
		return false;
	}
	
	if( assi_variable_equal( search.assi, var(literal), pol( literal ) ) ){
		return true;
	}
	#ifdef USE_PREFETCHING
		
		prefetch( &(watch[ index( literal ) ]) );
		prefetch( &(csp[ var( literal ) ]) );
	#endif
	#ifdef USE_DUAL_PREFETCHING
		prefetch( &(implication[ index( literal ) ]) );
	#endif
	
	
	RINGBUFFER_PUSH_BACK(lit_t, unit_queue, literal );	
	
	search.VAR_REASON( var(literal) ) = reason;	
	
	search.VAR_LEVEL( var(literal) ) = search.current_level;
	
	
	STACK_PUSH(lit_t, search.trail, literal);
	
	
	assi_set_polarity_backup( search.assi, var(literal), pol( literal ) );
	
	
	if(  RINGBUFFER_SIZE( lit_t,  unit_queue ) <= prefetched_items ){
		prefetch_watch( literal );
	}
	
	return true;
}

lit_t CspPropagation::dequeue_literal()
{
	lit_t literal;
	if( bfs ){	
		RINGBUFFER_POP_FRONT(lit_t, unit_queue, literal);
		if(  RINGBUFFER_SIZE( lit_t,  unit_queue ) > prefetched_items ) prefetch_watch( queue_get_literal(prefetched_items) );
	}
	else { 
		RINGBUFFER_POP_BACK(lit_t, unit_queue, literal);
		if(  RINGBUFFER_SIZE( lit_t,  unit_queue ) > prefetched_items ) prefetch_watch( queue_get_literal(prefetched_items) );
	}
	return literal;
}

void CspPropagation::literals_conflicted( VEC_TYPE(lit_t)& lits )
{

}

lit_t CspPropagation::queue_get_literal( const uint32_t ind ) const
{
	lit_t literal = NO_LIT;
	if( bfs ){	
		RINGBUFFER_GET_BACK( lit_t,  unit_queue, ind, literal );
	}
	else {
		RINGBUFFER_GET_FRONT( lit_t,  unit_queue, ind, literal );
	}
	
	return literal;
}


void  CspPropagation::prefetch_watch( const lit_t literal ) const
{
#ifdef  PREFETCHINGMETHOD2
	
	const VEC_TYPE(watch_t)& current_watchlist =  watch[ index( literal ) ];	
	
	
	for( uint32_t i = 0 ; i<VEC_SIZE(watch_t, current_watchlist ); ++i ){
		#ifdef BLOCKING_LIT
			if( !assi_is_sat( search.assi, current_watchlist[i].lit1) ) prefetch( gsa.addr(current_watchlist[i].clause) );
		#else
			prefetch( gsa.addr(current_watchlist[i].clause) );
		#endif
	}
#endif
}


conflictStruct CspPropagation::propagate_csp(searchData& search, lit_t literal){
	if( !doCsp ) return conflictStruct();
	
	VEC_TYPE(CL_REF)& clist =  csp[ var( literal ) ];

#ifdef PREFETCHINGMETHOD1
	
	for( uint32_t i = 0 ; i<VEC_SIZE(watch_t, clist ); ++i ){
			prefetch( gsa.addr(clist[i]) );
	}
#endif
	uint32_t non_moved_cls = 0;
	uint32_t gap = 0;

#ifdef PREFETCHINGMETHOD3
	uint32_t pMax = VEC_SIZE(watch_t, clist ) < 4 ? VEC_SIZE(watch_t, clist ) : 4;
	for( uint32_t i = 0 ; i<pMax; ++i ){
		prefetch( gsa.addr(clist[i]) );
	}
#endif

	const uint32_t lsize = VEC_SIZE(watch_t, clist );
	for( uint32_t watchind = 0 ; watchind < lsize; ++ watchind )
	{

#ifdef PREFETCHINGMETHOD3
		if( watchind + 4 < lsize ){
			prefetch( gsa.addr(clist[ watchind + 4]) );
		}
#endif
	
		const CL_REF clause = clist[ watchind ];
		CLAUSE& cl = gsa.get(clause);
		const uint32_t size = CL_SIZE( cl );

		const uint32_t literal_position=1;	
		
		
	
		if( var(CL_GET_LIT( cl, 0)) == var(literal ) ){
			CL_SWAP_LITS( cl, 0, 1);
		}
		lit_t other_watched_literal = CL_GET_LIT( cl, 0);

	
		
		
		if( cl.isXor() ){
			
			
			
			if( clause == search.VAR_REASON( var(literal) ).getCL() && !search.VAR_REASON( var(literal) ).isLit() ){
				
				CL_SWAP_LITS( cl, 0, 1);
				clist[ non_moved_cls ] = clist[ watchind ];
				non_moved_cls ++;
				continue;
			}
			
			uint32_t i = 2;
			for(; i < size; ++i ){
				const lit_t clit = CL_GET_LIT( cl, i );
				
				if( assi_is_undef( search.assi, var(clit) ) ){
					CL_SWAP_LITS( cl, literal_position, i );
					
					watch_csp(clist[ watchind ], clit );
					gap ++;
					break;
				}
			}
			if( i == size ){
				
				uint32_t par = 0;
				for(uint32_t j = 1; j < size; ++j ){
					if( assi_is_sat( search.assi, CL_GET_LIT(cl,j) ) ) par = par ^ 1;
				}
				
				if( 0 == (par ^ (size & 1)) ) other_watched_literal = inv_lit(other_watched_literal);
				
				
				
				
				if( enqueue_literal(search, other_watched_literal, reasonStruct(clause) ) == false ){
					if( gap != 0 ){
						for( uint32_t ti = non_moved_cls ; ti + gap < VEC_SIZE( CL_REF, clist); ++ti ){
							clist[ti] = clist[ti + gap];
						}
						VEC_RESIZE( CL_REF, clist, VEC_SIZE( CL_REF, clist) - gap, CL_REF() );
					}
					return conflictStruct(clist[non_moved_cls]);	
				} else {
					clist[ non_moved_cls ] = clist[ watchind ];
					non_moved_cls ++;
				}
			}
		} else if (cl.is1ooN() ){
			
			cerr << "c IMPLEMENT propagation for 1ooN!!!" << endl;
		}
	}
	
	VEC_RESIZE( CL_REF, clist, VEC_SIZE( CL_REF, clist) - gap, CL_REF() );
	return conflictStruct();
}

void CspPropagation::reinit(searchData& search, VEC_TYPE( CL_REF )& clause_set, VEC_TYPE(CL_REF)& restart_learnts, uint32_t var_cnt )
{
	
	for( var_t var1 = 1; var1 < var_cnt+1; var1++ ){
		
		VEC_CLEAR( watch_t, watch[ index( lit(var1,POS) ) ] );
		VEC_CLEAR( watch_t, watch[ index( lit(var1,NEG) ) ] );
		
		VEC_CLEAR( dual_watch_t, implication[ index( lit(var1,NEG) ) ] );
		VEC_CLEAR( dual_watch_t, implication[ index( lit(var1,POS) ) ] );
		if( doCsp) {
			
			for( uint32_t i = 0 ; i < VEC_SIZE(CL_REF,csp[var1]); ++i ){
				CL_DESTROY( gsa.get(csp[var1][i] ) );
				gsa.release( csp[var1][i] );
			}
			VEC_CLEAR( CL_REF, csp[ var1 ] );
		}
	}

	
	if( doCsp ){
		
		setupXor( clause_set, restart_learnts );
		
	}

	for( uint32_t round = 0; round < 2; ++ round ){
		VEC_TYPE( CL_REF )& clauses = ( (reinitFormulaFirst && round == 0) || (!reinitFormulaFirst && round == 1)  ) ? clause_set : restart_learnts;
	
		for(uint32_t i = 0; i< VEC_SIZE( CL_REF,  clauses ); ++i )
		{
			CLAUSE& cl = gsa.get( clauses[i] );
			
			if( doCsp ){
				if( CL_IS_IGNORED(cl) ){
					CL_UNIGNORE(cl);
					continue;
				}
			}
			if( CL_SIZE( cl ) > max_size && max_size != 0 ) continue;
			if( CL_SIZE( cl ) == 2 ){
				watch_dual( search, clauses[i] );
			} else {
				
				if( CL_SIZE( cl ) == 1 ){
					
					lit_t literal = CL_GET_LIT( cl, 0 );
					
					if( assi_variable_equal( search.assi, var(literal), pol( literal ) ) )
						continue;
					
					enqueue_literal( search, literal, reasonStruct() );
					
					search.VAR_LEVEL( var(literal) ) = 0;
				} else {
					
					watch_t watchMe;
					watchMe.clause = clauses[i];
	#ifdef BLOCKING_LIT
					watchMe.lit1 = CL_GET_LIT( cl, 0 );
	#endif
					watch_clause(search, watchMe, CL_GET_LIT( cl, 0) );
					watch_clause(search, watchMe, CL_GET_LIT( cl, 1) );
				}
			}
		}
	}
}

conflictStruct CspPropagation::propagate_literal(searchData& search, lit_t literal)
{
	#ifdef PROPAGATION_METRICS
		calls_propagate_lit1++;
	#endif
	
	VEC_TYPE(watch_t)& current_watchlist =  watch[ index( literal ) ];
#ifdef PREFETCHINGMETHOD1
	
	for( uint32_t i = 0 ; i<VEC_SIZE(watch_t, current_watchlist ); ++i ){
	#ifdef BLOCKING_LIT
			if( !assi_is_sat( search.assi, current_watchlist[i].lit1) ) prefetch( gsa.addr(current_watchlist[i].clause) );
	#else
			prefetch( gsa.addr(current_watchlist[i].clause) );
	#endif
	}
#endif
	
	uint32_t non_moved_cls = 0;
	uint32_t gap = 0;

#ifdef PREFETCHINGMETHOD3
	uint32_t pMax = VEC_SIZE(watch_t, current_watchlist ) < 4 ? VEC_SIZE(watch_t, current_watchlist ) : 4;
	for( uint32_t i = 0 ; i<pMax; ++i ){
	#ifdef BLOCKING_LIT
			if( !assi_is_sat( search.assi, current_watchlist[i].lit1) ) prefetch( gsa.addr(current_watchlist[i].clause) );
	#else
			prefetch( gsa.addr(current_watchlist[i].clause) );
	#endif
	}
#endif

	const uint32_t lsize = VEC_SIZE(watch_t, current_watchlist );
	for( uint32_t watchind = 0 ; watchind < lsize; ++ watchind )
	{

#ifdef PREFETCHINGMETHOD3
		if( watchind + 4 < lsize ){
		#ifdef BLOCKING_LIT
				if( !assi_is_sat( search.assi, current_watchlist[ watchind + 4].lit1) ) prefetch( gsa.addr(current_watchlist[ watchind + 4].clause) );
		#else
				prefetch( gsa.addr(current_watchlist[ watchind + 4].clause) );
		#endif
		}
#endif
	
		MON_EVENT( "WLlen", &lsize );
#ifdef BLOCKING_LIT
		
		if( assi_variable_equal( search.assi, var( current_watchlist[ watchind ].lit1 ), pol( current_watchlist[ watchind ].lit1 ) ) ){
			current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
			non_moved_cls++;
			MON_EVENT( "blockedSat", (void*)1 );
			continue;
		} else {
			MON_EVENT( "blockedSat", (void*)0 );
		}
#endif
			
		const CL_REF clause = current_watchlist[ watchind ].clause;	
		CLAUSE& cl = gsa.get(clause);
	#ifdef PROPAGATION_METRICS
		processed_clauses++;
		clauses_size_sum += CL_SIZE(cl);
	#endif
		
		const uint32_t size = CL_SIZE( cl );
		MON_EVENT( "propClLen", &size );
		
		
		lit_t other_watched_literal;
		uint32_t literal_position=0;	
		
		
		STAT_ACCCLAUSE( clause );
		
		
		if( CL_GET_LIT( cl, 0) == inv_lit( literal ) )
		{
			other_watched_literal = CL_GET_LIT( cl, 1);
			if(  swap_watches )	
			{
				CL_SWAP_LITS( cl, 0, 1);
				literal_position = 1;
			}
		} else {
			other_watched_literal = CL_GET_LIT( cl, 0);
			literal_position = 1;
		}		
		
		if( assi_variable_equal( search.assi, var( other_watched_literal ), pol( other_watched_literal ) ) ){
			current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
			non_moved_cls++;
			MON_EVENT( "watchedSat", (void*)1 );
			continue;
		} else {
			MON_EVENT( "watchedSat", (void*)0 );
		}

		
		uint32_t i = 2;
		bool foundSat = false;
		if( search_sat > 0){
			
			for(; i < size; ++i ){
				const lit_t current_literal = CL_GET_LIT( cl, i );
			#ifdef PROPAGATION_METRICS
				processed_literals++;
			#endif
				
				if( assi_is_sat( search.assi, current_literal ) ){
	#ifndef KEEP_SAT_IN_WATCH
					CL_SWAP_LITS( cl, literal_position, i );
					
					watch_clause(search, current_watchlist[ watchind ], current_literal );
					gap ++;
	#else
					if( assi_is_sat( search.assi,current_literal ) ){
						current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
						non_moved_cls ++;
					} else {
						CL_SWAP_LITS( cl, literal_position, i );
						
						watch_clause(search, current_watchlist[ watchind ], current_literal );
						gap ++;
					}
	#endif			
					foundSat = true;	
					break;	
				}
			}
		}
		
		if( ! foundSat ){
			i = 2;
			
			for(; i < size; ++i ){
				const lit_t current_literal = CL_GET_LIT( cl, i );
			#ifdef PROPAGATION_METRICS
				processed_literals++;
			#endif
				
				if( assi_is_sat( search.assi, current_literal) || assi_is_undef( search.assi, var(current_literal)) ){
	#ifndef KEEP_SAT_IN_WATCH
					CL_SWAP_LITS( cl, literal_position, i );
					
					watch_clause(search, current_watchlist[ watchind ], current_literal );
					gap ++;
					break;	
	#else
					if( assi_is_sat( search.assi, current_literal ) ){
						current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
						non_moved_cls ++;
					} else {
						CL_SWAP_LITS( cl, literal_position, i );
						
						watch_clause(search, current_watchlist[ watchind ], current_literal );
						gap ++;
					}
					break;	
	#endif				

				}
			}
		}
		
		
		if( i == size ){
			
			if( enqueue_literal(search, other_watched_literal, reasonStruct(clause) ) == false ){
				if( gap != 0 ){
					MON_EVENT( "breakWL", &i );
					for( uint32_t ti = non_moved_cls ; ti + gap < VEC_SIZE( CL_REF, current_watchlist); ++ti ){
						current_watchlist[ti] = current_watchlist[ti + gap];
					}
					VEC_RESIZE( CL_REF, current_watchlist, VEC_SIZE( CL_REF, current_watchlist) - gap, watch_t() );
				}
				return conflictStruct(current_watchlist[non_moved_cls].clause);	
			} else {
				current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
				non_moved_cls ++;
			}
		}

	}
	
	
	VEC_RESIZE( CL_REF, current_watchlist, non_moved_cls, watch_t() );

	return conflictStruct();
}


void CspPropagation::sortClauseLevel(searchData& search, CLAUSE& cl, uint32_t start, uint32_t end) const {
	const uint32_t size = CL_SIZE(cl);
	for (uint32_t i = start; i < end; ++i ){
		uint32_t p = i;
		int32_t lev = search.VAR_LEVEL( var( CL_GET_LIT(cl,i) ) );
		for( uint32_t j = i+1; j < size; ++j ){
			const lit_t l = CL_GET_LIT(cl,j);


			if( search.VAR_LEVEL( var(l) ) > lev ){ lev = search.VAR_LEVEL( var(l) ); p = j; }
		}
		CL_SWAP_LITS( cl, i,p );
	}
}

void CspPropagation::setupXor( VEC_TYPE( CL_REF )& cls1, VEC_TYPE( CL_REF )& cls2 ){
	cerr << "c start find xor" << endl;
	
	VEC_TYPE (CL_REF) table;

	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF,cls1); ++i) {
		const CL_REF c = cls1[i];
		CLAUSE& cl = gsa.get(c);
		if( CL_SIZE(cl) < minXorSize ) continue;
		
		VEC_PUSH_BACK( CL_REF, table, c );
		const uint32_t size = CL_SIZE(cl);
		
		for( uint32_t j = 1; j<size; ++j ){
			if( CL_GET_LIT(cl,j-1) > CL_GET_LIT(cl,j) ){
				CL_SORT( cl );
				break;
			}
		}
	}
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF,cls2); ++i) {
		const CL_REF c = cls2[i];
		CLAUSE& cl = gsa.get(c);
		if( CL_SIZE(cl) < 3 ) continue;
		
		VEC_PUSH_BACK( CL_REF, table, c );
		const uint32_t size = CL_SIZE(cl);
		
		for( uint32_t j = 1; j<size; ++j ){
			if( CL_GET_LIT(cl,j-1) > CL_GET_LIT(cl,j) ){
				CL_SORT( cl );
				break;
			}
		}
	}
	
	cerr << "c relevant clauses: " << VEC_SIZE(CL_REF,table) << endl;
	uint32_t xors = 0;
	
	uint32_t cL = 3;	
	uint32_t cP = 0;	
	uint32_t xorClauses=0;	
	
	sort(table);
	
	
	while( cP < VEC_SIZE (CL_REF, table) && cL < 63){
		uint32_t start = cP;
		cL = CL_SIZE( gsa.get( table[cP] ) );
		
		while( cP < VEC_SIZE(CL_REF,table) && CL_SIZE( gsa.get( table[cP] ) ) == cL ) cP ++;
	
		
		for (uint32_t i = start; i < cP-1; ++i) {
			const CL_REF c = table[i];
			const CLAUSE& cl = gsa.get(c);
			uint32_t stop = i + 1;
			
			for ( ; stop < cP; ++stop) {
				const CL_REF c2 = table[stop];
				const CLAUSE& cl2 = gsa.get(c2);
				uint32_t k = 0;
				for( ; k < CL_SIZE(cl2); k ++ ){
					if( var(CL_GET_LIT(cl,k)) != var(CL_GET_LIT(cl2,k)) ) break;
				}
				if( k != CL_SIZE(cl2) ) break;
			}
			
			
			uint64_t shift = 1;
			shift = shift << ( CL_SIZE(cl) -1 );

			
			if( stop - start >= shift ){
				
				
				uint32_t odd = 0;
				uint32_t count = 0;
				
				for( uint32_t j = 0 ; j < CL_SIZE(cl); ++j){
					if( pol( CL_GET_LIT(cl,j) ) == NEG ) odd = odd ^ 1;
				}
				
				for( uint32_t j = i ; j < stop; ++j){
					const CL_REF c2 = table[j];
					const CLAUSE& cl2 = gsa.get(c2);
					uint32_t o = 0;
					for( uint32_t k = 0; k < CL_SIZE(cl2); k ++ )
						if( pol( CL_GET_LIT(cl2,k) ) == NEG ) o = o ^ 1;
					if( o == odd ) count ++;
				}
				if( count == shift ){
	
					CLAUSE xcl = cl;
					xcl.setXor(true);
					
					if( ( (CL_SIZE(xcl)&1) ) == 0 ){	
						CL_SET_LIT(xcl,0, inv_lit( CL_GET_LIT(xcl,0) ) );
					}
					CL_REF xc = gsa.create( xcl );
	
					
					CL_DESTROY(xcl);
					xors ++;
					xorClauses += count;
				
					
					watch_csp(xc, CL_GET_LIT(xcl,0));
					watch_csp(xc, CL_GET_LIT(xcl,1));
				
					
					for( uint32_t j = i ; j < stop; ++j){
						CLAUSE& cl2 = gsa.get(table[j]);
						uint32_t o = 0;
						for( uint32_t k = 0; k < CL_SIZE(cl2); k ++ )
							if( pol( CL_GET_LIT(cl2,k) ) == NEG ) o = o ^ 1;
						if( o == odd ) CL_IGNORE(cl2);
					}
				}
				
				start = stop;
			}
		}
		
		
		cL ++;
	}

	cerr << "c found " << xors << " non-binary xors encoded with " << xorClauses << " clauses" << endl;
}

void CspPropagation::setup1ooN( VEC_TYPE( CL_REF )& cl1, VEC_TYPE( CL_REF )& cl2 ){

}


void CspPropagation::sort(VEC_TYPE( CL_REF )& clauses){

	uint32_t n = VEC_SIZE(CL_REF,clauses);
	int32_t m, s;
	
	CL_REF* a=new CL_REF[ n ];
	for( uint32_t i = 0 ; i < n; i++ ){
		a[i] = clauses[i];
	}
	CL_REF* original = a;	
	CL_REF* b=new CL_REF[ n ];

	
	for (s=1; s<n; s+=s)
	{
		m = n;
		do {
			m = m - 2*s;	
			int32_t hi = (m+s > 0) ? m + s : 0;	
			
			int32_t i = (m > 0) ? m : 0;	
			int32_t j = hi;
			
			int32_t stopb = m + 2*s;	
			int32_t currentb = i;			
			
			
			while( i < hi && j < stopb)
			{
				if( ( gsa.get(a[i]) ) < ( gsa.get(a[j]))  )
					b[currentb++] = a[i++];
				else
					b[currentb++] = a[j++];
			}
			
			for( ; i < hi; )
				b[currentb++] = a[i++];
				
			for( ; j< stopb; 	)
				b[currentb++] = a[j++];
				
		} while( m > 0 );
		
		
		CL_REF* tmp = a;
		a = b;
		b = tmp;
	}
	
	delete [] b;
	
	for( uint32_t i = 0 ; i < n; i++ ){
		clauses[i] = a[i];
	}
	return;
}
void CspPropagation::set_parameter( const StringMap& commandline )
{
	
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"up_swap" ) )  swap_watches = 0 != atoi( commandline.get( (const char*)"up_swap" ).c_str() );
	if( commandline.contains( (const char*)"up_prop_order" ) )  keep_order_prop = 0 != atoi( commandline.get( (const char*)"up_prop_order" ).c_str() );
	if( commandline.contains( (const char*)"up_remove_order" ) )  keep_order_remo = 0 != atoi( commandline.get( (const char*)"up_remove_order" ).c_str() );
	if( commandline.contains( (const char*)"up_bfs" ) )  bfs = 0 != atoi( commandline.get( (const char*)"up_bfs" ).c_str() );
	if( commandline.contains( (const char*)"dup_closure" ) )  implication_closure = 0 != atoi( commandline.get( (const char*)"dup_closure" ).c_str() );
	if( commandline.contains( (const char*)"dup_long_conflicts" ) )  long_conflicts = 0 != atoi( commandline.get( (const char*)"dup_long_conflicts" ).c_str() );
	if( commandline.contains( (const char*)"up_pref_items" ) ) prefetched_items = atoi( commandline.get( (const char*)"up_pref_items" ).c_str() );
	if( commandline.contains( (const char*)"up_last_phase" ) ) lastPhase = atoi( commandline.get( (const char*)"up_last_phase" ).c_str() );
	if( commandline.contains( (const char*)"up_search_sat" ) ) search_sat = atoi( commandline.get( (const char*)"up_search_sat" ).c_str() );
	if( commandline.contains( (const char*)"up_csp" ) ) doCsp = 0 != atoi( commandline.get( (const char*)"up_csp" ).c_str() );
	if( commandline.contains( (const char*)"up_init_order" ) )  reinitFormulaFirst = 0 != atoi( commandline.get( (const char*)"up_init_order" ).c_str() );
	if( commandline.contains( (const char*)"up_minXorSize" ) ) minXorSize = atoi( commandline.get( (const char*)"up_minXorSize" ).c_str() );
	if( minXorSize < 3 ) minXorSize = 3;
#endif
	
#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== dual propagation information ===" << std::endl;
		std::cerr << " parameter           pos.value  info" << std::endl;
		std::cerr << " up_swap             0,1        current watch is on second position in clause after propagation"  << std::endl;
		std::cerr << " up_prop_order       0,1        keep order, if clause is watched on another literal" << std::endl;
		std::cerr << " up_remove_order     0,1        keep order, if clause is removed due to removal heuristic" << std::endl;
		std::cerr << " up_bfs              0,1        propagate implications in bfs or dfs manner" << std::endl;
		std::cerr << " up_pref_items       0-n        number of literals, whose watched clauses are prefetched" << std::endl;
		std::cerr << " up_search_sat       0,1        always look for a satisfied literal to watch" << std::endl;
		std::cerr << " up_last_phase       0,1        enable phase saving for last trail level" << std::endl;
		std::cerr << " up_init_order       0,1        0 = formula first, 1 = learned first" << std::endl;
		std::cerr << " up_csp              0,1        try to extract xor and 1ooN for special propagation" << std::endl;
		std::cerr << " up_minXorSize       3,n        minimal size s.t. xors are represented in a special way" << std::endl;
		std::cerr << " dup_closure         0,1        propagate implication closure of dual clauses" << std::endl;
		std::cerr << " dup_long_conflicts  0,1        continue prop. after dual conflict to find a long one" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
}

#ifdef COMPILE_LIBRARY

extern "C" CspPropagation* create_unitpropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt,  const uint32_t max_size, const StringMap& commandline){
	return new CspPropagation( search, clause_set, var_cnt, max_size, commandline );
}

extern "C" void destroy_unitpropagation( CspPropagation* propagation ){
	delete propagation;
}
#endif
