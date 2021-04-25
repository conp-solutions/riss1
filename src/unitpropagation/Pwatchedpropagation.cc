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


#include "unitpropagation/Pwatchedpropagation.h"


#include <iostream>

PWatchedPropagation::PWatchedPropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, const uint32_t max_size, const StringMap& commandline)
 : threads(1),
	jobqueue( 0 ),
	wLocker(1), 
	cLocker(1), 
	swap_watches(true),
	keep_order_prop(true),
	keep_order_remo(true),
	bfs(true),
	foreward(true),
	search_sat(0),
	reinitFormulaFirst(true),
	debug(0)
{

	add_here = 0;
	debug = 0;

	
	set_parameter( commandline );
	
	
	jobqueue.init(threads - 1);
	jobqueue.setState( 0 );	

	
	watch = new VEC_TYPE( VEC_TYPE(watch_t) )[threads];
	for( uint32_t t = 0 ; t < threads; t++ ){
		VEC_CREATE_SPACE( VEC_TYPE(watch_t), watch[t], max_index( search.var_cnt ) );	
	}
	unit_ind_thread = new uint32_t[ threads * threads];	
	for( uint32_t i = 0; i < threads * threads; ++i ) unit_ind_thread[i] = 0;

	
	propTasks = new propStruct[threads];
	sds = new searchData[threads];
	for( uint32_t i = 1; i < threads; ++i ){
		sds[i-1].init( search );
	}

	
	unit_queue = new lit_t*[threads];
	queue_size = new cacheLine[threads];
	for( uint32_t i = 0; i < threads; i ++ ){
		unit_queue[i] = (lit_t*) malloc( sizeof(lit_t) * search.var_cnt+1 );
		queue_size[i].data = 0;
	}
	
	if( clause_set != 0 ) 
	{
		uint32_t avglen = VEC_SIZE( CL_REF, (*clause_set) ) / search.var_cnt + 1;
		for( uint32_t i = 0; i< max_index( search.var_cnt ); ++i ){
			VEC_TYPE( watch_t ) tmp;
			
			for( uint32_t t = 0 ; t < threads; t++ ){
				VEC_CREATE_SPACE( watch_t, tmp, avglen + 4 );
				VEC_PUSH_BACK(VEC_TYPE(watch_t),  watch[t], tmp );
			}
		}
		
		VEC_TYPE( CL_REF ) tmp;
		VEC_CREATE( CL_REF, tmp );
		reinit(search, (*clause_set), tmp, search.var_cnt );
		VEC_DESTROY( CL_REF, tmp );
	}
}

PWatchedPropagation::~PWatchedPropagation()
{
	for( uint32_t t = 0 ; t < threads; ++t ){
		for(uint32_t i = 0; i<VEC_SIZE(VEC_TYPE(watch_t),  watch[t] ); ++i ){
			VEC_DESTROY( watch_t,  watch[t][i] );
		}
		VEC_DESTROY(VEC_TYPE(watch_t),  watch[t] );
	}
	
	for( uint32_t i = 0; i < threads; i++ ) VEC_DESTROY(lit_t,  unit_queue[i] );
	delete [] unit_queue;
	
	delete [] propTasks;
	
	delete [] unit_ind_thread;
}

void PWatchedPropagation::enqueue_decision(searchData& search, lit_t decision_literal )
{
	
	for( uint32_t i = 1; i < threads; ++i){
		sds[i-1].current_level = search.current_level;
		enqueue_literal( sds[i-1], decision_literal, reasonStruct(), i );
	}
	
	enqueue_literal( search, decision_literal, reasonStruct(), 0 );
}

void PWatchedPropagation::propagate(searchData& search, VEC_TYPE( conflictStruct  )& conflict_clauses )
{
	uint32_t old_size = 0;
	
	
	for( uint32_t i = 0; i < threads * threads; ++i ){	
		unit_ind_thread[i] = 0;
	}
	
	sharedSD = &search;
	conflictThread = -1;
	globalConflict = conflictStruct();	
	
	
	for( uint32_t t = 0 ; t < threads; t++ ){
		if( t == 0 ) propTasks[t] = propStruct( this, &search, &conflict_clauses, t);	
		else {
			sds[t-1].current_level = search.current_level;
			propTasks[t] = propStruct( this, & (sds[t-1] ), &conflict_clauses, t);	
		}
		JobQueue::Job job;
		job.function = &(PWatchedPropagation::propagate_single);
		job.argument = (void*) &(propTasks[t]);
		
		if( t != 0 ) jobqueue.addJob( job );
	}
	jobqueue.setState( 1 ); 

	
	propagate_single( (void*) &(propTasks[0]) );

	
	for( uint32_t i = 1; i < threads; i++ ){
		while( jobqueue.getThredState(i-1) != 0 ){}
	}
	jobqueue.setState( 0 );	

	
	if( !globalConflict.empty() ){
		VEC_PUSH_BACK( conflictStruct, conflict_clauses, globalConflict );
	}

	
	for( uint32_t i = 0; i < threads; i++ )	queue_size[i].data = 0;
	return;
}

