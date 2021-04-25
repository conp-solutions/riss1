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


#include <iostream>

#include "analyze/cdclminanalyze.h"

CdclMinimizeAnalyzer::CdclMinimizeAnalyzer( uint32_t var_cnt, const StringMap& commandline ) :
	minimize( 3 ),
	add_trivail_short ( false ),
	new_level_limit (1),
	prefetch_reasons( 5),
	dump_conflict_graphs( false),
	analysis_cnt( 0)
{
	
	inserted_variable = boolarray_create( var_cnt + 1 );
	set_parameter( commandline );
	VEC_CREATE_SPACE(lit_t, literals, 100 );
	VEC_CREATE_SPACE(lit_t, clear, 100 );
	VEC_CREATE_SPACE(lit_t, stack, 100 );
}


CdclMinimizeAnalyzer::~CdclMinimizeAnalyzer(){
		boolarray_destroy( inserted_variable );
		VEC_DESTROY(lit_t, literals );
		VEC_DESTROY(lit_t, clear );
		VEC_DESTROY(lit_t, stack );
		VEC_DESTROY(CL_REF, otfClauses );
	}

void CdclMinimizeAnalyzer::analyze(searchData& search, VEC_TYPE(CL_REF)& used_clauses, VEC_TYPE(conflictStruct)& conflict_clauses, VEC_TYPE(CL_REF)& learned_clauses, VEC_TYPE(lit_t)& used_literals, VEC_TYPE(CL_REF)& otfClauses )
{
	uint32_t current_level = search.current_level;
	int32_t min_level = current_level;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( reasonStruct,conflict_clauses ); i++ )
	{
		VEC_PUSH_BACK( CL_REF, learned_clauses, analyze( search, used_clauses, conflict_clauses[i], used_literals, otfClauses ) );
		min_level = ( min_level <= search.current_level ) ? min_level : search.current_level;
		
		search.current_level = current_level;
	}
	
	search.current_level = min_level;
}

