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


#include "preprocessor/satellike.h"

#include<set>

SatellikePreprocessor::SatellikePreprocessor(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline ):
	print_dimacs( false),
	run_pure(true),
	run_ve(true),
	mode (1),
	process_blocked( false),
	block_max_cls_size ( 7),
	block_max_occs ( 15),
	max_cls ( 4000000 )
{
	block_remove = 0;
	
	this->var_cnt = var_cnt;
	
	elimWhite = boolarray_create (var_cnt + 1);
	
	set_parameter( commandline );
}

SatellikePreprocessor::~SatellikePreprocessor(){}

struct mysorter_s
{
  bool operator() (lit_t i,lit_t j) { return var(i)<var(j);}
};

void SatellikePreprocessor::delete_structures(){
  
  delete [] var_occurrences;
  VEC_CLEAR( lit_t, unit_queue );
  VEC_CLEAR( CL_REF, subsumption_queue );
  boolarray_destroy( touched_variable );
  
  VEC_DESTROY( VEC_TYPE( CL_REF  ), occurences_pos );
  VEC_DESTROY( VEC_TYPE( CL_REF  ), occurences_neg );
}

solution_t SatellikePreprocessor::preprocess( searchData& sd, VEC_TYPE( CL_REF )* cls)
{

	
	

	data = &sd;
	
	assi_t assignment = assi_create(var_cnt);

	uint32_t cls_cnt = VEC_SIZE( CL_REF, (*cls) );
	uint32_t this_var_cnt = var_cnt;
	uint32_t *clause_count = &cls_cnt;
	uint32_t *var1_count = &this_var_cnt;

	const bool check_order = false;
	if( check_order ){
		
		std::cerr << "c check clause ordering" << std::endl;
		bool ordered = true;
		for( uint32_t i = 0; i < VEC_SIZE( CL_REF, (*cls) ); ++i ){
			const CL_REF clause = (*cls)[i];
			CLAUSE& cl = gsa.get(clause);
			for( uint32_t j = 0 ; j < CL_SIZE( cl ) - 1 ; ++j ){
				if( CL_GET_LIT( cl, j ) > CL_GET_LIT( cl, j+1) ){ ordered = false; break; }
			}
		}
		if( ordered ) std::cerr << "c literals in clauses are ordered before preprocessing" << std::endl;
	}

	
  if(*clause_count >= max_cls)
  {
  	assi_destroy( assignment, var_cnt );
  	return UNKNOWN;
  }

  bool found_unit = true;
  
  init(cls, clause_count, var1_count);

  
  if( run_pure ) destroy_pure_literal( assignment );

  
  if(mode == 1){
		var_t variable;  
     
		if( process_blocked ) {
		  result = remove_blocked_clauses( cls );
		  if(result != UNKNOWN){
				assi_destroy( assignment, var_cnt );
				return result;
			}
		}
		
		if( run_ve ) {
			while( VEC_SIZE( CL_REF, subsumption_queue ) > 0 || !resolution_queue.is_empty())
			{
				
				result = subsumption_and_simplification();
				if(result != UNKNOWN)
				{
	  			assi_destroy( assignment, var_cnt );
					return result;
				}
	
				while(!resolution_queue.is_empty()){
					variable = resolution_queue.item_at(0);
					resolution_queue.remove_item(variable);

					result = resolve_var1( assignment, variable);
					if(result != UNKNOWN){
						assi_destroy( assignment, var_cnt );
						return result;
					}
				}
				
				add_active_clauses();
			}
		}
		
		
		result = propagate(  &unit_queue, &unit_queue_postprocess, &clause_cnt, assignment);
		if(result != UNKNOWN)
		{
 			assi_destroy( assignment, var_cnt );
			return result;
		}
		
		
		if( blackFile.size() != 0 ){
			VarFileParser vfp( blackFile );
			VEC_TYPE(var_t) evars;
			VEC_CREATE(var_t,evars);

			vfp.extract(evars);

			std::cerr << "c force variable elimination of " << VEC_SIZE(var_t,evars) << " variables" << std::endl;
			for( uint32_t i = 0 ; i < VEC_SIZE(var_t,evars); ++i ){
				
				resolve_var1(assignment, evars[i], (print_dimacs)? 2 : 1 );
			}
		}
		
		
		reduceClauses(clauses, cls, clause_count, clause_cnt);      
  }

  
  if(mode == 2){
		var_t variable;
		while(found_unit){
			
			found_unit = false;
		  if( process_blocked ) {
				result = remove_blocked_clauses( cls );
				if(result != UNKNOWN){
					assi_destroy( assignment, var_cnt );
					return result;
				}
			}
			
			if( run_ve ) {
				while( VEC_SIZE( CL_REF, subsumption_queue ) > 0 || !resolution_queue.is_empty()){  
					
					result = subsumption_and_simplification();
					if(result!=UNKNOWN)
					{
			 			assi_destroy( assignment, var_cnt );
						return result;
					}
			
					while(!resolution_queue.is_empty()){
						variable = resolution_queue.item_at(0);
						resolution_queue.remove_item(variable);
					
						result = resolve_var1( assignment, variable);
						if(result != UNKNOWN)
						{
				 			assi_destroy( assignment, var_cnt );
							return result;
						}
					
					}
					
					add_active_clauses();
				}
			}
			
			if( VEC_SIZE( lit_t, unit_queue ) > 0)
				found_unit = true;

			result = propagate(  &unit_queue, &unit_queue_postprocess, &clause_cnt, assignment);
			if(result != UNKNOWN)
			{
	 			assi_destroy( assignment, var_cnt );
				return result;
			}
			reduceClauses(clauses, cls, clause_count, clause_cnt);
			add_active_clauses();
		}
	}

  
  if(mode == 3){
  
  	for( uint32_t i = 0; i < 2; ++i){
  	
  		
  		
			var_t variable;
		  
		  std::cerr << "c prep(" << i+1 << ") clss: " << VEC_SIZE( CL_REF, (*cls) ) << std::endl;
		  
		  if( process_blocked ) {
				result = remove_blocked_clauses( cls );
				if(result != UNKNOWN){
					assi_destroy( assignment, var_cnt );
					return result;
				}
			}
			if( process_blocked ) std::cerr << "c removed clauses due to blocking so far: " << block_remove << std::endl;
			
			if( run_ve ) {
				while( VEC_SIZE( CL_REF, subsumption_queue ) > 0 || !resolution_queue.is_empty()){  
					
					result = subsumption_and_simplification();
					if(result!=UNKNOWN)
					{
			 			assi_destroy( assignment, var_cnt );
						return result;
					}
					while(!resolution_queue.is_empty())
					{
						variable = resolution_queue.item_at(0);
						resolution_queue.remove_item(variable);

						result = resolve_var1(  assignment, variable);
						if(result != UNKNOWN)
						{
				 			assi_destroy( assignment, var_cnt );
							return result;
						}
					}
					
					add_active_clauses();
				}
			}
			result = propagate(  &unit_queue, &unit_queue_postprocess, &clause_cnt, assignment);
			if(result != UNKNOWN)
			{
	 			assi_destroy( assignment, var_cnt );
				return result;
			}
		
		}

		reduceClauses(clauses, cls, clause_count, clause_cnt);
		
  }
 
  if(print_dimacs){
	uint32_t unitClauses = 0;
	
	for( var_t v = 1; v <= var_cnt; v++ ) if( ! assi_is_undef( assignment,  v) ) unitClauses ++ ;
    std::cout << "p cnf " << *var1_count << " " << unitClauses + VEC_SIZE( CL_REF, (*cls) ) << std::endl;
    
    for( var_t v = 1; v <= var_cnt; v++ ){
    	if( ! assi_is_undef( assignment,  v) ) std::cout << nr( lit(v, assi_get_polarity( assignment,v ) ) ) << " 0" << std::endl;
    }
    
    for(uint32_t ind_clause = 0; ind_clause < VEC_SIZE( CL_REF, (*cls) ); ++ind_clause)
    {
    	CLAUSE& cl = gsa.get((*cls)[ ind_clause ]);
    	for( uint32_t i = 0; i < CL_SIZE( cl ); ++i ) std::cout << nr( CL_GET_LIT( cl, i ) ) << " ";
    	std::cout << "0" << std::endl;
    }
    
    exit(12);
  }
  
  delete_structures();
 
	assi_destroy( assignment, var_cnt );
	
	if( process_blocked ) std::cerr << "c removed clauses due to blocking: " << block_remove << std::endl;
 
  return result;
}