void* PWatchedPropagation::propagate_single(void *argument){	
	propStruct* r = (propStruct*) argument;

	(r->_prop)->propagate_thread( *(r->_search), *(r->_conflict_clauses), r->_threadnr );
	return 0;
}

void PWatchedPropagation::propagate_thread(searchData& search, VEC_TYPE( conflictStruct  )& conflict_clauses, const uint32_t threadnr )
{
	lit_t literal = NO_LIT;						
	conflictStruct conflicted_clause;	
	volatile uint32_t current_ind[threads];
	
	for( uint32_t i=0; i < threads; ++ i ){
		current_ind[i] = unit_ind_thread[threadnr*threads+i];
	}

	volatile bool proceed = true;
	while( proceed ){

		
		while ( current_ind[threadnr] < queue_size[threadnr].data && VEC_SIZE( CL_REF, conflict_clauses ) == 0 ){
			
			const uint32_t myIndex = current_ind[threadnr];
			literal = unit_queue[threadnr][myIndex];
			
			
			conflicted_clause = propagate_literal(search, literal, threadnr);
			if ( !conflicted_clause.empty() ) {
				
				cLocker.wait();
				
				
				if( globalConflict.empty() ){
					globalConflict = conflicted_clause;
					conflictThread = threadnr;
				}
				cLocker.unlock();
				break;
			}
			current_ind[threadnr] ++; 
		}
		
		
		if( ! globalConflict.empty() ) break;
		
		
		proceed = true;
		
		for( uint32_t i = 0; i < threads; ++ i ){
			
			if( i == threadnr ) continue;
			
			
			if( current_ind[i] < queue_size[i].data ){
				uint32_t size = queue_size[i].data;
				
				if( i == 0 ){
					for( ; current_ind[i] < size; ++ current_ind[i] ){
						
						const uint32_t myIndex = current_ind[i];
						if(!enqueue_literal( search, unit_queue[i][ myIndex ], sharedSD->VAR_REASON(var(unit_queue[i][ myIndex ])), threadnr )){
							cLocker.wait();
							
							const reasonStruct rs = sharedSD->VAR_REASON(var(unit_queue[i][ myIndex ]));
							if( globalConflict.empty() ){
								if( rs.isLit() ){
									
									globalConflict = conflictStruct( unit_queue[i][ myIndex ], rs.getLit());
								} else {
									globalConflict = conflictStruct( rs.getCL() );
								}
								conflictThread = threadnr;
							}
							cLocker.unlock();
							break;
						}
					}
					proceed = false;
				} else {
					for( ; current_ind[i] < size; ++ current_ind[i] ){
						
						const uint32_t myIndex = current_ind[i];
						if( !enqueue_literal( search, unit_queue[i][ myIndex ], sds[i-1].VAR_REASON(var(unit_queue[i][ myIndex ])), threadnr ) ){
							cLocker.wait();
							
							const reasonStruct rs = sds[i-1].VAR_REASON(var(unit_queue[i][ myIndex ]));
							if( globalConflict.empty() ){
								if( rs.isLit() ){
									
									globalConflict = conflictStruct( unit_queue[i][ myIndex ], rs.getLit());
								} else {
									globalConflict = conflictStruct( rs.getCL() );
								}
								conflictThread = threadnr;
							}
							cLocker.unlock();
							break;
						}
					}
				}
				proceed = false;
			}
		}
		
		
		if( !globalConflict.empty() ) break;
		
		
		if( !proceed ){
			proceed = true;
			continue;
		}

		
		
		
		
		proceed = true;
		
		volatile int32_t sum = 0;
		for( uint32_t i = 0; i < threads; ++ i ){
			
			for( uint32_t j = 0; j < threads; ++j ){
				sum += unit_ind_thread[threads*i + j];
			}
		}
		for( uint32_t i = 0; i < threads; ++ i ){
			sum -= queue_size[i].data * threads;
		}
		for( uint32_t i = 1; i < threads; ++ i ){
			if( queue_size[i].data != queue_size[i-1].data ) sum = 1;
		}
		if( sum == 0 ) proceed = false;
		
		
		
		for( uint32_t i = 0; i < threads; i++ ){
			unit_ind_thread[threadnr*threads + i ] = current_ind[i];
		}
	};
	
	
	if( threadnr == 0  ){
		
		cLocker.wait();
		if( conflictThread != 0 && ! globalConflict.empty() ){
			
			const uint32_t size = queue_size[conflictThread].data;
			assert( conflictThread != 0 );
			const uint32_t cThread = conflictThread;
		
			for( ; current_ind[cThread] < size; ++ current_ind[cThread] ){
				
				const uint32_t myIndex = current_ind[cThread];
				const lit_t othersLit = unit_queue[cThread][ myIndex ];
				const reasonStruct othersR = sds[cThread-1].VAR_REASON( var(othersLit) );
				const int32_t level = sds[cThread-1].VAR_LEVEL( var(othersLit) );

				
				if(!enqueue_literal( search, unit_queue[cThread][ myIndex ], sds[cThread-1].VAR_REASON(var(othersLit)), threadnr )){
					VEC_CLEAR( conflictStruct, conflict_clauses );
					
					
					
					if( sds[cThread-1].VAR_REASON(var(othersLit)).isLit() ){
						globalConflict = conflictStruct( othersLit, sds[cThread-1].VAR_REASON(var(othersLit)).getLit() );
					} else {
						globalConflict = conflictStruct( sds[cThread-1].VAR_REASON(var(othersLit)).getCL() );
					}
					conflictThread = threadnr;
					break;
				}
			}
			
			for( uint32_t i = 0; i < threads; i++ ){
				unit_ind_thread[threadnr*threads + i ] = current_ind[i];
			}
			
		}
		cLocker.unlock();
	}
	return;
}

