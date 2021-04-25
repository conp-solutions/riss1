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


#ifndef _CLS_RATIO_EVENT_HEURISTIC
#define _CLS_RATIO_EVENT_HEURISTIC


#include <stdlib.h>
#include <assert.h>
#include <cstring>
#include <string>

#include "utils/stringmap.h"
#include "debug.h"

#include "structures/c_assignment.h"

#include "sat/searchdata.h"

#include "macros/vector_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"


class ClsRatioEventHeuristic{
private:
	
	uint32_t formula_cls_cnt;	
	uint32_t learnt_cnt;
	
	
	double learntsize_factor; 
	CONST_PARAM double learntsize_inc; 
	
	std::string event;
public:
	ClsRatioEventHeuristic(char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline);

	~ClsRatioEventHeuristic();

	bool required(uint32_t conflicts, uint32_t decisions, const searchData& sd);

	void clauses_added( const VEC_TYPE(CL_REF)& clauses, const searchData& search );

	void clauses_removed( const VEC_TYPE(CL_REF)& clauses );

	void restart_done();
	
private:
	void set_parameter(char* event_name, const StringMap& commandline );
};

#endif
