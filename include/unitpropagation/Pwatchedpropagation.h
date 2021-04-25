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


#ifndef _WATCHEDPROPAGATION_H
#define _WATCHEDPROPAGATION_H

#include <iostream>
#include <sstream>

#include <assert.h>

#include <pthread.h>	
#include <semaphore.h>

#include "macros/stack_macros.h"
#include "macros/vector_macros.h"
#include "macros/ringbuffer_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "types.h"
#include "structures/c_assignment.h"
#include "structures/c_boolarray.h"
#include "utils/lock.hh"
#include "utils/jobqueue.hh"
#include "utils/stringmap.h"
#include "utils/statistics.h"
#include "debug.h"

#include "utils/prefetch.h"
#include "sat/searchdata.h"

using namespace std;



class PWatchedPropagation
{
	
	struct propStruct{
		PWatchedPropagation* _prop;
		searchData* _search;
		VEC_TYPE(conflictStruct)* _conflict_clauses;
		assi_t* _assi;
		int _threadnr;
		propStruct( PWatchedPropagation* prop,searchData* search, VEC_TYPE(conflictStruct)* conflict_clauses, int threadnr ) : _prop(prop), _search( search), _conflict_clauses(conflict_clauses), _threadnr(threadnr)
		{}
		propStruct() : _prop(0), _search(0), _conflict_clauses(0), _threadnr(0) {}
	};

	
	
	struct cacheLine{
		uint32_t data;
		uint32_t fill [15];
	};
	
	
	struct watch_t{
#ifdef BLOCKING_LIT
		lit_t	lit1;
#endif
		
		reasonStruct clause;
	} WATCH_STRUCT_PACK ;
	
	
	VEC_TYPE( VEC_TYPE( watch_t) )* watch; 
	
	searchData* sharedSD;	
	searchData* sds;	
	int32_t conflictThread;	
	conflictStruct globalConflict;	
	
	uint32_t threads;	
	uint32_t* unit_ind_thread;	
	propStruct* propTasks; 
	
	JobQueue jobqueue;	
	Lock wLocker;	
	Lock cLocker;	
	Lock sLocker;	
	
	uint32_t add_here;	
	
	lit_t** unit_queue;		
	cacheLine* queue_size;	
	
	boolarray_t inserted_variable;		
	CONST_PARAM bool swap_watches;		
	CONST_PARAM bool keep_order_prop;	
	CONST_PARAM bool keep_order_remo;	
	CONST_PARAM bool bfs;				
	CONST_PARAM bool foreward;			
	CONST_PARAM uint32_t search_sat; 	
	CONST_PARAM bool reinitFormulaFirst;
	CONST_PARAM uint32_t debug;		
public:
	
	
	PWatchedPropagation(searchData& search, VEC_TYPE( CL_REF )* clause_set, const uint32_t max_size, const StringMap& commandline);

	~PWatchedPropagation();

	
	void enqueue_decision(searchData& search, lit_t decision_literal );

	void propagate(searchData& search, VEC_TYPE( conflictStruct  )& conflicted_clauses );

	bool clauses_added(searchData& search, VEC_TYPE(CL_REF)& clauses);

	void clauses_removed(searchData& search, VEC_TYPE(CL_REF)& clauses  );

	void backtrack(searchData& search, int32_t level , VEC_TYPE(var_t)& undefed_literals);

	void reinit(searchData& search, VEC_TYPE( CL_REF )& clause_set, VEC_TYPE(CL_REF)& restart_learnts, uint32_t var_cnt );
	
	
	void literals_conflicted( VEC_TYPE(lit_t)& lits ){}
private:
	bool clause_added(searchData& search, CL_REF clause);

	static void* clauses_remove_single(void *argument);	
	void clauses_removed_thread(searchData& search, VEC_TYPE(CL_REF)& clauses, uint32_t threadnr );

	void watch_clause(searchData& search, watch_t clause, lit_t watched_literal, uint32_t threadnr );

	
	bool enqueue_literal(searchData& search, lit_t literal, const reasonStruct& reason, const uint32_t threadnr);

	static void* back_single(void *argument);
	void back_thread( searchData& search, int32_t level, uint32_t threadnr );
	
	static void* propagate_single(void *argument);
	void propagate_thread(searchData& search, VEC_TYPE( conflictStruct  )& conflict_clauses, uint32_t threadnr );
	conflictStruct propagate_literal(searchData& search, lit_t literal, uint32_t threadnr);

	
	uint32_t addClauseInTask(const CL_REF& clause);

	
	void set_parameter(const StringMap& commandline );
	
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
