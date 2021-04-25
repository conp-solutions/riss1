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


#ifndef _NO_PREPROCESSOR_H
#define _NO_PREPROCESSOR_H

#include "structures/literal_system.h"
#include "macros/vector_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"

#include "utils/stringmap.h"
#include "utils/microtime.h"

#include "structures/c_assignment.h"

#include "sat/searchdata.h"


class NoPreprocessor
{
public:
	
	NoPreprocessor(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline ){}
	
	
	~NoPreprocessor( ){}

	
	solution_t preprocess ( searchData& sd, VEC_TYPE( CL_REF )* formula ){return UNKNOWN;}
	
	
	bool doSimplify(searchData& sd){return false;}
	
	
	solution_t simplifysearch (searchData& sd, VEC_TYPE (CL_REF)& clauses, VEC_TYPE (CL_REF)& learnt){return UNKNOWN;}
	
	
	void postprocess (VEC_TYPE (assi_t) * assignments){}
	
	
	void addEquis( const VEC_TYPE(lit_t)& eqs ){}

private:

	
	void postprocessSingle (assi_t assignment){}

	
	void set_parameter(const StringMap& commandline ){}
};

#endif