void SatellikePreprocessor::add_active_clauses(){
  for(uint32_t ind_clauses = 0; ind_clauses < VEC_SIZE( CL_REF, (*clauses) ); ++ind_clauses)
  {
  	CLAUSE& cl = gsa.get((*clauses)[ind_clauses]);
    if(! CL_IS_IGNORED( cl ) )
    {
      for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
      {
				if(  boolarray_get( touched_variable, var(  CL_GET_LIT( cl,ind_clause ) )  )  ){
					VEC_PUSH_BACK( CL_REF, subsumption_queue, (*clauses)[ind_clauses] );
					break;
				}
			}
		}
	}

  for(uint32_t ind_touched = 1; ind_touched < (var_cnt + 1 ); ++ind_touched){
    if(  boolarray_get( touched_variable,ind_touched ) ){
      if(resolution_queue.contains(ind_touched)) resolution_queue.update_item(ind_touched);
      else resolution_queue.insert_item(ind_touched);
    }
    boolarray_set( touched_variable, ind_touched, false);
  }
  
}

void SatellikePreprocessor::remove_clause_from_occurrences(CL_REF clause){
  lit_t literal;
  var_t variable;
	CLAUSE& cl = gsa.get(clause);
  for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ind_clause++){
    literal = CL_GET_LIT( cl, ind_clause );
    variable = var(literal);
    
    if(resolution_queue.contains(variable)){
      if(var_occurrences[variable] >= 1){
				var_occurrences[variable]--;   
				resolution_queue.update_item(variable);
      }
      else{
    	  std::cerr << "remove var " << variable << " from heap" << std::endl;
    	  resolution_queue.remove_item(variable);

    	  var_occurrences[variable] = 0;
      }
    }    
    
    if(pol(literal) == POS){
      for( uint32_t it_clause = 0; it_clause < VEC_SIZE( CL_REF, occurences_pos[variable] ); ++it_clause)
      {
				if(clause == occurences_pos[variable][it_clause] ){
					VEC_ERASE( CL_REF, occurences_pos[variable], it_clause );
					break;
				}
			}
    }
    else
    {
      for( uint32_t it_clause = 0; it_clause < VEC_SIZE( CL_REF, occurences_neg[variable] ); ++it_clause)
      {
				if(clause == occurences_neg[variable][it_clause] ){
					VEC_ERASE( CL_REF, occurences_neg[variable], it_clause );
					break;
				}
			}
		}
    
  }
  
}