bool PWatchedPropagation::clauses_added(searchData& search, VEC_TYPE(CL_REF)& clauses){
	bool fail = true;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i ){
		fail = fail && clause_added( search, clauses[i] );
	}
	return fail;
}


bool PWatchedPropagation::clause_added(searchData& search, CL_REF clause){
	assert( clause != 0 );
	assert( CL_SIZE( gsa.get(clause) )> 0 );
	
	bool not_unsatisfied = true;
	
	
	CLAUSE& cl = gsa.get(clause);
	
	if( CL_SIZE(cl) == 1 ){
		lit_t literal = CL_GET_LIT( cl, 0 );
		
		bool ret = true;
		
		for( uint32_t i = 1; i < threads; ++i) ret = ret && enqueue_literal( sds[i-1], literal, reasonStruct(clause), i  );
		
		ret = ret && enqueue_literal( search, literal, reasonStruct(clause), 0 );
		
		
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
			
			for( uint32_t i = 1; i < threads; ++i) not_unsatisfied = not_unsatisfied && enqueue_literal( sds[i-1], CL_GET_LIT( cl, 0 ) , reasonStruct(clause), i  );
			
			not_unsatisfied = not_unsatisfied && enqueue_literal( search, CL_GET_LIT( cl, 0 ) , reasonStruct(clause), 0 );
		}
	} else {
		if( watched_literals == 0 ){
			
			not_unsatisfied = false;
			cerr << "c added unsatisfied clause" << endl;
		}
	}
	
	uint32_t addIndex = addClauseInTask(  clause );
	
	watch_t watchMe;
#ifdef BLOCKING_LIT
	watchMe.lit1 = CL_GET_LIT( cl, 0 );
#endif

#ifdef USE_IMPLICIT_BINARY
	if( CL_SIZE(cl) > 2 ){
#endif
		watchMe.clause = reasonStruct(clause);
		watch_clause(search, watchMe, CL_GET_LIT(cl, 0), addIndex );
		watch_clause(search, watchMe, CL_GET_LIT(cl, 1), addIndex );
#ifdef USE_IMPLICIT_BINARY
	} else {
		
		watchMe.clause = reasonStruct(CL_GET_LIT(cl, 1), true);
		watch_clause(search, watchMe, CL_GET_LIT(cl, 0),addIndex );
		watchMe.clause = reasonStruct(CL_GET_LIT(cl, 0),true);
		watch_clause(search, watchMe, CL_GET_LIT(cl, 1),addIndex );
	}
