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



#include "decisionheuristic/var_activity_heuristic.h"


#include <iostream>


VariableActivityHeuristic::VariableActivityHeuristic(VEC_TYPE( CL_REF )* clause_set, searchData& sd, const StringMap& commandline):
	keyboard( false ),
	minisat_rnd ( true),
	remove ( true),	
	decay ( 1/0.85), 
	increment ( 1),	
	max_increment ( 1e100),
	random_desicion_freq ( 2),
	polarity_rate (0),
	inc_learn_literals ( true),
	inc_clause_literals ( false),
	cc_lits_mode( 0),
	learnedJW(true),
	vmtf(false),
	useDependendP(0),
	polarity_mode (0),	
	phase_backup_mode(3),
	tabu_size(0),
	threshold_size ( 2),	
	occurence_plus ( 0),
	jw(0),
	dependendOf(0)
{
	
	var1_count = sd.var_cnt;
	
	
	decision_timer = new uint32_t[sd.var_cnt + 1];
	memset( decision_timer, 0, sizeof(uint32_t) * (sd.var_cnt+1) );
	timer = tabu_size + 1;
	tabu_accesses = 0;

	
	set_parameter( commandline );

	
	decisions = 1;
	decisions_neg = 0;

	
	if( polarity_mode == 3 || phase_backup_mode == 3){
		jw = (float*) MALLOC( sizeof(float) * max_index(sd.var_cnt) );
		memset( jw, 0, sizeof(float) * max_index(sd.var_cnt) );
	}

	
	if( useDependendP != 0 ){
		dependendOf = (lit_t*) MALLOC( sizeof(lit_t) * max_index(sd.var_cnt) );
		memset( dependendOf, 0, sizeof(lit_t) * max_index(sd.var_cnt) );
	}

	
	realVars = 0;

	
	if( clause_set != 0 ){
		
		unsigned char implies [max_index(sd.var_cnt)];
		memset( implies, 0, sizeof(unsigned char) * max_index(sd.var_cnt) );

		bool present [var1_count];
		for( uint32_t v = 0; v <= var1_count; ++v ){
			present[v] = 0;
		}

		
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, (*clause_set) ); ++i ){
			CLAUSE& cl = gsa.get( (*clause_set)[i] );
			const uint32_t size = CL_SIZE( cl );
			if( jw != 0){	
				float thisexp = 1.0 / (float) pow(2, (float)size );
				for( uint32_t j = 0; j < size; ++j ){
					jw[ index(CL_GET_LIT( cl, j )) ] += thisexp;
				}
			}
			
			
			for( uint32_t j = 0; j < size; ++j ){
				if( present[ var(CL_GET_LIT( cl, j )) ] == 0 ){
					present[ var(CL_GET_LIT( cl, j )) ] = 1;
					realVars ++;
				}
			}

			
			
			
			if( size == 2  && useDependendP != 0 ){
				if( implies[ index( inv_lit( CL_GET_LIT(cl,0) ) ) ] < 253 ) implies[ index( inv_lit( CL_GET_LIT(cl,0) ) ) ]++;
				if( implies[ index( inv_lit( CL_GET_LIT(cl,1) ) ) ] < 253 ) implies[ index( inv_lit( CL_GET_LIT(cl,1) ) ) ]++;
			}
			
			if( size > threshold_size ) continue;
		
			
			for( uint32_t j = 0; j < size; ++j ){
				
				sd.activities[ var(CL_GET_LIT( cl, j ) ) ] += occurence_plus / (double)size;
			}
		}
		
		
		if( useDependendP != 0){
		
			for( uint32_t j = 0 ; j < 2; j ++ ){
				for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, (*clause_set) ); ++i ){
					CLAUSE& cl = gsa.get( (*clause_set)[i] );
					const uint32_t size = CL_SIZE( cl );
					if( size != 2 ) continue;
				
					const lit_t l1 = CL_GET_LIT( cl,0 );
					const lit_t l2 = CL_GET_LIT( cl,1 );
					
					if( implies[ index( inv_lit( l2 ) ) ] > implies[ index( dependendOf[ index(l1) ] ) ] ) dependendOf[ index(l1) ] = inv_lit( l2 );
					if( implies[ index( inv_lit( l1 ) ) ] > implies[ index( dependendOf[ index(l2) ] ) ] ) dependendOf[ index(l2) ] = inv_lit( l1 );
				}
			}
		}
		
	}

	
	sorting = HEAP_CREATE(double, sd.activities, ( sd.var_cnt + 1) );
	
	
	for (var_t var1 = 1; var1 < var1_count+1; var1 ++) {
		
		if( !HEAP_CONTAINS(double,  sorting, var1 ) && !boolarray_get(sd.eliminated, var1 ) )
			HEAP_INSERT_ITEM(double,  sorting, var1 );
	}
	
	assignMe = STACK_CREATE( lit_t);
}