solution_t SatellikePreprocessor::resolve_var1(assi_t assignment, var_t var1, int force){
  if( assi_get_polarity( assignment, var1 ) != UNDEF)
    return UNKNOWN;

  if ( boolarray_get(elimWhite,var1) ) return UNKNOWN;

  
  uint32_t number_of_new_clauses, number_of_clauses_pos_neg;

  CL_REF resolve_clause;
  CL_REF clause;
  lit_t literal;
  var_t variable;

  uint32_t number_of_old_clauses;
  
  elimination_t elimination;

  number_of_old_clauses = VEC_SIZE( CL_REF, (*clauses) );

  VEC_TYPE(CL_REF )& clauses_pos = occurences_pos[var1];
  VEC_TYPE(CL_REF )& clauses_neg = occurences_neg[var1];

  number_of_clauses_pos_neg = VEC_SIZE( CL_REF, clauses_pos ) + VEC_SIZE( CL_REF, clauses_neg );

  number_of_new_clauses = 0;
  
  
  
  
  
  
  for(uint32_t it_clauses_pos = 0; it_clauses_pos < VEC_SIZE( CL_REF, clauses_pos ); it_clauses_pos++)
  {
    for( uint32_t it_clauses_neg = 0; it_clauses_neg < VEC_SIZE( CL_REF, clauses_neg ); it_clauses_neg++)
    {
      if( CL_CHECK_RESOLVE( gsa.get(clauses_pos[it_clauses_pos]), gsa.get(clauses_neg[it_clauses_neg]), var1 ) ) ++number_of_new_clauses;
      if(number_of_new_clauses > number_of_clauses_pos_neg && force == 0) return UNKNOWN;
    }
  }

  
  for(uint32_t it_clauses_pos = 0; it_clauses_pos < VEC_SIZE( CL_REF, clauses_pos ); it_clauses_pos++)
  {
    for( uint32_t it_clauses_neg = 0; it_clauses_neg < VEC_SIZE( CL_REF, clauses_neg ); it_clauses_neg++)
    {
    	CLAUSE* cp = CL_RESOLVE_VAR( gsa.get(clauses_pos[it_clauses_pos]), gsa.get(clauses_neg[ it_clauses_neg]), var1);
    	if( cp != 0 ){
	      resolve_clause = gsa.create( *cp );
	      VEC_PUSH_BACK( CL_REF, (*clauses), resolve_clause);
	    	delete cp;
	    }
    }
  }

	if( boolarray_get( data->eliminated, var1 ) ) return UNKNOWN;
  boolarray_set( data->eliminated, var1, true );
  
  
  elimination.var1 = var1;

  while(VEC_SIZE( CL_REF, occurences_pos[var1] ) > 0){
    clause = occurences_pos[var1][0];
    remove_clause_from_occurrences(clause);

    CL_IGNORE( gsa.get(clause) );
    elimination.clauses.push_back( clause );
    clause_cnt--;
  }
  
  while(VEC_SIZE( CL_REF, occurences_neg[var1] ) > 0){
    clause = occurences_neg[var1][0];
    
    remove_clause_from_occurrences(clause);

    CL_IGNORE( gsa.get(clause) );
    elimination.clauses.push_back( clause );
    clause_cnt--;
    
  }
  resolution_queue_postprocess.push_back(elimination);

  
  for(uint32_t ind_clauses = number_of_old_clauses; ind_clauses < VEC_SIZE( CL_REF, (*clauses) ); ++ind_clauses)
  {
    clause = (*clauses)[ind_clauses];
    CLAUSE& cl = gsa.get(clause);
    clause_cnt++;
    if(CL_SIZE( cl ) == 1)
      VEC_PUSH_BACK( lit_t, unit_queue, CL_GET_LIT( cl, 0 ));
      
		VEC_PUSH_BACK( CL_REF, subsumption_queue, clause);
		for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause){
			literal = CL_GET_LIT( cl, ind_clause );
			variable = var(literal);
	
			var_occurrences[variable]++;
	
			if(pol(literal) == POS) VEC_PUSH_BACK( CL_REF,occurences_pos[variable], clause );
			else VEC_PUSH_BACK( CL_REF,occurences_neg[variable], clause);
	
			if(resolution_queue.contains(variable)) resolution_queue.update_item(variable);
			else resolution_queue.insert_item(variable);
		}
  }
  return subsumption_and_simplification();
}