#endif
	
	
	return not_unsatisfied;
}

struct removeStruct{
	PWatchedPropagation* _prop;
	searchData* _search;
	VEC_TYPE(CL_REF)* _clauses;
	int _threadnr;
	removeStruct( PWatchedPropagation* prop,searchData* search, VEC_TYPE(CL_REF)* clauses, int threadnr ) : _prop(prop), _search( search), _clauses(clauses), _threadnr(threadnr)
	{}
	removeStruct() : _prop(0), _search(0), _clauses(0), _threadnr(0) {}
};

void PWatchedPropagation::clauses_removed(searchData& search, VEC_TYPE(CL_REF)& clauses )
{	
	removeStruct r[threads];
	for( uint32_t t = 0 ; t < threads; t++ ){
		if( t==0 ) r[t] = removeStruct( this, &search, &clauses, t);
		else  r[t] = removeStruct( this, &(sds[t-1]), &clauses, t);
		JobQueue::Job job;
		job.function = &(PWatchedPropagation::clauses_remove_single);
		job.argument = (void*) &(r[t]);
		if( t != 0 ) jobqueue.addJob( job );
	}
	jobqueue.setState( 1 ); 
	
	
	clauses_remove_single( (void*) &(r[0]) );
	
	
	for( uint32_t i = 1; i < threads; i++ ){
		while( jobqueue.getThredState(i-1) != 0 ){}
	}
	jobqueue.setState( 0 ); 
	
}

void* PWatchedPropagation::clauses_remove_single(void *argument){
	removeStruct* r = (removeStruct*) argument;
	(r->_prop)->clauses_removed_thread( *(r->_search), *(r->_clauses), r->_threadnr );
	return 0;
}

void PWatchedPropagation::clauses_removed_thread(searchData& search, VEC_TYPE(CL_REF)& clauses, uint32_t threadnr ){
	for( uint32_t cind = 0; cind < VEC_SIZE( CL_REF, clauses ); ++cind ){
		CLAUSE& cl = gsa.get(clauses[cind]);
		if( CL_SIZE(cl) > 2 ){
			for(uint32_t i = 0; i<2; ++i){
				lit_t watched_literal = inv_lit( CL_GET_LIT(cl,i) );
				
				VEC_TYPE(watch_t)& current_watchlist = watch[threadnr][ index( watched_literal ) ];
			
				for(uint32_t ind = 0; ind < VEC_SIZE( CL_REF, current_watchlist ); ind++){
					if( clauses[cind] == current_watchlist[ind].clause.getCL() && !current_watchlist[ind].clause.isLit() ){
						
						if( keep_order_remo ) VEC_ERASE(CL_REF,  current_watchlist, ind );
						else VEC_ERASE_NO_ORDER( CL_REF,  current_watchlist, ind );
						break;
					}
				}
			}
		} else {
			
			const lit_t l1 = CL_GET_LIT(cl,0);
			const lit_t l2 = CL_GET_LIT(cl,1);
			
			VEC_TYPE(watch_t)& wl1 = watch[threadnr][ index( inv_lit(l1) ) ];
			for(uint32_t ind = 0; ind < VEC_SIZE( CL_REF, wl1 ); ind++){
				if( l2 == wl1[ind].clause.getLit() ){
					
					if( keep_order_remo ) VEC_ERASE(CL_REF,  wl1, ind );
					else VEC_ERASE_NO_ORDER( CL_REF,  wl1, ind );
					break;
				}
			}
			VEC_TYPE(watch_t)& wl2 = watch[threadnr][ index( inv_lit(l1) ) ];
			for(uint32_t ind = 0; ind < VEC_SIZE( CL_REF, wl2 ); ind++){
				if( l2 == wl2[ind].clause.getLit() ){
					
					if( keep_order_remo ) VEC_ERASE(CL_REF,  wl2, ind );
					else VEC_ERASE_NO_ORDER( CL_REF,  wl2, ind );
					break;
				}
			}
		}
	}

}

struct backStruct{
	PWatchedPropagation* _prop;
	searchData* _search;
	int32_t level;
	int _threadnr;
	backStruct( PWatchedPropagation* prop,searchData* search, int32_t l, int threadnr ) : _prop(prop), _search( search), level(l), _threadnr(threadnr)
	{}
	backStruct() : _prop(0), _search(0), level(0), _threadnr(0) {}
};

