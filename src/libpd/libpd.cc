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

#ifdef _LIBPD_ACTIVATED

#include <libpd/solver.h>

#include <assert.h>



#include "types.h"

#ifdef __cplusplus
	extern "C" {
#endif




LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( conflicts );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( decisions );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( restarts );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( deleted );

LIBPD_DECLARE_PERFORMANCE_DATA_TIME( read_in_time );
LIBPD_DECLARE_PERFORMANCE_DATA_TIME( preprocesstime );
LIBPD_DECLARE_PERFORMANCE_DATA_TIME( searchtime );
LIBPD_DECLARE_PERFORMANCE_DATA_TIME( postprocesstime );

LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( prepro_reduced_clauses );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( succeed_propagations );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( backtrackdistance_sum );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( restartdistance_sum );
LIBPD_DECLARE_PERFORMANCE_DATA_INTEGER( max_learnt_clauses_per_prop );


LIBPD_REGISTER {
	LIBPD_REGISTER_PERFORMANCE_DATA ( conflicts );
	LIBPD_REGISTER_PERFORMANCE_DATA ( decisions );
	LIBPD_REGISTER_PERFORMANCE_DATA ( restarts );	
	LIBPD_REGISTER_PERFORMANCE_DATA ( deleted );	

	LIBPD_REGISTER_PERFORMANCE_DATA ( read_in_time	);
	LIBPD_REGISTER_PERFORMANCE_DATA ( preprocesstime );
	LIBPD_REGISTER_PERFORMANCE_DATA ( searchtime );
	LIBPD_REGISTER_PERFORMANCE_DATA ( postprocesstime );

	LIBPD_REGISTER_PERFORMANCE_DATA ( prepro_reduced_clauses );
	LIBPD_REGISTER_PERFORMANCE_DATA ( succeed_propagations	);
	LIBPD_REGISTER_PERFORMANCE_DATA ( backtrackdistance_sum	);
	LIBPD_REGISTER_PERFORMANCE_DATA ( restartdistance_sum	);	
	LIBPD_REGISTER_PERFORMANCE_DATA ( max_learnt_clauses_per_prop		);
}




libpd_variable_t libpd_hook_read_variable(const void *variable)
{
	libpd_variable_t var;
	
	var = (libpd_variable_t)(*((var_t*)variable));

	return var;
}


libpd_literal_t libpd_hook_read_literal(const void *literal)
{
	libpd_literal_t lit;
	
	lit = (libpd_literal_t)((lit_t*)literal);

	return lit;
}


libpd_clause_t libpd_hook_read_clause(const void *clause)
{
	libpd_clause_t out_clause = LIBPD_INITIAL_CLAUSE;
	
	assert(0);
	
	return out_clause;
}


libpd_literal_t libpd_hook_clause_get_literal(const void *clause, size_t index)
{
	assert(0);
	
	return -1;
}


size_t 	libpd_hook_clause_get_size(const void *clause)
{
	assert(0);
	
	return -1;
}


libpd_integer_t libpd_hook_get_context()
{
	return 0;
}


#ifdef __cplusplus
	}
#endif

#endif
