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


#ifndef _BACKTRACK_HEURISTIC
#define _BACKTRACK_HEURISTIC


#include "types.h"
#include "sat/searchdata.h"
#include "structures/c_assignment.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/vector_macros.h"
#include "utils/stringmap.h"
#include "utils/sort.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>


class BacktrackHeuristic {
	
	CONST_PARAM uint32_t backtrack_mode;	
	CONST_PARAM uint32_t condition_mode;	
	CONST_PARAM uint32_t recalcDistance;	
	CONST_PARAM bool shrinkLast;	
	
	
	uint32_t conflicts; 
	double meanSum;	
	double squareSum;	
	double realConflictClauses;	
	uint32_t threshold;	
	bool lastWasShrinked;	
	
public:
	
	BacktrackHeuristic( const StringMap& commandline);

	
	~BacktrackHeuristic();

	
	int32_t backtrack_to( searchData& sd, VEC_TYPE(CL_REF)& clauses);
private:

	
	void updateData( VEC_TYPE(CL_REF)& clauses );
	
	
	bool check_shrink_condition( searchData& sd, VEC_TYPE(CL_REF)& clauses);
	
	
	int32_t shrink( searchData& sd, VEC_TYPE(CL_REF)& clauses);
	
	
	void recalc();

	
	void set_parameter(const StringMap& commandline );
};

#endif