VariableActivityHeuristic::~VariableActivityHeuristic()
{
	STACK_DESTROY( lit_t, assignMe)
	HEAP_DESTROY(double, sorting );
	
	delete [] decision_timer;
	
	if( jw != 0 ) free(jw);
#ifndef COMPETITION
#ifndef SILENT
	
#endif
#endif
}




lit_t VariableActivityHeuristic::get_literal( searchData& search)
{
	
	if( STACK_SIZE(lit_t, assignMe ) > 0 ){
		lit_t top = NO_LIT;
		
		while( STACK_SIZE(lit_t, assignMe ) > 0 ){
			top = STACK_TOP( lit_t, assignMe );
			STACK_POP( lit_t, assignMe );
			if( assi_is_undef( search.assi, var(top) ) ) return top;
		}
	}

	
	if( keyboard ){
		int32_t number = 0;
		lit_t top;
		do {
			cerr << "c input the next decision literal: ";
			cin >> number;
			top = lit( number );
		} while( ! assi_is_undef( search.assi, var(top) ) );
		
		if( top == 0 ){
			keyboard = false;
		} else return top;
	}

	pol_t polarity;
	var_t var1;
	lit_t lit1;
	
	
	var1 = NO_VAR;

	
	if(minisat_rnd){	
		
		if ( (uint32_t)(random() % 100) <= random_desicion_freq && !HEAP_IS_EMPTY(double,  sorting ) ){
			uint32_t ind = random() % HEAP_SIZE(double,  sorting );
			var1 = HEAP_ITEM_AT( double, sorting, ind );
			if ( !assi_variable_equal(search.assi, var1, UNDEF) ) var1=NO_VAR;
		}
	} else {
		if (decisions % random_desicion_freq == 0) {
			unsigned tries;
			
			tries = 10;

			while (tries--) {
				
				var1 = (random() % var1_count) + 1;
			
				
				if ( assi_variable_equal(search.assi, var1, UNDEF) ) break;
				else var1 = NO_VAR;
			}
		}
	}

	
	if( var1 != NO_VAR && boolarray_get(search.eliminated, var1 ) ) var1 = NO_VAR;

	
	if (remove)
	{
		uint32_t ind = 0;
		if( tabu_size > 0 ){
			while ( var1 == NO_VAR && HEAP_SIZE(double,sorting) > tabu_size ) {
				
				var1 = HEAP_ITEM_AT(double,  sorting, ind);
				
				if( decision_timer[var1] + tabu_size >= timer ) { ind ++; tabu_accesses++;continue; }

				
				HEAP_REMOVE_ITEM(double,  sorting, var1);
				
				if ( !assi_variable_equal(search.assi, var1, UNDEF) || boolarray_get(search.eliminated, var1 )  ){
					var1 = NO_VAR;
				}
			}
		}
		
		while( var1 == NO_VAR && !HEAP_IS_EMPTY(double,  sorting ) ){
			var1 = HEAP_ITEM_AT(double,  sorting, 0);
			HEAP_REMOVE_ITEM(double,  sorting, var1);
			if ( !assi_variable_equal(search.assi, var1, UNDEF) ){
				var1 = NO_VAR;
			}
		}
	}
	else
	{
		unsigned i = 0;
		while (var1 == NO_VAR && i<HEAP_SIZE(double,  sorting ) ) {
			
			var1 = HEAP_ITEM_AT(double, sorting, i++);
			
			if ( !assi_variable_equal(search.assi, var1, UNDEF) ) var1 = NO_VAR;
		}
	}

	
	if (var1 == NO_VAR)
		return NO_LIT;

	
	
	switch( polarity_mode )
	{
		
		case 0: polarity = assi_get_backup_polarity( search.assi, var1);
				
				
				if( polarity != UNDEF ) break;
				switch(phase_backup_mode){
					case 1: polarity = ( (((float)decisions_neg) / (float)decisions) >= polarity_rate ) ? NEG : POS;
							break;
					case 2:	polarity = (pol_t)( (rand() % 2) + 1 );
							break;
					case 3: polarity = (jw[ index( lit(var1,POS))] > jw[ index( lit(var1,NEG))]) ? POS : NEG;
							break;
				}
				break;
		
		case 1: polarity = ( (((float)decisions_neg) / (float)decisions) >= polarity_rate ) ? NEG : POS;
						break;
		
		case 2:	polarity = (pol_t)( (rand() % 2) + 1 );
						break;
		
		case 3: polarity = jw[ index( lit(var1,POS))] > jw[ index( lit(var1,NEG))] ? POS : NEG;
			break;
		default: assert( 0 == 1 );
						polarity = NEG;
						break;
	}
	
	lit1 = lit(var1, polarity);

	
	decision_timer[ var( lit1 ) ] = timer;
	if( ++timer == 0xFFFFFFFF ){
		timer = tabu_size + 2;
		memset( decision_timer, 0, var1_count * sizeof(uint32_t) );
	}

	if( useDependendP != 0 ){
		uint32_t iter = useDependendP;
		if( useDependendP < 0 ) {
			double max = 0.0 - useDependendP;
			double ts = STACK_SIZE( lit_t, search.trail );
			double rv = realVars;
			
			double x = ts / rv;
			iter = (int32_t)( (x > 0.25 && x < 0.75 ) ? 2*max*x : 0);
			iter = max - iter;
			
		}
	
		lit_t original = lit1;
		
		for( uint32_t i = iter ; i > 0; --i ){
			const lit_t tmp = dependendOf[original];
			if( tmp != NO_LIT && assi_is_undef(search.assi, var(tmp)) ){
				
				if( remove && !HEAP_CONTAINS(double,  sorting, var(original) ) ) HEAP_INSERT_ITEM(double,  sorting, var(original) );
				original = tmp;
				polarity = pol(original);
			} else break;
		}
		lit1 = original;
	}

	
	decisions++;
	decisions_neg += (polarity == NEG);

	return lit1;
}