void PWatchedPropagation::backtrack(searchData& search, int32_t level , VEC_TYPE(var_t)& undefed_literals)
{	
	
	uint32_t size = STACK_SIZE(lit_t,  search.trail );
	if( debug > 2 ){
		cerr << "c PRE trail sizes: " << size;
		for( uint32_t t = 1; t < threads; t++ ){
			 cerr << "." << STACK_SIZE(lit_t,  sds[t-1].trail );
		}
		cerr << "." << endl;
	}

	backStruct r[threads];

	for( uint32_t t = 1 ; t < threads; t++ ){
		r[t] = backStruct( this, &(sds[t-1]), level, t);

		JobQueue::Job job;
		job.function = &(PWatchedPropagation::back_single);
		job.argument = (void*) &(r[t]);
		jobqueue.addJob( job );
	}
	jobqueue.setState( 1 ); 
	
	
	search.current_level = level;
	
	queue_size[0].data = 0;

	
	while( STACK_SIZE(lit_t,  search.trail ) > 0 && search.VAR_LEVEL( var( STACK_TOP(lit_t, search.trail) ) ) > level ){
		
		const var_t& variable = var( STACK_TOP(lit_t, search.trail) );
		search.VAR_REASON( variable ) = reasonStruct();
		search.VAR_LEVEL( variable ) = -1;
		assi_undef_variable_backup( search.assi, variable );	

		
		VEC_PUSH_BACK( lit_t, undefed_literals, variable );

		
		STACK_POP(lit_t,  search.trail );	
	}
	
	
	for( uint32_t i = 1; i < threads; i++ ){
		while( jobqueue.getThredState(i-1) != 0 ){}
	}
	
	jobqueue.setState( 0 ); 
	
	
	if( debug > 2 ){
		size = STACK_SIZE(lit_t,  search.trail );
		cerr << "c POST trail sizes: " << size;
		for( uint32_t t = 1; t < threads; t++ ){
			 cerr << "." << STACK_SIZE(lit_t,  sds[t-1].trail );
		}
		cerr << "." << endl;
	}
}

void* PWatchedPropagation::back_single(void *argument){	
	backStruct* r = (backStruct*) argument;
	
	(r->_prop)->back_thread( *(r->_search), r->level, r->_threadnr );
	return 0;
}


void PWatchedPropagation::back_thread( searchData& search, int32_t level, uint32_t threadnr ){
	search.current_level = level;
	
	queue_size[threadnr].data = 0;
	
	
	while( STACK_SIZE(lit_t,  search.trail ) > 0 && search.VAR_LEVEL( var( STACK_TOP(lit_t, search.trail) ) ) > level ){
		
		const var_t& variable = var( STACK_TOP(lit_t, search.trail) );
		search.VAR_REASON( variable ) = reasonStruct();
		search.VAR_LEVEL( variable ) = -1;
		assi_undef_variable_backup( search.assi, variable );	

		
		STACK_POP(lit_t,  search.trail );	
	}
}



void PWatchedPropagation::watch_clause(searchData& search, watch_t clause, lit_t watched_literal, uint32_t threadnr )
{
	VEC_PUSH_BACK( watch_t, watch[threadnr][ index( inv_lit(watched_literal) ) ], clause);
}


bool PWatchedPropagation::enqueue_literal(searchData& search, lit_t literal, const reasonStruct& reason, const uint32_t threadnr){
	
	if( assi_variable_equal( search.assi, var(literal), pol( inv_lit(literal) ) ) ){
	
		if( debug > 0 ){
			
			if( ! reason.isLit() ){
				CLAUSE& cl = gsa.get( reason.getCL() );
				uint32_t i = 0;
				for(  ; i < CL_SIZE(cl); ++i ){
					if( !assi_is_unsat( search.assi, CL_GET_LIT(cl,i)) ) break;
				}		
				if( i != CL_SIZE(cl) ){
					stringstream st;
					st << "c enqueue failed for literal " << nr(literal) << " at level " << search.current_level << " for thread " << threadnr << " with reason " << reason.getCL() << endl; 
					cerr << st.str();
				}
			}
		}
		return false;
	}
	if( assi_variable_equal( search.assi, var(literal), pol( literal ) ) ){
		return true;
	}


	
	
	
	

	
		
	
	assi_set_polarity_backup( search.assi, var(literal), pol( literal ) );
		
	search.VAR_REASON( var(literal) ) = reason;	
	search.VAR_LEVEL( var(literal) ) = search.current_level;
	
	
	unit_queue[threadnr][ queue_size[threadnr].data ] = literal;
	
	#ifdef USE_MEM_CLOB
	
	asm volatile ( "" ::: "memory");	
	#endif
	
	#ifdef USE_SFENCE
	
	asm volatile ( "sfence" );
	#endif
	
	
	
	
	queue_size[threadnr].data ++;

	
	STACK_PUSH(lit_t, search.trail, literal);
	return true;
}