CL_REF CdclMinimizeAnalyzer::analyze(searchData& search, VEC_TYPE(CL_REF)& used_clauses, conflictStruct conflict_clause, VEC_TYPE(lit_t)& used_literals, VEC_TYPE(CL_REF)& otfs )
{

	uint64_t level_hash;
	reasonStruct reason;
	uint32_t i, j;
	
	#ifdef USE_CONFLICT_PREFETCHING
	if( !conflict_clause.isLit() ){
		CLAUSE& conflict = gsa.get(conflict_clause.getCL());
		
			uint32_t min = (prefetch_reasons < CL_SIZE( conflict )) ? prefetch_reasons : CL_SIZE( conflict );
			for( uint32_t i = 0 ; i < min ; ++i ){
				if( !search.VAR_REASON( var( CL_GET_LIT( conflict, i ) ) ).isLit() ) prefetch( gsa.addr( search.VAR_REASON( var( CL_GET_LIT( conflict, i ) ) ).getCL() ) );
			}
		
	}
	#endif
	
	
	analysis_cnt++;
	if(dump_conflict_graphs && !conflict_clause.isLit() ) conflictDumper.dump( analysis_cnt, conflict_clause.getCL(), search );

	
	if( false ){
	
		for( uint32_t ti = 0 ; ti < STACK_SIZE( lit_t, search.trail ); ++ ti ){
			const lit_t l = search.trail[ti];
			const var_t v = var(l);
			const int32_t lev = search.VAR_LEVEL(v);
			const reasonStruct r = search.VAR_REASON(v);
			if( !assi_is_sat( search.assi, l ) ) cerr << "c not satisfied literal " << nr(l) << " on trail at " << lev << endl;
			if( r.empty() ){
				if( ti == 0 ) continue;
				if( search.VAR_LEVEL( var(search.trail[ti] ) ) != search.VAR_LEVEL( var(search.trail[ti-1] ) ) +1 ) cerr << "c wrong level information for literal " << nr(l) << " with empty reason at " << lev << endl;
				continue;
			}
			if( r.isLit() ){
				if( !assi_is_unsat( search.assi, r.getLit() ) ) cerr << "c not unsatisfied reason literal " << nr( r.getLit() ) << " for literal " << nr(l) << " on trail at " << lev << endl;
			} else {
				CLAUSE& cl = gsa.get( r.getCL());
				uint32_t tj = 0 ;
				for( ; tj < CL_SIZE(cl); tj++ ){
					if( l == CL_GET_LIT(cl,tj) ) continue;
					if( !assi_is_unsat( search.assi, CL_GET_LIT(cl,tj) ) ){  cerr << "c not unsatisfied literal " << nr( CL_GET_LIT(cl,tj) ) << " in rason clause for literal " << nr(l) << " on trail at " << lev << endl; break; }
				}
				if( tj != CL_SIZE(cl) ) { cerr << "c full clause: "; printClause(  search, r.getCL() ); cerr << endl; }
				if( CL_SIZE(cl) == 1 && lev != 0 ) cerr << "c syntactical unit found at higher level as reason clause " << endl;
			}
		}
	}


		
#ifdef OTFSS
	uint32_t topLLits = 0;
#endif
	lit_t asserting = NO_LIT;
	var_t asserting_var1 = NO_VAR;
	uint32_t same_level = 0;
	uint32_t trail_ind = STACK_SIZE(lit_t, search.trail ) - 1 ;
	
	
	
	if( !conflict_clause.isLit() ){
		reason = reasonStruct(conflict_clause.getCL());
		
	}
	else {
		
		for (i = 0; i < 2; i++) {
			const lit_t lit1 = conflict_clause.getLit(i);
			if (lit1 == asserting) continue;
			const var_t var1 = var(lit1);
			const int32_t level = search.VAR_LEVEL(var1);
			if ((! boolarray_get(inserted_variable,var1) ) && (level > 0)){
				
				VEC_PUSH_BACK( lit_t, used_literals, lit1 );
				boolarray_set( inserted_variable,var1, true);
				if (level >= search.current_level ){
					#ifdef USE_SAME_LEVEL_PREFETCHING
						if( !search.VAR_REASON( var1 ).isLit() ) prefetch( gsa.addr( search.VAR_REASON( var1 ).getCL()) );
					#endif
					same_level ++;
				} else {
					VEC_PUSH_BACK(lit_t,  literals, lit1 );
				}
			}
			#ifdef OTFSS
			else {
				if(! boolarray_get(inserted_variable,var1) ) topLLits++;
			}
			#endif
		}
		
		
		do {
			asserting = search.trail[trail_ind];
			asserting_var1 = var(asserting);
	
				assert( search.VAR_LEVEL(asserting_var1) == search.current_level);
			
				trail_ind--;
		}while ( !boolarray_get( inserted_variable,asserting_var1) );
		
		reason = search.VAR_REASON(asserting_var1);
		boolarray_set( inserted_variable, asserting_var1, false);
		same_level--;
		
		if(same_level == 0 ){
			cerr << "c found literal on wrong level, namly lit " << nr(asserting) << " @ " << search.VAR_LEVEL(asserting_var1) << endl;
			
			if( ! conflict_clause.isLit() ){
				cerr << "c conflict clause: "; printClause(  search, conflict_clause.getCL() ); cerr << endl;
			}
			cerr << "c current same level vars: " << same_level << endl;
			cerr << "c trail index of the literal: " << trail_ind << endl;
			cerr << "c current literals: ";
			for( uint32_t i = 0; i < VEC_SIZE( lit_t, literals); i++ ) cerr << " " << nr(literals[i]);
			cerr << endl;
			
		}
		assert(same_level > 0);
		
	}

	
	VEC_CLEAR( lit_t, literals);
	VEC_PUSH_BACK(lit_t,  literals, NO_LIT );

	
	do {
		
		
		assert ( !reason.empty() );
		

		if( reason.isLit() ){
			for (i = 0; i < 1; i++) {
				const lit_t lit1 = reason.getLit();
				if (lit1 == asserting) continue;
				const var_t var1 = var(lit1);
				const int32_t level = search.VAR_LEVEL(var1);
				if ((! boolarray_get(inserted_variable,var1) ) && (level > 0)){
					
					VEC_PUSH_BACK( lit_t, used_literals, lit1 );
					boolarray_set( inserted_variable,var1, true);
					if (level >= search.current_level ){
						#ifdef USE_SAME_LEVEL_PREFETCHING
							if( !search.VAR_REASON( var1 ).isLit() ) prefetch( gsa.addr( search.VAR_REASON( var1 ).getCL()) );
						#endif
						same_level ++;
					} else {
						VEC_PUSH_BACK(lit_t,  literals, lit1 );
					}
				}
				#ifdef OTFSS
				else {
					if(! boolarray_get(inserted_variable,var1) ) topLLits++;
				}
				#endif
			}

		} else {
			
		
			
			VEC_PUSH_BACK( CL_REF, used_clauses, reason.getCL() );
			

			CLAUSE& reason_cl = gsa.get(reason.getCL() );
			if( !reason_cl.isXor() && !reason_cl.is1ooN() ){
				
				for (i = 0; i < CL_SIZE( reason_cl ); i++) {
					const lit_t lit1 = CL_GET_LIT( reason_cl, i);
					if (lit1 == asserting) continue;
					const var_t var1 = var(lit1);
					const int32_t level = search.VAR_LEVEL(var1);
					if ((! boolarray_get(inserted_variable,var1) ) && (level > 0))
					{
						
						VEC_PUSH_BACK( lit_t, used_literals, lit1 );
						boolarray_set( inserted_variable,var1, true);
						if (level >= search.current_level ){
							#ifdef USE_SAME_LEVEL_PREFETCHING
								if( !search.VAR_REASON( var1 ).isLit() ) prefetch( gsa.addr( search.VAR_REASON( var1 ).getCL()) );
							#endif
							same_level ++;
						} else {
							VEC_PUSH_BACK(lit_t, literals, lit1 );
						}
					}
					#ifdef OTFSS
					else {
						if(! boolarray_get(inserted_variable,var1) ) topLLits++;
					}
					#endif
				}
				#ifdef OTFSS
				if( CL_SIZE( reason_cl ) == topLLits + same_level + VEC_SIZE( lit_t, literals) ){
					lit_t otfC[topLLits + same_level + VEC_SIZE( lit_t, literals)];
					uint32_t j = 0;
					CL_REF c = gsa.create( CL_COPY_EXCEPT(reason_cl,var(asserting)) );
					
					CL_IGNORE(reason_cl);
					if( CL_IS_LEARNT( reason_cl) != CL_IS_LEARNT(gsa.get(c) ) ) std::cerr << "c learnt info of otf cl is wrong" << std::endl;
					VEC_PUSH_BACK(CL_REF, otfs, c );
				}
				#endif
			} else {
				
				if( reason_cl.isXor() ){
					
					
					for (i = 0; i < CL_SIZE( reason_cl ); i++) {
						lit_t lit1 = CL_GET_LIT( reason_cl, i);
						
						
						if( i == 0 && reason.getCL() != conflict_clause.getCL() && !conflict_clause.isLit() ) lit1 = (assi_is_sat(search.assi, lit1) ) ? lit1 : inv_lit(lit1);
						
						else if( assi_is_sat(search.assi, lit1) ) lit1 = inv_lit(lit1);
						
						if ( var(lit1) == var(asserting) ) continue;
						const var_t var1 = var(lit1);
						const int32_t level = search.VAR_LEVEL(var1);
						if ((! boolarray_get(inserted_variable,var1) ) && (level > 0)){
							
							VEC_PUSH_BACK( lit_t, used_literals, lit1 );
							boolarray_set( inserted_variable,var1, true);
							if (level >= search.current_level ){
								#ifdef USE_SAME_LEVEL_PREFETCHING
									if( !search.VAR_REASON( var1 ).isLit() ) prefetch( gsa.addr( search.VAR_REASON( var1 ).getCL()) );
								#endif
								same_level ++;
							} else {
								VEC_PUSH_BACK(lit_t, literals, lit1 );
							}
						}
					}
				} if (reason_cl.is1ooN() ){
					
					cerr << "c IMPLEMENT analysis for 1ooN!!!" << endl;
				}
			}
		}

		
		do {
			asserting = search.trail[trail_ind];
			asserting_var1 = var(asserting);



			if( search.VAR_LEVEL(asserting_var1) != search.current_level ){
				cerr << "c found literal on wrong level, namly lit " << nr(asserting) << " @ " << search.VAR_LEVEL(asserting_var1) << endl;
				
				if( ! conflict_clause.isLit() ){
					cerr << "c conflict clause: "; printClause(  search, conflict_clause.getCL() ); cerr << endl;
				}
				cerr << "c current same level vars: " << same_level << endl;
				cerr << "c trail index of the literal: " << trail_ind << endl;
				cerr << "c current literals: ";
				for( uint32_t i = 0; i < VEC_SIZE( lit_t, literals); i++ ) cerr << " " << nr(literals[i]);
				cerr << endl;
				dumpTrail( search );
			}
			assert(! (search.VAR_LEVEL(asserting_var1) > search.current_level));
			assert(! (search.VAR_LEVEL(asserting_var1) < search.current_level));
			
			trail_ind--;
		}
		while ( !boolarray_get( inserted_variable,asserting_var1) );
		
		reason = search.VAR_REASON(asserting_var1);
		boolarray_set( inserted_variable, asserting_var1, false);
		same_level--;

	} while (same_level > 0);
	
	literals[0] = inv_lit(asserting);
	
	
	
	
	
	level_hash = 0;
	
	for (i = 0; i < VEC_SIZE(lit_t,  literals ); i++) {
		var_t var1;
		
		var1 = var(literals[i]);
		level_hash |= 1 << (search.VAR_LEVEL(var1) & 63);

		assert(i==0 || boolarray_get( inserted_variable,var1 ) == true);
	}
	
	
	VEC_ASSIGN( lit_t, clear, literals );

	
	
	if(dump_conflict_graphs) conflictDumper.addVec("unminimized",literals);
	
	if ( minimize > 0){
		if( minimize < 3){
			
			uint32_t i, j;
			clear = VEC_COPY( lit_t, literals); 
			if (minimize == 2){
				uint32_t abstract_level = 0;
				for (i = 1; i < VEC_SIZE( lit_t, literals); i++) 
				    abstract_level |= (1 << (search.VAR_LEVEL( var(literals[i]) ) & 31)); 

				for (i = j = 1; i < VEC_SIZE( lit_t, literals); i++)
				    if (search.VAR_REASON(var(literals[i])).empty() || !litRedundant(search, literals[i], abstract_level))
				        literals[j++] = literals[i];

			}else if (minimize == 1){
				for (i = j = 1; i < VEC_SIZE( lit_t, literals); i++){
				    const var_t x = var(literals[i]);

				    if (search.VAR_REASON(x).empty())
				        literals[j++] = literals[i];
				    else{
				    	if( search.VAR_REASON(var(literals[i])).isLit() ){
				    		const lit_t ilit = search.VAR_REASON(var(literals[i])).getLit();
					    	const var_t w = var(ilit);
					      if (!boolarray_get(inserted_variable,var(w)) && search.VAR_LEVEL(w) > 0){
					            literals[j++] = literals[i];
						    }
						  } else {
						    CLAUSE& c = gsa.get(search.VAR_REASON(var(literals[i])).getCL()  );
						    for (uint32_t k = 0; k < CL_SIZE(c); k++){
						    	
						    	const var_t w = var(CL_GET_LIT( c,k ));
									if( x == w ) continue;	
						      if (!boolarray_get(inserted_variable,var(w)) && search.VAR_LEVEL(w) > 0){
						          literals[j++] = literals[i];
						          break;
						      }
						    }						
						  }
				    }
				}
			}else
				i = j = VEC_SIZE( lit_t, literals);

			VEC_RESIZE( lit_t, literals,j, NO_LIT);
    	} else {
	
			VEC_CLEAR( lit_t, stack );
			VEC_RESERVE( lit_t, stack, VEC_SIZE(lit_t,  literals ) );
		
			
			int32_t new_level = 0;
		
			
			for (i = j = 1; i < VEC_SIZE(lit_t,  literals ); i++)
			{
				VEC_CLEAR(lit_t, stack );
				uint32_t clear_size;
				bool abort;
			
				
				
				VEC_PUSH_BACK(lit_t,  stack, literals[i] );
				clear_size = VEC_SIZE(lit_t,  clear );
				abort = false;
			
				while (VEC_SIZE(lit_t,  stack ) > 0 && !abort) {
					
					const lit_t lit1 = stack[ VEC_SIZE(lit_t,  stack ) - 1 ];
					VEC_POP_BACK(lit_t,  stack );
					reason = search.VAR_REASON( var(lit1) );
					
					if ( reason.empty() ) { 
						abort = true;
						break;
					}

				
					if( reason.isLit() ){
						
						const lit_t r_lit1 = reason.getLit();
						const var_t r_var1 = var(r_lit1);
					
						
						assert (r_lit1 != inv_lit(lit1)) ;
						
						if (( boolarray_get( inserted_variable,r_var1) == true) || (search.VAR_LEVEL( r_var1 ) == 0)) continue;
					
						
						
						
						const int32_t r_level = search.VAR_LEVEL(r_var1);
						const reasonStruct r_reason = search.VAR_REASON(r_var1);
					
						if ( r_reason.empty() || ( new_level_limit == 1 && ((1 << (r_level & 63)) & level_hash) == 0 ) ){
							abort = true;
							break;
						} else {
							new_level += (int32_t)((1 << (r_level & 63)) & level_hash) ;
						}
						
						VEC_PUSH_BACK(lit_t, clear, r_lit1 );
						VEC_PUSH_BACK(lit_t, stack, r_lit1 );
						boolarray_set( inserted_variable,r_var1, true);
					} else {
						
						CLAUSE& reason_cl = gsa.get(reason.getCL() );
						for (uint32_t k = 0; k< CL_SIZE( reason_cl ); k++) {
							lit_t r_lit1 = CL_GET_LIT( reason_cl, k);
							
							if( reason_cl.isXor() ){
								
								
								if( k == 0 ) r_lit1 = (assi_is_sat(search.assi, r_lit1) ) ? r_lit1 : inv_lit(r_lit1);
								
								else if( assi_is_sat(search.assi, r_lit1) ) r_lit1 = inv_lit(r_lit1);
							}
							const var_t r_var1 = var(r_lit1);
					
							
							if (r_lit1 == inv_lit(lit1)) continue;
							
							if (( boolarray_get( inserted_variable,r_var1) == true) || (search.VAR_LEVEL( r_var1 ) == 0)) continue;
					
							
							
							
							const int32_t r_level = search.VAR_LEVEL(r_var1);
							const reasonStruct r_reason = search.VAR_REASON(r_var1);
					
							if ( r_reason.empty() || ( new_level_limit == 1 && ((1 << (r_level & 63)) & level_hash) == 0 ) ){
								abort = true;
								break;
							} else {
								new_level += (int32_t)((1 << (r_level & 63)) & level_hash) ;
							}
					
							
							VEC_PUSH_BACK( CL_REF, used_clauses, reason.getCL() );
					
							
							VEC_PUSH_BACK(lit_t,  clear, r_lit1 );
							VEC_PUSH_BACK(lit_t,  stack, r_lit1 );
							boolarray_set( inserted_variable,r_var1, true);
						}
					}
				}
			
				if (abort) {
					
					literals[j] = literals[i];
					j++;
				
					
					while ( VEC_SIZE(lit_t,  clear ) > clear_size) {
						boolarray_set( inserted_variable, var( clear[ VEC_SIZE(lit_t, clear) - 1 ] ), false);
						VEC_POP_BACK(lit_t,  clear );
					}
				}
			

			}
	
			
			if (j < i)
				VEC_RESIZE(lit_t,  literals, j , 0);
		}
	}
	
	
	while ( VEC_SIZE(lit_t,  clear ) > 0) {
		boolarray_set( inserted_variable, var( clear[ VEC_SIZE(lit_t, clear) - 1 ] ), false);
		VEC_POP_BACK(lit_t,  clear );
	}

	
	int32_t max_level = -1;
	uint32_t max_pos = 0;

	for (uint32_t i = 1; i < VEC_SIZE(lit_t,  literals ); i ++) {
		lit_t lit1 = literals[i];
		int32_t level = search.VAR_LEVEL( var(lit1) );
		
		if (level > max_level) {
			max_level = level;
			max_pos = i;
		}
	}

	
	if (max_level > -1) {
		lit_t tmp = literals[max_pos];
		literals[max_pos] = literals[1];
		literals[1] = tmp;
		
		search.current_level = max_level;
	}
	 else {
		search.current_level = 0;
	}

	
	CL_REF out_clause = gsa.create( CL_CREATE( &(literals[0]) , VEC_SIZE(lit_t,  literals ) ) );

	if(dump_conflict_graphs){
		conflictDumper.addCls("final",out_clause);
		conflictDumper.finalize_last();
	}
	
	
	return out_clause;
}



