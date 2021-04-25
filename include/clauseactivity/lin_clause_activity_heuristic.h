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


#ifndef _LIN_CL_ACTIVITY_HEURISTIC
#define _LIN_CL_ACTIVITY_HEURISTIC

#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "types.h"
#include "utils/stringmap.h"
#include "utils/sort.h"
#include "debug.h"
#include "sat/searchdata.h"


class LinClauseActivity
{
	CONST_PARAM int init_mode; 
	float inc;	
	CONST_PARAM float inc_decay;	
	CONST_PARAM bool care_undefined;	

public:
	
	LinClauseActivity( const StringMap& commandline );

	
	~LinClauseActivity();

	
	void set_learned_activity(searchData& search, VEC_TYPE( CL_REF )& clauses );

	
	bool inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  );

	
	void restart_done();

	
	float overflow_divisor(){ return 1e20f; }
private:
	
	void set_parameter( const StringMap& commandline );

};

#endif