conflictStruct PWatchedPropagation::propagate_literal(searchData& search, lit_t literal,  uint32_t threadnr)
{
	
	VEC_TYPE(watch_t)& current_watchlist = watch[threadnr][ index( literal ) ];


#ifdef PREFETCHINGMETHOD1
	
	for( uint32_t i = 0 ; i<VEC_SIZE(watch_t, current_watchlist ); ++i ){
		
		if( !current_watchlist[i].clause.isLit() ) {
	#ifdef BLOCKING_LIT
		
		if( !assi_is_sat( search.assi, current_watchlist[i].lit1) ) prefetch( gsa.addr( current_watchlist[i].clause.getCL() ) );
	#else
		prefetch( gsa.addr(current_watchlist[i].clause.getCL() ) );
	#endif
		}
	}
#endif
	
	uint32_t non_moved_cls = 0;
	uint32_t gap = 0;

#ifdef PREFETCHINGMETHOD3
	uint32_t pMax = VEC_SIZE(watch_t, current_watchlist ) < 4 ? VEC_SIZE(watch_t, current_watchlist ) : 4;
	for( uint32_t i = 0 ; i<pMax; ++i ){
	#ifdef BLOCKING_LIT
			if( !assi_is_sat( search.assi, current_watchlist[i].lit1) && !current_watchlist[i].clause.isLit() ) prefetch( gsa.addr(current_watchlist[i].clause.getCL() ) );
	#else
			if( !current_watchlist[i].clause.isLit() ) prefetch( gsa.addr(current_watchlist[i].clause.getCL() ) );
	#endif
	}
#endif

	const uint32_t lsize = VEC_SIZE(watch_t, current_watchlist );
	for( uint32_t watchind = 0 ; watchind < lsize; ++ watchind )
	{

#ifdef PREFETCHINGMETHOD3
		if( watchind + 4 < lsize ){
	#ifdef BLOCKING_LIT
			if( !assi_is_sat( search.assi, current_watchlist[ watchind + 4].lit1) && !current_watchlist[ watchind + 4].clause.isLit() )
				prefetch( gsa.addr(current_watchlist[ watchind + 4].clause.getCL() ) );
	#else
			if( !current_watchlist[ watchind + 4].clause.isLit() ) 
				prefetch( gsa.addr(current_watchlist[ watchind + 4].clause.getCL() ) );
	#endif
		}
#endif
	
#ifdef BLOCKING_LIT
		
		if( assi_variable_equal( search.assi, var( current_watchlist[ watchind ].lit1 ), pol( current_watchlist[ watchind ].lit1 ) ) ){
			current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
			non_moved_cls++;
			continue;
		}
#endif
		
		if( current_watchlist[ watchind ].clause.isLit() ){
			const lit_t other = current_watchlist[ watchind ].clause.getLit();
			if(! enqueue_literal(search, other, reasonStruct( inv_lit(literal) , true ), threadnr ) ){
			
				if( gap != 0 ){
					for( uint32_t i = non_moved_cls ; i + gap < VEC_SIZE( CL_REF, current_watchlist); ++i ){
						current_watchlist[i] = current_watchlist[i + gap];
					}
					VEC_RESIZE( watch_t, current_watchlist, VEC_SIZE( CL_REF, current_watchlist) - gap, watch_t() );
				}
				return conflictStruct( inv_lit(literal), other );	
			} else {
			
				current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
				non_moved_cls ++;
				
			}
			continue;
		}
		
		
		assert( !current_watchlist[ watchind ].clause.isLit() );
		
		
		const CL_REF clause = current_watchlist[ watchind ].clause.getCL();	
		const uint32_t size = CL_SIZE( gsa.get(clause) );
		
		
		lit_t other_watched_literal;
		uint32_t literal_position=0;	
		
		
		
		if( CL_GET_LIT( gsa.get(clause), 0) == inv_lit( literal ) ){
			other_watched_literal = CL_GET_LIT( gsa.get(clause), 1);
			if(  swap_watches ){	
				CL_SWAP_LITS( gsa.get(clause), 0, 1);
				literal_position = 1;
			}
		} else {
			other_watched_literal = CL_GET_LIT( gsa.get(clause), 0);
			literal_position = 1;
		}		
		
		if( assi_variable_equal( search.assi, var( other_watched_literal ), pol( other_watched_literal ) ) ){
			current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
			non_moved_cls++;
			continue;
		}

		
		uint32_t i = 2;
		
		bool foundSat = false;
		if( search_sat > 0 ){
			
			const uint32_t max = search_sat > size ? size : search_sat;
			for(; i < max; ++i ){
				const lit_t current_literal = CL_GET_LIT( gsa.get(clause), i );
			#ifdef PROPAGATION_METRICS
				processed_literals++;
			#endif
				
				if( assi_is_sat( search.assi, current_literal ) ){
	#ifndef KEEP_SAT_IN_WATCH
					CL_SWAP_LITS( gsa.get(clause), literal_position, i );
					
					watch_clause(search, current_watchlist[ watchind ], current_literal, threadnr );
					gap ++;
	#else
					
					current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
					non_moved_cls ++;
	#endif			
					foundSat = true;	
					break;	
				}
			}
		}
		
		if( ! foundSat ){
			i = 2;
			for(; i < size; ++i ){
				const lit_t current_literal = CL_GET_LIT( gsa.get(clause), i );

				
				if( !assi_is_unsat( search.assi, current_literal ) ){
	#ifndef KEEP_SAT_IN_WATCH
					CL_SWAP_LITS( gsa.get(clause), literal_position, i );
					
					watch_clause(search, current_watchlist[ watchind ], current_literal, threadnr );
					gap ++;
	#else
					if( assi_variable_equal( search.assi, var( current_literal ), pol( current_literal) ) ){
						current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
						non_moved_cls ++;
					} else {
						CL_SWAP_LITS( gsa.get(clause), literal_position, i );
						
						watch_clause(search, current_watchlist[ watchind ], current_literal );
						gap ++;
					}
	#endif				
					break;	
				}
			}
		}
		
		
		if( i == size ){
			assert( clause != 0 );
			
			if( enqueue_literal(search, other_watched_literal, reasonStruct(clause), threadnr ) == false ){
				if( gap != 0 ){
					for( uint32_t i = non_moved_cls ; i + gap < VEC_SIZE( CL_REF, current_watchlist); ++i ){
						current_watchlist[i] = current_watchlist[i + gap];
					}
					VEC_RESIZE( watch_t, current_watchlist, VEC_SIZE( CL_REF, current_watchlist) - gap, watch_t() );
				}
				return conflictStruct(current_watchlist[non_moved_cls].clause.getCL());	
			} else {
				current_watchlist[ non_moved_cls ] = current_watchlist[ watchind ];
				non_moved_cls ++;
			}
		}

	}
	
	
	VEC_RESIZE( watch_t, current_watchlist, non_moved_cls, watch_t() );
	return conflictStruct();
}