bool CdclMinimizeAnalyzer::litRedundant(searchData& search, lit_t p, uint32_t abstract_levels)
{
	VEC_CLEAR( lit_t, stack);
	VEC_PUSH_BACK( lit_t, stack, p);
    int top = VEC_SIZE( lit_t, clear );
    while ( VEC_SIZE( lit_t, stack) > 0){
    	const var_t v = var( stack[ VEC_SIZE( lit_t, stack)-1] );
        assert( !search.VAR_REASON(v).empty() );
        
        if( search.VAR_REASON(v).isLit() ){
		    const lit_t ilit = search.VAR_REASON(v).getLit();
		    VEC_POP_BACK( lit_t, stack);
		    for (uint32_t i = 0; i < 1; i++){
		        const lit_t p = ilit;
		        if( var(p) == v ) continue; 

		        if (!boolarray_get(inserted_variable,var(p)) && search.VAR_LEVEL(var(p)) > 0){
		            if ( !search.VAR_REASON(var(p)).empty() && ( ( 1 << (search.VAR_LEVEL(var(p)) & 31)) & abstract_levels) != 0){
		            	boolarray_set(inserted_variable,var(p), true);
		            	VEC_PUSH_BACK( lit_t, stack, p);
		                VEC_PUSH_BACK( lit_t, clear, p);
		            }else{
		                for (uint32_t j = top; j < VEC_SIZE( lit_t, clear ); j++)
		                	boolarray_set(inserted_variable,var(clear[j]), false);
		                VEC_RESIZE( lit_t, clear,top, NO_LIT);
		                return false;
		            }
		        }
		    }
        } else {
		    CLAUSE& c = gsa.get( search.VAR_REASON(v).getCL() );
		    VEC_POP_BACK( lit_t, stack);
		    for (uint32_t i = 0; i < CL_SIZE(c); i++){
		        const lit_t p = CL_GET_LIT( c,i );
		        if( var(p) == v ) continue; 

		        if (!boolarray_get(inserted_variable,var(p)) && search.VAR_LEVEL(var(p)) > 0){
		            if ( !search.VAR_REASON(var(p)).empty() && ( ( 1 << (search.VAR_LEVEL(var(p)) & 31)) & abstract_levels) != 0){
		            	boolarray_set(inserted_variable,var(p), true);
		            	VEC_PUSH_BACK( lit_t, stack, p);
		                VEC_PUSH_BACK( lit_t, clear, p);
		            }else{
		                for (uint32_t j = top; j < VEC_SIZE( lit_t, clear ); j++)
		                	boolarray_set(inserted_variable,var(clear[j]), false);
		                VEC_RESIZE( lit_t, clear,top, NO_LIT);
		                return false;
		            }
		        }
		    }
        }
        
    }

    return true;
}

