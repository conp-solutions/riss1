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


#ifndef _CSP_PROPAGATION_H
#define _CSP_PROPAGATION_H

#include <string.h>
#include <assert.h>

#include "macros/stack_macros.h"
#include "macros/vector_macros.h"
#include "macros/ringbuffer_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "types.h"
#include "structures/c_assignment.h"
#include "structures/c_boolarray.h"
#include "utils/stringmap.h"
#include "utils/statistics.h"
#include "utils/monitor.h"
#include "debug.h"
#include "sat/searchdata.h"

#include "macros/heap_macros.h"

#include "utils/prefetch.h"







class CspPropagation
{

public:
	
	struct dual_watch_t
	{
		lit_t	other;
#ifndef USE_IMPLICIT_BINARY
	CL_REF	clause;
#endif
	} WATCH_STRUCT_PACK ;

	
	struct watch_t{
#ifdef BLOCKING_LIT
		lit_t	lit1;
#endif
		CL_REF	clause;
	} WATCH_STRUCT_PACK ;

	
	VEC_TYPE( VEC_TYPE( watch_t ) ) watch; 
	RINGBUFFER_TYPE( lit_t ) unit_queue; 
	boolarray_t inserted_variable; 
	VEC_TYPE( VEC_TYPE( dual_watch_t ) ) implication; 
	boolarray_t seen_implications;	

	
	VEC_TYPE( VEC_TYPE( CL_REF ) ) csp; 
	
	uint32_t unseen_ind; 
	uint32_t var_cnt;	
	uint32_t max_size;	
	
	VEC_TYPE( lit_t ) impliedLits;	
	VEC_TYPE( reasonStruct ) reasons; 
	
	
	CONST_PARAM bool implication_closure; 
	CONST_PARAM bool search_sat; 
	CONST_PARAM bool long_conflicts; 
	CONST_PARAM bool swap_watches;	
	CONST_PARAM bool keep_order_prop; 
	CONST_PARAM bool keep_order_remo; 
	CONST_PARAM bool bfs;	
	CONST_PARAM bool unit_fifo; 
	CONST_PARAM bool lastPhase;	
	CONST_PARAM bool reinitFormulaFirst; 
	CONST_PARAM uint32_t prefetched_items; 
	CONST_PARAM bool doCsp; 
	CONST_PARAM uint32_t minXorSize; 
public:		
	
	
	CspPropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, const uint32_t max_size, const StringMap& commandline);
	
	
	~CspPropagation();

	
	void enqueue_decision(searchData& search, lit_t decision_literal );

	
	void propagate(searchData& search, VEC_TYPE( conflictStruct  )& conflicted_clauses );

	
	bool clauses_added(searchData& search, VEC_TYPE(CL_REF)& clauses);
	
	
	void clauses_removed(searchData& search, VEC_TYPE(CL_REF)& clauses );

	
	void backtrack(searchData& search, int32_t level , VEC_TYPE(var_t)& undefed_literals );

		
	void reinit(searchData& search, VEC_TYPE( CL_REF )& clause_set, VEC_TYPE(CL_REF)& restart_learnts, uint32_t var_cnt );

	
	void literals_conflicted( VEC_TYPE(lit_t)& lits );
private:
	
	bool clause_added(searchData& search, CL_REF clause);

	
	void watch_clause(searchData& search, watch_t clause, lit_t watched_literal );

	
	void watch_dual(searchData& search, CL_REF clause );

	
	void watch_csp( CL_REF clause, lit_t literal );

	
	bool enqueue_literal(searchData& search, lit_t literal,  const reasonStruct& reason );

	
	lit_t dequeue_literal();

	
	lit_t queue_get_literal( const uint32_t ind ) const;

	
	conflictStruct propagate_literal(searchData& search, lit_t literal);
	
	
	conflictStruct propagate_implications(searchData& search, lit_t literal);
	
	
	conflictStruct propagate_csp(searchData& search, lit_t literal);
	
	
	void prefetch_watch( const lit_t literal ) const;

	
	void sortClauseLevel(searchData& search, CLAUSE& cl, uint32_t start, uint32_t end) const;


	
	void setupXor( VEC_TYPE( CL_REF )& cl1, VEC_TYPE( CL_REF )& cl2 );

	
	void setup1ooN( VEC_TYPE( CL_REF )& cl1, VEC_TYPE( CL_REF )& cl2 );

	
	void sort(VEC_TYPE( CL_REF )& clauses);

	
	void set_parameter(const StringMap& commandline);
	
	#ifdef PROPAGATION_METRICS
		uint64_t processed_clauses;
		uint64_t clauses_size_sum;
		uint64_t processed_literals;
		uint64_t calls_propagate;
		uint64_t calls_propagate_lit1;
		uint64_t calls_propagate_dual;
	#endif
};

#endif