solution_t SatellikePreprocessor::remove_blocked_clauses( VEC_TYPE( CL_REF )* cls ){
	return UNKNOWN;
}

solution_t SatellikePreprocessor::subsumption_and_simplification(){
 
  var_t var1, variable;
  VEC_TYPE(CL_REF ) occurrence_del_lit1;
  VEC_TYPE(CL_REF ) occurrences;

  CL_REF clause_oc;
  lit_t lit1_oc;

  
  while( VEC_SIZE( CL_REF, subsumption_queue ) > 0 ){

    
    CL_REF clause = subsumption_queue[ VEC_SIZE( CL_REF, subsumption_queue ) -1];
    CLAUSE& cl = gsa.get(clause);
    VEC_POP_BACK( CL_REF, subsumption_queue );

    
    if(CL_IS_IGNORED( cl )) continue;

    
    var1 = var(CL_GET_LIT( cl, 0 ));
    for(uint32_t ind_clause = 1; ind_clause < CL_SIZE( cl ); ++ind_clause){
      if(var_occurrences[var(CL_GET_LIT( cl, ind_clause ))] < var_occurrences[var1]){
				var1 = var(CL_GET_LIT( cl, ind_clause ));
			}
		}

		
    
    for(uint32_t ind_pos_neg = 0; ind_pos_neg < 2; ++ind_pos_neg){      

      
      occurrences = (ind_pos_neg == 0) ? occurences_pos[var1] : occurences_neg[var1];



			
      for(uint32_t ind_oc = 0; ind_oc < VEC_SIZE( CL_REF, occurrences ); ++ind_oc){
	
				clause_oc = occurrences[ind_oc];
				CLAUSE& cl_oc = gsa.get( clause_oc );
				
				
				if(CL_IS_IGNORED( cl_oc ) || clause_oc == clause || ! CL_SUBSUMES_OR_SIMPLIFIES( cl,cl_oc, &lit1_oc) ){
					
					continue;
				}

				
				
				
				
				
				
				
				
				if(lit1_oc == NO_LIT){
					
					remove_clause_from_occurrences(clause_oc);
					CL_IGNORE_AND_DELETE(cl_oc);
					clause_cnt--;
					
					ind_oc--;
					
				}
				else
				{
					
					
		
		
					CL_DELETE_LIT( cl_oc, lit1_oc);

					
					variable = var(lit1_oc);
					if(pol(lit1_oc) == POS){
						
						for( uint32_t it_clause = 0; it_clause < VEC_SIZE( CL_REF, occurences_pos[variable] ); ++it_clause)
						{
							if(clause_oc == occurences_pos[variable][it_clause] ){
								VEC_ERASE( CL_REF, occurences_pos[variable], it_clause );
								break;
							}
						}
					}
					else
					{
						
						for( uint32_t it_clause = 0; it_clause < VEC_SIZE( CL_REF, occurences_neg[variable] ); ++it_clause)
						{
							if(clause_oc == occurences_neg[variable][it_clause] ){
								VEC_ERASE( CL_REF, occurences_neg[variable], it_clause );
								break;
							}
						}
					}
					
					
					if(CL_SIZE( cl_oc ) == 0){
						std::cerr << "unsat during subsumption" << std::endl;
						return UNSAT;
					}
		
					if(CL_SIZE( cl_oc ) == 1)
						VEC_PUSH_BACK( lit_t, unit_queue, CL_GET_LIT( cl_oc,0) );

					
					
					var_occurrences[var(lit1_oc)]--;
					
					
					
					
					
					VEC_PUSH_BACK( CL_REF, subsumption_queue, clause_oc);
				}
			}
		}
  }
  return result;
}