void CdclMinimizeAnalyzer::set_parameter( const StringMap& commandline )
{
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"cdcl_minimize" ) ) minimize = atoi( commandline.get( (const char*)"cdcl_minimize" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_add_short" ) ) add_trivail_short = 0 != atoi( commandline.get( (const char*)"cdcl_add_short" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_new_limit" ) ) new_level_limit = atoi( commandline.get( (const char*)"cdcl_new_limit" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_prefetch" ) ) prefetch_reasons = atoi( commandline.get( (const char*)"cdcl_prefetch" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_dump_graph" ) ) dump_conflict_graphs = 0 != atoi( commandline.get( (const char*)"cdcl_dump_graph" ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== cdcl min analyze information ===" << std::endl;
		std::cerr << " parameter            pos.value info" << std::endl;
		std::cerr << " cdcl_minimize        0,3       minimize learnt clauses(1=simple,2=recusive heavy,3=iterative heavy)" << std::endl;
		std::cerr << " cdcl_add_short       0,1       add dpll-unit/dual at low conflict levels" << std::endl;
		std::cerr << " cdcl_new_limit       0,1       do level check, or don't" << std::endl;
		std::cerr << " cdcl_prefetch        0-n       number of first literals in conflict, whose reasons are prefetched" << std::endl;
		std::cerr << " cdcl_dump_graph      0,1       dump implication graph at all conflicts" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
}

#ifdef COMPILE_LIBRARY

extern "C" CdclMinimizeAnalyzer* create_conflictanalysis( uint32_t var_cnt, const StringMap& commandline){
	return new CdclMinimizeAnalyzer(  var_cnt, commandline );
}

extern "C" void destroy_conflictanalysis( CdclMinimizeAnalyzer* analysis ){
	delete analysis;
}
#endif
