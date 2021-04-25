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


#ifndef _VAR_ACTIVITY_HEURISTIC
#define _VAR_ACTIVITY_HEURISTIC


#include "types.h"
#include "structures/c_assignment.h"
#include "macros/heap_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/vector_macros.h"
#include "macros/stack_macros.h"
#include "utils/stringmap.h"
#include "sat/searchdata.h"

#include "debug.h"

#include <cmath>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;


class VariableActivityHeuristic {
		
		
		
		HEAP_TYPE( double ) sorting;

		
		CONST_PARAM bool				keyboard; 
		CONST_PARAM bool				minisat_rnd;	
		CONST_PARAM bool				remove;	
		CONST_PARAM double				decay;	
		double	increment;
		CONST_PARAM double				max_increment;	
		CONST_PARAM unsigned			random_desicion_freq;	
		CONST_PARAM double				polarity_rate;	
		CONST_PARAM bool				inc_learn_literals;	
		CONST_PARAM bool				inc_clause_literals;	
		CONST_PARAM uint32_t			cc_lits_mode;	
		CONST_PARAM bool				learnedJW;	
		CONST_PARAM bool				vmtf;	
		CONST_PARAM int32_t				useDependendP;	
		
		CONST_PARAM int					polarity_mode; 
		CONST_PARAM int					phase_backup_mode; 
		CONST_PARAM uint32_t			tabu_size;	


		
		uint32_t	threshold_size;	
		double		occurence_plus;	

		float		*jw; 
		lit_t*		dependendOf;	

		uint32_t	*decision_timer;	
		uint32_t	timer;	

		uint64_t	var1_count;
		uint64_t	realVars;	
		uint64_t	decisions;
		uint64_t	decisions_neg;
		uint64_t	tabu_accesses;

		STACK_TYPE( lit_t ) assignMe;	

public:

	
	VariableActivityHeuristic( VEC_TYPE( CL_REF )* clause_set, searchData& sd, const StringMap& commandline);

	
	~VariableActivityHeuristic();

	
	void clauses_learned( const searchData& sd, VEC_TYPE(CL_REF)& clauses);

	
	
	lit_t get_literal( searchData& search);

	
	void literals_conflicted( searchData& sd, VEC_TYPE(lit_t)& lits );

	
	
	
	
	void variables_unassigned( VEC_TYPE(var_t)& var1s);

private:

	
	void fillAssignStack(  const searchData& sd, VEC_TYPE(CL_REF)& clauses);

	
	void set_parameter(const StringMap& commandline );
};




#endif