void SatellikePreprocessor::destroy_pure_literal( assi_t assignment ){
  bool found_pure_literal = true;

	VEC_CREATE( lit_t, unit_queue );

  
  for(uint32_t ind_oc = 1; ind_oc < VEC_SIZE( VEC_TYPE( CL_REF ), occurences_pos ); ++ind_oc){
  
  	
  	if ( boolarray_get(elimWhite,ind_oc) ) continue;
  
    if(assi_get_polarity( assignment, ind_oc ) == UNDEF){
      found_pure_literal = true;      
      for(uint32_t ind_cl = 0; ind_cl < VEC_SIZE( CL_REF , occurences_pos[ind_oc] ); ++ind_cl){
				if( CL_IS_IGNORED( gsa.get(occurences_pos[ind_oc][ind_cl]) ) ) continue;
				else{
					found_pure_literal = false;
					break;
				}
	
      }
      if(found_pure_literal){
				VEC_PUSH_BACK( lit_t, unit_queue,lit(ind_oc, NEG) );
				break;
      }
      found_pure_literal = true;
      for(uint32_t ind_cl = 0; ind_cl < VEC_SIZE( CL_REF , occurences_neg[ind_oc] ); ++ind_cl){
				if( CL_IS_IGNORED( gsa.get(occurences_neg[ind_oc][ind_cl]) ) ) continue;
				else{
					found_pure_literal = false;
					break;
				}
	
      }
      if(found_pure_literal){
				VEC_PUSH_BACK( lit_t, unit_queue, lit(ind_oc, POS));
				break;
      }      
      
    }
    
  }
  
}

void SatellikePreprocessor::reduceClauses(VEC_TYPE(CL_REF ) *clauses, 
					  VEC_TYPE(CL_REF ) *cls, 
					  uint32_t *clause_count,
					  uint32_t clause_cnt){

    
    VEC_TYPE(CL_REF ) clauses_reduced;
    VEC_CREATE( CL_REF, clauses_reduced );
    VEC_RESIZE( CL_REF, clauses_reduced, clause_cnt, 0);
    
    uint32_t ind_cr = 0;
    for(uint32_t ind = 0; ind < VEC_SIZE( CL_REF, (*clauses) ); ++ind){
    	CLAUSE& cl = gsa.get( (*clauses)[ind]);
      if(! CL_IS_IGNORED( cl ) ){
				clauses_reduced[ind_cr] = (*clauses)[ind];
				ind_cr++;
      }
      else if( CL_IS_IGNORED_AND_DELETED( cl ) ){
      	CL_DESTROY( cl );
      	gsa.release( (*clauses)[ind] );
      }
    }
    
    *clause_count = clause_cnt;
    std::cerr << "c set clausecount to " << clause_cnt << std::endl;

		VEC_DESTROY( CL_REF, (*cls) );
		
		VEC_RESIZE( CL_REF, clauses_reduced, ind_cr, 0);
		(*cls) = VEC_COPY( CL_REF, clauses_reduced );
		
    VEC_DESTROY( CL_REF, clauses_reduced );
		
		std::cerr << "c added clauses: " << ind_cr << std::endl;
		std::cerr << "c returned size: " << VEC_SIZE( CL_REF, (*cls) ) << std::endl;
}



