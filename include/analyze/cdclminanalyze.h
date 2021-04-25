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


#ifndef _CDCL_MIN_ANALYZE_H
#define _CDCL_MIN_ANALYZE_H

#include "macros/vector_macros.h"
#include "macros/stack_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"

#include "structures/c_boolarray.h"

#include "utils/conflictgraph.h"
#include "utils/prefetch.h"

#include "sat/searchdata.h"

#include "debug.h"

using namespace std;


class CdclMinimizeAnalyzer {
	boolarray_t inserted_variable;	
	
	CONST_PARAM uint32_t minimize;			
	CONST_PARAM bool add_trivail_short;		
	CONST_PARAM int32_t new_level_limit;	
	CONST_PARAM uint32_t prefetch_reasons;	
	CONST_PARAM bool dump_conflict_graphs;	
	VEC_TYPE(lit_t) literals;	
	VEC_TYPE(lit_t) clear;		
	VEC_TYPE(lit_t) stack;		

	uint32_t analysis_cnt;	
	
	ConflictDump conflictDumper;	
public:

	
	CdclMinimizeAnalyzer( uint32_t var_cnt, const StringMap& commandline );
	
	
	~CdclMinimizeAnalyzer();
	
	
	void analyze(searchData& search, VEC_TYPE(CL_REF)& used_clauses, VEC_TYPE(conflictStruct)& conflict_clauses, VEC_TYPE(CL_REF)& learned_clauses, VEC_TYPE(lit_t)& used_literals, VEC_TYPE(CL_REF)& otfClauses );
	
private:

	
	CL_REF analyze(searchData& search, VEC_TYPE(CL_REF)& used_clauses, conflictStruct conflict_clause, VEC_TYPE(lit_t)& used_literals, VEC_TYPE(CL_REF)& otfs );
	
	
	bool litRedundant(searchData& search, lit_t p, uint32_t abstract_levels);
	
	
	void set_parameter( const StringMap& commandline );
};

#endif