void PWatchedPropagation::reinit(searchData& search, VEC_TYPE( CL_REF )& clause_set, VEC_TYPE(CL_REF)& restart_learnts, uint32_t var_cnt )
{
	
	for( var_t var1 = 1; var1 < var_cnt+1; var1++ )
	{
		for( uint32_t t = 0; t < threads; ++t ){
			VEC_CLEAR( CL_REF, watch[t][ index( lit(var1,POS) ) ] );
			VEC_CLEAR( CL_REF, watch[t][ index( lit(var1,NEG) ) ] );
		}
	}

	for( uint32_t round = 0; round < 2; ++ round ){
		VEC_TYPE( CL_REF )& clauses = ( (reinitFormulaFirst && round == 0) || (!reinitFormulaFirst && round == 1)  ) ? clause_set : restart_learnts;
	
		
		
		for(uint32_t i = 0; i<VEC_SIZE( CL_REF,  clauses ); ++i )
		{
			CLAUSE& cl = gsa.get(clauses[i]);
			if( CL_SIZE( cl ) == 1 ){
				
				lit_t literal = CL_GET_LIT( cl, 0 );
				
				if( assi_variable_equal( search.assi, var(literal), pol( literal ) ) )
					continue;
			
				for( uint32_t i = 1; i < threads; i ++ ){
					enqueue_literal( sds[i-1], literal, reasonStruct(), i );
					sds[i-1].VAR_LEVEL( var(literal) ) = 0;
				}
				enqueue_literal( search, literal, reasonStruct(), 0 );
				search.VAR_LEVEL( var(literal) ) = 0;
			} else {
				
				uint32_t addIndex = addClauseInTask(clauses[i]);
				watch_t watchMe;
	#ifdef BLOCKING_LIT
				watchMe.lit1 = CL_GET_LIT( cl, 0 );
	#endif
	#ifdef USE_IMPLICIT_BINARY
				if( CL_SIZE(cl) > 2 ){
	#endif
					watchMe.clause = reasonStruct(clauses[i]);
					watch_clause(search, watchMe, CL_GET_LIT(cl, 0), addIndex );
					watch_clause(search, watchMe, CL_GET_LIT(cl, 1), addIndex );
	#ifdef USE_IMPLICIT_BINARY
				} else {
					
					watchMe.clause = reasonStruct(CL_GET_LIT(cl, 1), true);
					watch_clause(search, watchMe, CL_GET_LIT(cl, 0),addIndex );
					watchMe.clause = reasonStruct(CL_GET_LIT(cl, 0),true);
					watch_clause(search, watchMe, CL_GET_LIT(cl, 1),addIndex );
				}
	#endif
			}
		}
	}

}