solution_t SatellikePreprocessor::propagate( VEC_TYPE( lit_t )* unit_queue, VEC_TYPE( lit_t )* remind_unit_queue, uint32_t *clause_cnt, assi_t assignment)
{
		VEC_CREATE( lit_t, unit_queue_postprocess );
    
    lit_t unit_lit1;
    var_t unit_var1;
    pol_t unit_pol;
    while( VEC_SIZE( lit_t, (*unit_queue) ) != 0){
      
      unit_lit1 = (*unit_queue)[ VEC_SIZE( lit_t, (*unit_queue) ) - 1 ];
      unit_var1 = var(unit_lit1);
      unit_pol = pol(unit_lit1);

      
      VEC_POP_BACK( lit_t, (*unit_queue) );

      
      if( (assi_get_polarity( assignment, unit_var1 ) == POS && unit_pol == NEG) || (assi_get_polarity( assignment, unit_var1 ) == NEG && unit_pol == POS) ){
      	std::cerr << "UNSAT during propagation" << std::endl;
				return UNSAT;
      }

      if((assi_get_polarity( assignment, unit_var1 ) != UNDEF) ) continue;

			assi_set_polarity( assignment, unit_var1, unit_pol );
      VEC_PUSH_BACK( lit_t, unit_queue_postprocess, unit_lit1);
      
      
      
      
      
      if(unit_pol == NEG){    
				for(uint32_t ind_oc = 0; ind_oc < VEC_SIZE( CL_REF, occurences_pos[unit_var1] ); ++ind_oc){
					CL_REF clause = occurences_pos[unit_var1][ind_oc];
					CLAUSE& cl = gsa.get(clause);
					if(CL_IS_IGNORED( cl )) continue;
					if(CL_SIZE( cl ) == 1){
						std::cerr << "UNSAT during propagate 2" << std::endl;
						return UNSAT;
					}
			
					CL_DELETE_LIT( cl, inv_lit(unit_lit1) );
			
					if(CL_SIZE( cl ) == 1){
						VEC_PUSH_BACK( lit_t, (*unit_queue), CL_GET_LIT( cl, 0 ));
						VEC_PUSH_BACK( lit_t, (*remind_unit_queue), CL_GET_LIT( cl, 0 ));
						CL_IGNORE_AND_DELETE( cl );
						(*clause_cnt)--;
					
					}
					else
						for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
							boolarray_set( touched_variable,var(CL_GET_LIT( cl, ind_clause )), true);
			

				}
				for(uint32_t ind_oc = 0; ind_oc < VEC_SIZE( CL_REF, occurences_neg[unit_var1] ); ++ind_oc){
					CL_REF clause = occurences_neg[unit_var1][ind_oc];
					CLAUSE& cl = gsa.get(clause);
					if(CL_IS_IGNORED( cl )) continue;
					remove_clause_from_occurrences(clause);
					for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
						boolarray_set( touched_variable, var(CL_GET_LIT( cl, ind_clause )), true);
					CL_IGNORE_AND_DELETE( cl );
					(*clause_cnt)--;
			

				}

      }
      else{
				for(uint32_t ind_oc = 0; ind_oc < VEC_SIZE( CL_REF, occurences_neg[unit_var1] ); ++ind_oc){
					CL_REF clause = occurences_neg[unit_var1][ind_oc];
					CLAUSE& cl = gsa.get(clause);
					if(CL_IS_IGNORED( cl )) continue;
					if(CL_SIZE( cl ) == 1)
					{
						std::cerr << "unsat during propagate 3" << std::endl;
						return UNSAT;
					}
					CL_DELETE_LIT( cl, inv_lit(unit_lit1) );
					if(CL_SIZE( cl ) == 1){
						VEC_PUSH_BACK( lit_t, (*unit_queue), CL_GET_LIT( cl, 0 ));
						VEC_PUSH_BACK( lit_t, (*remind_unit_queue), CL_GET_LIT( cl, 0 ));
						CL_IGNORE_AND_DELETE( cl );
						(*clause_cnt)--;
					}
					else
						for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
							boolarray_set( touched_variable, var(CL_GET_LIT( cl, ind_clause )), true);

				}
				for(uint32_t ind_oc = 0; ind_oc < VEC_SIZE( CL_REF, occurences_pos[unit_var1] ); ++ind_oc){
					CL_REF clause = occurences_pos[unit_var1][ind_oc];
					CLAUSE& cl = gsa.get(clause);
					if(CL_IS_IGNORED( cl )) continue;
					remove_clause_from_occurrences(clause);
					for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
						boolarray_set( touched_variable, var(CL_GET_LIT( cl, ind_clause )), true);
					CL_IGNORE_AND_DELETE( cl );
					(*clause_cnt)--;

				}

      }
	
      
    }
    return UNKNOWN;
}

