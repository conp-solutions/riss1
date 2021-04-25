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


#ifndef _SATELLIKE_H
#define _SATELLIKE_H

#include "defines.h"
#include "types.h"

#include "structures/literal_system.h"
#include "structures/c_assignment.h"
#include "structures/cpp_heap.h"
#include "structures/c_boolarray.h"

#include "macros/vector_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/heap_macros.h"

#include "sat/searchdata.h"

#include "utils/stringmap.h"
#include "utils/microtime.h"
#include "utils/sort.h"
#include "utils/varfileparser.h"

#include <vector>

struct elimination_t{
	var_t		var1;
	std::vector<CL_REF>	clauses;
};


 
class SatellikePreprocessor
{
 private:
 
  CONST_PARAM bool      print_dimacs; 
  bool      run_pure; 
  CONST_PARAM bool      run_ve; 
  CONST_PARAM uint32_t  mode;	
  CONST_PARAM bool		process_blocked;	
  CONST_PARAM uint32_t  block_max_cls_size;	
  CONST_PARAM uint32_t  block_max_occs;	
  CONST_PARAM uint32_t	max_cls;	
  int32_t	max_solutions;	
  std::string blackFile;	
  std::string whiteFile;	
  
  
	solution_t							result;
	uint32_t								clause_cnt;
	uint32_t								var_cnt;
	uint32_t								var_cnt_postprocess;

	uint32_t block_remove;	

  boolarray_t touched_variable;
  boolarray_t elimWhite; 

  VEC_TYPE( CL_REF ) subsumption_queue;
  VEC_TYPE( CL_REF )  *clauses;
  
  VEC_TYPE( VEC_TYPE( CL_REF  ) ) occurences_pos;
  VEC_TYPE( VEC_TYPE( CL_REF  ) ) occurences_neg;
  
  uint32_t *var_occurrences;
  searchData* data;	

  VEC_TYPE( lit_t ) unit_queue;
  VEC_TYPE( lit_t ) unit_queue_postprocess;

	
	std::vector<elimination_t> unblock_list;

	
  std::vector< elimination_t > resolution_queue_postprocess;

	cpp_heap<uint32_t, std::less<uint32_t> > resolution_queue;

 public:
	SatellikePreprocessor( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline );
	~SatellikePreprocessor();
	
	solution_t preprocess(searchData& sd,VEC_TYPE( CL_REF )* formula);
	
	bool doSimplify(searchData& sd){return false;}
	solution_t simplifysearch (searchData& sd, VEC_TYPE (CL_REF)& clauses, VEC_TYPE (CL_REF)& learnt);
	
	void postprocess(  VEC_TYPE(assi_t)* assignments);
	
	
	void addEquis( const VEC_TYPE(lit_t)& eqs ){}

private:
	
	void init( VEC_TYPE( CL_REF ) *cls, uint32_t *clause_count, uint32_t *var1_count);
	
	
	solution_t propagate( VEC_TYPE( lit_t )* unit_queue, VEC_TYPE( lit_t )* remind_unit_queue, uint32_t *clause_cnt, assi_t assignment);
	
	
	void reduceClauses(VEC_TYPE( CL_REF ) *clauses, VEC_TYPE( CL_REF ) *cls,uint32_t *clause_count, uint32_t clause_cnt);
	
	
	void destroy_pure_literal( assi_t assignment );
	
	
	solution_t subsumption_and_simplification();  
	
	
	solution_t resolve_var1(assi_t assignment, var_t var1, int force=0);
	
	
	void remove_clause_from_occurrences(CL_REF clause);
	
	
	void add_active_clauses();
	
	
	void delete_structures();

	
	solution_t remove_blocked_clauses(VEC_TYPE( CL_REF )* cls);

	void postprocessSingle( assi_t assignment);

	
	void set_parameter(const StringMap& commandline );
};

#endif