uint32_t PWatchedPropagation::addClauseInTask( const CL_REF& clause ){
	
	uint32_t ret = add_here;
	add_here ++;
	add_here = ((add_here - threads) == 0) ? 0 : add_here;
	return ret;
}

void PWatchedPropagation::set_parameter( const StringMap& commandline ){
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"up_swap" ) )  swap_watches = 0 != atoi( commandline.get( (const char*)"up_swap" ).c_str() );
	if( commandline.contains( (const char*)"up_prop_order" ) )  keep_order_prop = 0 != atoi( commandline.get( (const char*)"up_prop_order" ).c_str() );
	if( commandline.contains( (const char*)"up_remove_order" ) )  keep_order_remo = 0 != atoi( commandline.get( (const char*)"up_remove_order" ).c_str() );
	if( commandline.contains( (const char*)"up_bfs" ) )  bfs = 0 != atoi( commandline.get( (const char*)"up_bfs" ).c_str() );
	if( commandline.contains( (const char*)"up_foreward" ) )  foreward = 0 != atoi( commandline.get( (const char*)"up_foreward" ).c_str() );
	if( commandline.contains( (const char*)"up_threads" ) )  threads = atoi( commandline.get( (const char*)"up_threads" ).c_str() );
	if( commandline.contains( (const char*)"up_debug" ) )  debug = atoi( commandline.get( (const char*)"up_debug" ).c_str() );
	if( commandline.contains( (const char*)"up_search_sat" ) ) search_sat = atoi( commandline.get( (const char*)"up_search_sat" ).c_str() );
	if( commandline.contains( (const char*)"up_init_order" ) )  reinitFormulaFirst = 0 != atoi( commandline.get( (const char*)"up_init_order" ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		cerr << "=== watched propagation information ===" << endl;
		cerr << " parameter        pos.value  info" << endl;
		cerr << " up_swap          0,1        current watch is on second position in clause after propagation"  << endl;
		cerr << " up_prop_order    0,1        keep order, if clause is watched on another literal" << endl;
		cerr << " up_remove_order  0,1        keep order, if clause is removed due to removal heuristic" << endl;
		cerr << " up_bfs           0,1        propagate implications in bfs or dfs manner" << endl;
		cerr << " up_init_order    0,1        0 = formula first, 1 = learned first" << std::endl;
		cerr << " up_search_sat    0,n        look for first n literals first whether they are sat and can be watched" << std::endl;
		cerr << " up_foreward      0,1        do proagation in clause vector foreward" << endl;
		cerr << " up_threads       0,1        how many threads to use" << endl << endl;
		cerr << " up_debug         0,1        debug output level" << endl << endl;
		cerr << endl;
	}
#endif
#endif
}


#ifdef COMPILE_LIBRARY

extern "C" PWatchedPropagation* create_unitpropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt,  const uint32_t max_size, const StringMap& commandline){
	return new PWatchedPropagation( search, clause_set, var_cnt, search.assi, max_size, commandline );
}

extern "C" void destroy_unitpropagation( PWatchedPropagation* propagation ){
	delete propagation;
}
#endif