void SatellikePreprocessor::init(VEC_TYPE(CL_REF ) *cls, 
				 uint32_t *clause_count, 
				 uint32_t *var1_count){
  

  
  clause_cnt = *clause_count;
  var_cnt = *var1_count;
  clauses = cls;
  var_cnt_postprocess = var_cnt;

  result = UNKNOWN;

  
  VEC_CREATE_SPACE( VEC_TYPE( CL_REF ), occurences_pos, var_cnt + 1 );
  VEC_CREATE_SPACE( VEC_TYPE( CL_REF ), occurences_neg, var_cnt + 1 );
	
	VEC_CLEAR( VEC_TYPE( CL_REF ), occurences_pos);
	VEC_CLEAR( VEC_TYPE( CL_REF ), occurences_neg);

  for(uint32_t ind_oc = 0; ind_oc < var_cnt + 1; ++ind_oc){
  	VEC_TYPE( CL_REF ) tmp;
  	VEC_CREATE( CL_REF, tmp );
  	VEC_PUSH_BACK(	VEC_TYPE( CL_REF ), occurences_pos, tmp );
  	
  	VEC_CREATE( CL_REF, tmp );
  	VEC_PUSH_BACK(	VEC_TYPE( CL_REF ), occurences_neg, tmp );
  }
  
  var_occurrences = new uint32_t[var_cnt + 1];
  
  for(uint32_t ind_occ = 0; ind_occ < var_cnt+1; ++ind_occ)
    var_occurrences[ind_occ] = 0;


	VEC_CREATE( lit_t, unit_queue );
	
	
  
  
  
  for(uint32_t ind_clauses = 0; ind_clauses < clause_cnt; ++ind_clauses)
  {
  	CLAUSE& cl = gsa.get( (*clauses)[ind_clauses] );
    for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause){
      
      if( CL_SIZE( cl ) < 2)
				VEC_PUSH_BACK( lit_t, unit_queue, CL_GET_LIT( cl,0) );

      if (pol( CL_GET_LIT( cl, ind_clause) ) == POS){
      	VEC_PUSH_BACK( CL_REF, occurences_pos[ var( CL_GET_LIT( cl, ind_clause) ) ], (*clauses)[ind_clauses] );
      }
      else {
      	VEC_PUSH_BACK( CL_REF, occurences_neg[ var( CL_GET_LIT( cl, ind_clause) ) ], (*clauses)[ind_clauses] );
      }

      var_occurrences[var( CL_GET_LIT( cl,ind_clause) ) ] ++;
    }
	}

  
  VEC_CREATE( CL_REF, subsumption_queue );
  VEC_RESERVE( CL_REF, subsumption_queue, var_cnt+1);
    
  
  touched_variable = boolarray_create( var_cnt + 1 );
  for(uint32_t ind_clauses = 0; ind_clauses < VEC_SIZE( CL_REF, (*clauses) ); ++ind_clauses){
		
  	VEC_PUSH_BACK( CL_REF, subsumption_queue, (*clauses)[ind_clauses] );
  	
  	
  	CLAUSE& cl = gsa.get( (*clauses)[ind_clauses] );
    for(uint32_t ind_clause = 0; ind_clause < CL_SIZE( cl ); ++ind_clause)
      boolarray_set( touched_variable, var( CL_GET_LIT( cl,ind_clause) ), true);
      
	}


  
  resolution_queue = cpp_heap<uint32_t, std::less<uint32_t> >( &var_occurrences, var_cnt + 1);
  for(var_t ind_var1 = 1; ind_var1 < var_cnt+1; ++ind_var1)
    resolution_queue.insert_item(ind_var1);  
  
}

void SatellikePreprocessor::postprocess(  VEC_TYPE(assi_t)* assignments){

	VEC_TYPE(lit_t) model;

	
	VEC_CREATE( lit_t, model );

	for( uint32_t i = 0 ; i < VEC_SIZE( assi_t, (*assignments) ); ++i ){

		for( int32_t ind = resolution_queue_postprocess.size() - 1 ; ind>=0; --ind ){
			const elimination_t& elimination = resolution_queue_postprocess[ind];
			
			
			if( ! assi_is_undef( (*assignments)[i], elimination.var1 ) ) continue;
			
			uint32_t k = 0;
			for (; k < elimination.clauses.size(); ++k) {
				uint32_t ind;
				lit_t literal = NO_LIT;
				CLAUSE & cl = gsa.get (elimination.clauses[k]);
				for (ind = 0; ind < CL_SIZE (cl); ++ind) {
					const lit_t literal_c = CL_GET_LIT (cl, ind);
					const var_t variable_c = var (literal_c);
					
					if (variable_c == elimination.var1)
						literal = literal_c;
					else if (assi_is_sat ( (*assignments)[i], literal_c))
						break;
				}
				
				if (ind == CL_SIZE (cl)) {
					assi_set_polarity ( (*assignments)[i] , var (literal), pol (literal));
					break;
				}
			}
			if( k == elimination.clauses.size() ){
				
				if( max_solutions == -1 || max_solutions > (int32_t)VEC_SIZE( assi_t, (*assignments) ) ){
					assi_t tmp = assi_copy( (*assignments)[i], var_cnt );
					assi_set_polarity( tmp, elimination.var1, NEG );
					VEC_PUSH_BACK(assi_t, (*assignments), tmp );
				}
				assi_set_polarity ( (*assignments)[i] , elimination.var1, POS );
			}
		
		}
  
  }

}