void VariableActivityHeuristic::literals_conflicted(  searchData& sd, VEC_TYPE(lit_t)& lits )
{
	
	if( inc_learn_literals )
	{
		for( uint32_t ind = 0 ; ind < VEC_SIZE( lit_t, lits ); ind++ )
		{
			
			const var_t var1 = var( lits[ind] );

			
			sd.activities[var1] = (!vmtf) ? sd.activities[var1] + increment : increment;
	
			if( HEAP_CONTAINS(double,  sorting, var1 ) ) 
				HEAP_UPDATE_ITEM(double,  sorting, var1 );

			
			
			if ( sd.activities[var1] > max_increment) {
				
				for (uint32_t act = 1; act < var1_count + 1; act ++) { 
					sd.activities[act] /= max_increment;
				}
				increment /= max_increment;
			}
		}
	}
}


void VariableActivityHeuristic::clauses_learned( const searchData& sd, VEC_TYPE(CL_REF)& clauses)
{
	
	
	if( inc_clause_literals )
	{
		for( uint32_t ind = 0 ; ind < VEC_SIZE( CL_REF, clauses ); ind++ )
		{
			CLAUSE& cl = gsa.get(clauses[ind]);
			const uint32_t size =  CL_SIZE( cl );
			for( uint32_t i=0; i < size; ++i )
			{
				
				const lit_t lit1 = CL_GET_LIT( cl, i );
				
				const var_t var1 = var(lit1);

				
				sd.activities[var1] += increment;
	
				if( HEAP_CONTAINS(double,  sorting, var1 ) ) 
					HEAP_UPDATE_ITEM(double,  sorting, var1 );

				if( learnedJW && jw != 0 ) jw[ index(lit1) ] += (float) pow(2, - size );

				
				
				if ( sd.activities[var1] > max_increment) {
					
					for (uint32_t act = 1; act < var1_count + 1; act ++) { 
						sd.activities[act] /= max_increment;
					}
					increment /= max_increment;
				}			
			}
		}
	}	
	

	fillAssignStack( sd, clauses );
	
	
	increment *= decay;
}

void VariableActivityHeuristic::fillAssignStack(  const searchData& sd, VEC_TYPE(CL_REF)& clauses)
{
	STACK_CLEAR( lit_t, assignMe );
	
	
	if( cc_lits_mode == 0 ) return;
	
	uint32_t lits = 0;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); i++ ){
		lits+=CL_SIZE( gsa.get(clauses[i]) );
	}
	
	lit_t literals[lits];
	uint32_t count = 0;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); i++ ){
		const CLAUSE& cl = gsa.get(clauses[i] );
		for( uint32_t j = 0; j < CL_SIZE( cl ); ++j ) literals[count++] = CL_GET_LIT( cl,j);
	}
	
	
	if( cc_lits_mode == 1 ){
		for( uint32_t i = 0 ; i < lits; ++i ){	
			uint32_t p = i;
			for( uint32_t j = i+1 ; j < lits; ++j ){
				if( sd.VAR_LEVEL( var( literals[j] ) ) < sd.VAR_LEVEL( var( literals[p] ) ) ){
					p = j;					
				}
			}
			STACK_PUSH(lit_t, assignMe, inv_lit(literals[p]) );
			literals[p] = literals[i];
		}
	}
	if( cc_lits_mode == 2 ){
		for( uint32_t i = 0 ; i < lits; ++i ){	
			uint32_t p = i;
			for( uint32_t j = i+1 ; j < lits; ++j ){
				if( sd.VAR_LEVEL( var( literals[j] ) ) > sd.VAR_LEVEL( var( literals[p] ) ) ){
					p = j;					
				}
			}
			STACK_PUSH(lit_t, assignMe, inv_lit(literals[p]) );
			literals[p] = literals[i];
		}
	}
	if( cc_lits_mode == 3 ){
		for( uint32_t i = 0 ; i < lits; ++i ){	
			uint32_t p = i;
			for( uint32_t j = i+1 ; j < lits; ++j ){
				if( sd.activities[var( literals[j] ) ] > sd.activities[ var( literals[p] ) ] ){
					p = j;					
				}
			}
			STACK_PUSH(lit_t, assignMe, inv_lit(literals[p]) );
			literals[p] = literals[i];
		}
	}
}

 



void VariableActivityHeuristic::variables_unassigned( VEC_TYPE(var_t)& var1s)
{
	for( uint32_t i = 0 ; i < VEC_SIZE( var_t, var1s ); ++i )
	{
		
		if( !HEAP_CONTAINS(double,  sorting, var1s[i] ) )
			HEAP_INSERT_ITEM(double,  sorting, var1s[i] );
	}
}