void SatellikePreprocessor::postprocessSingle( assi_t assignment)
{

}

solution_t SatellikePreprocessor::simplifysearch (searchData& sd, VEC_TYPE (CL_REF)& clauses, VEC_TYPE (CL_REF)& learnt)
{
	return UNKNOWN;
}

void SatellikePreprocessor::set_parameter( const StringMap& commandline )
{
	max_solutions = 0;

	
	if( commandline.contains( (const char*)"sol" ) != 0 )  max_solutions = atoi( commandline.get( (const char*)"sol" ).c_str() );
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"preprocessor_mode" ) ) mode = atoi( commandline.get( (const char*)"preprocessor_mode" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_print_clauses" ) ) print_dimacs = 0 != atoi( commandline.get( (const char*)"preprocessor_print_clauses" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_run_ve" ) ) run_ve = 0 != atoi( commandline.get( (const char*)"preprocessor_run_ve" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_blocked" ) ) process_blocked = 0 != atoi( commandline.get( (const char*)"preprocessor_blocked" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_block_max_cls_size" ) ) block_max_cls_size = atoi( commandline.get( (const char*)"preprocessor_block_max_cls_size" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_block_max_occs" ) ) block_max_occs = atoi( commandline.get( (const char*)"preprocessor_block_max_cls_size" ).c_str() );
	if( commandline.contains( (const char*)"preprocessor_run_pure" ) ) run_pure = 0 != atoi( commandline.get( (const char*)"preprocessor_run_pure" ).c_str() );	
	if( commandline.contains( (const char*)"preprocessor_max_cls" ) ) max_cls = atoi( commandline.get( (const char*)"preprocessor_max_cls" ).c_str() );
	if( mode == 3 ) process_blocked = true;
	
	blackFile = commandline.get( (const char*)"preprocessor_blackFile" );
	whiteFile = commandline.get( (const char*)"preprocessor_whiteFile" );
#endif

	
	if( max_solutions != 0 ) run_pure = false;

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== satellike preprocessor information ===" << std::endl;
		std::cerr << " parameter                       values info" << std::endl;
		std::cerr << " preprocessor_mode               0 - 3  mode of preprocessing (0=satelit1e,3=precosat bebe)" << std::endl;
		std::cerr << " preprocessor_print_clauses      0,1    print preprocessresult and quit" << std::endl;
		std::cerr << " preprocessor_run_ve             0,1    run ve" << std::endl;
		std::cerr << " preprocessor_run_pure           0,1    run pure" << std::endl;
		std::cerr << " preprocessor_blocked            0,1    process blocked clauses before" << std::endl;
		std::cerr << " preprocessor_block_max_cls_size 0-n    max cls size to check for blocking" << std::endl;
		std::cerr << " preprocessor_block_max_occs     0-n    max lit1 occs to check for blocking" << std::endl;
		std::cerr << " preprocessor_max_cls            1-n    do not preprocess if formula contains more clss" << std::endl;
		std::cerr << " preprocessor_whiteFile name  name of variable-file of variables that must be eliminated" << std::endl;
		std::cerr << " preprocessor_blackFile          name   name of variable-file of variables that must be eliminated" << std::endl;
		std::cerr << std::endl;
	}	
#endif
#endif

	if( whiteFile.size() != 0 ){
		VarFileParser vfp( whiteFile );
		VEC_TYPE(var_t) evars;
		VEC_CREATE(var_t,evars);
		vfp.extract(evars);
		for( uint32_t i = 0; i < VEC_SIZE(var_t, evars); ++i ){
			if( evars[i] <= var_cnt ) boolarray_set( elimWhite, evars[i], true);
		}		
		
	}
}


#ifdef COMPILE_LIBRARY

extern "C" SatellikePreprocessor* create_preprocessor(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline ){
	return new SatellikePreprocessor( clause_set, var_cnt, commandline );
}

extern "C" void destroy_preprocessor( SatellikePreprocessor* heuristic ){
	delete heuristic;
}
#endif