void VariableActivityHeuristic::set_parameter( const StringMap& commandline )
{

#ifdef USE_COMMANDLINEPARAMETER
	
	if( commandline.contains( (const char*)"var_act_minisat_rnd" ) ) minisat_rnd = 0 != atoi( commandline.get( (const char*)"var_act_minisat_rnd" ).c_str() );
	if( commandline.contains( (const char*)"var_act_remove" ) ) remove = 0 != atoi( commandline.get( (const char*)"var_act_remove" ).c_str() );
	if( commandline.contains( (const char*)"var_act_decay_divisor" ) ) decay = 1.0 / atof( commandline.get( (const char*)"var_act_decay_divisor" ).c_str() );
	if( commandline.contains( (const char*)"var_act_rand_freq" ) ) random_desicion_freq = atoi( commandline.get( (const char*)"var_act_rand_freq" ).c_str() );
	if( commandline.contains( (const char*)"var_act_pol_rate" ) ) polarity_rate = atof( commandline.get( (const char*)"var_act_pol_rate" ).c_str() );
	if( commandline.contains( (const char*)"var_act_pol_mode" ) ) polarity_mode = atoi( commandline.get( (const char*)"var_act_pol_mode" ).c_str() );
	if( commandline.contains( (const char*)"var_act_pol_backup_mode" ) ) phase_backup_mode = atoi( commandline.get( (const char*)"var_act_pol_backup_mode" ).c_str() );
	if( commandline.contains( (const char*)"var_act_min_init_size" ) ) threshold_size = atoi( commandline.get( (const char*)"var_act_min_init_size" ).c_str() );
	if( commandline.contains( (const char*)"var_act_init_activity" ) ) occurence_plus = atof( commandline.get( (const char*)"var_act_init_activity" ).c_str() );
	if( commandline.contains( (const char*)"var_act_increment" ) ) increment = atof( commandline.get( (const char*)"var_act_increment" ).c_str() );
	if( commandline.contains( (const char*)"var_act_inc_lits" ) ) inc_learn_literals = 0 != atoi( commandline.get( (const char*)"var_act_inc_lits" ).c_str() );
	if( commandline.contains( (const char*)"var_act_inc_clause" ) ) inc_clause_literals = 0 != atoi( commandline.get( (const char*)"var_act_inc_clause" ).c_str() );
	if( commandline.contains( (const char*)"var_act_tabu_size" ) ) tabu_size = atoi( commandline.get( (const char*)"var_act_tabu_size" ).c_str() );
	if( commandline.contains( (const char*)"var_act_cc_mode" ) ) cc_lits_mode = atoi( commandline.get( (const char*)"var_act_cc_mode" ).c_str() );
	if( commandline.contains( (const char*)"var_act_learnedJW" ) ) learnedJW = 0 != atoi( commandline.get( (const char*)"var_act_learnedJW" ).c_str() );
	if( commandline.contains( (const char*)"var_act_vmtf" ) ) vmtf = 0 != atoi( commandline.get( (const char*)"var_act_vmtf" ).c_str() );
	if( commandline.contains( (const char*)"var_act_key" ) ) keyboard = 0 != atoi( commandline.get( (const char*)"var_act_key" ).c_str() );
	if( commandline.contains( (const char*)"var_act_depend" ) ) useDependendP = atoi( commandline.get( (const char*)"var_act_depend" ).c_str() );
#endif
	
#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== variable activity heuristic information ===" << std::endl;
		std::cerr << " parameter               pos.value info" << std::endl;
		std::cerr << " var_act_minisat_rnd     0,1       1 .. random decision like minisat" << std::endl;
		std::cerr << " var_act_remove          0,1       use removing in heap, if not given, it is not used" << std::endl;
		std::cerr << " var_act_decay_divisor   0 - n     1/X is the decay for every conflict" << std::endl;
		std::cerr << " var_act_rand_freq       1 - n     do a random decision every X decisions (minisat->percentage value)" << std::endl;
		std::cerr << " var_act_pol_mode        0,1,2,3   how to get the polarity of the decision variable" << std::endl;
		std::cerr << "                         0         get last polarity, if there is one, else polarity_rate" << std::endl;
		std::cerr << "                         1         try to stick to the ration between pos and negative" << std::endl;
		std::cerr << "                         2         random" << std::endl;
		std::cerr << "                         4		 jeroslaw-wang" << std::endl;
		std::cerr << " var_act_pol_backup_mode 1,2,3     method to apply, if no phase is present" << std::endl;
		std::cerr << " var_act_pol_rate        0 - 1     ration between positive and all decisions" << std::endl;
		std::cerr << " var_act_increment       0 - n     activity value increase for new literals after a conflict" << std::endl;
		std::cerr << " var_act_min_init_size   0 - n     clauses of size less than this are considered for init" << std::endl;
		std::cerr << " var_act_init_activity   0 - n     activity for occurence in an init clause" << std::endl;
		std::cerr << " var_act_inc_lits        0,1       inc activity of literals used while analyzing conflict" << std::endl;
		std::cerr << " var_act_inc_clause      0,1       inc activity of literals in the learned clause" << std::endl;
		std::cerr << " var_act_tabu_size       0-n       number of decisions before a variable can be chosen again" << std::endl;
		std::cerr << " var_act_cc_mode         0-3       how to handle literals of last conflict clause (assignment shrinking)" << std::endl;
		std::cerr << "                                   0 do nothing" << std::endl;
		std::cerr << "                                   1 assign to false in order of their levels" << std::endl;
		std::cerr << "                                   2 assign to false in reverse level order" << std::endl;
		std::cerr << "                                   3 assign to false, order according to activity" << std::endl;
		std::cerr << " var_act_key             0,1       use the keyboard to pick literals" << std::endl;
		std::cerr << " var_act_depend          -n-n      return not the chosen literal, but a literal where this literal depends on" << std::endl;
		std::cerr << "                                   positive: number of max. possible recursions" << std::endl;
		std::cerr << "                                   negative: dynamic selection, max. iterations" << std::endl;
		std::cerr << " var_act_vmtf            0,1       do not consider previous activity, but only new (imitate vmtf)" << std::endl;
		
		std::cerr << std::endl;
	}
#endif
#endif
}

#ifdef COMPILE_LIBRARY

extern "C" VariableActivityHeuristic* create_decisionheuristic(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline){
	return new VariableActivityHeuristic( clause_set, var_cnt, commandline );
}

extern "C" void destroy_decisionheuristic( VariableActivityHeuristic* decisionheuristic ){
	delete decisionheuristic;
}
#endif
