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


#ifndef _CDCLSEARCH_H
#define _CDCLSEARCH_H

#include "debug.h"

#include "types.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/stack_macros.h"
#include "macros/vector_macros.h"

#include "decisionheuristic/backtrack_heuristic.h"

#include "structures/c_assignment.h"
#include "utils/stringmap.h"
#include "utils/statistics.h"
#include "utils/clausehandler.h"
#include "utils/monitor.h"

#include "utils/formulaSCCfinder.h"
#include "utils/formulaLitSCC.h"
#include "utils/network.h"

#include "sat/searchdata.h"

#include <fstream>
#include <pthread.h>


template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
class CdclSearch
{
protected:
	uint64_t startTime;	
	bool searchTimedOut;	
	searchData& sd;	
	VEC_TYPE( CL_REF )* formula; 

public:
	
	CdclSearch( searchData& data, VEC_TYPE( CL_REF )* formula, PRP& prepro, const StringMap& commandline, Network& _nw ,int64_t timeO = -1 );

	
	~CdclSearch();
	
	
	VEC_TYPE( assi_t )* search();	

	
	
	solution_t init();
	
	
	bool timedOut();
	
	
protected:



	
	bool clauses_learnt();

	
	bool addClauses(VEC_TYPE(CL_REF)& newClauses);
	
	
	void remove();
	
	
	void analyze();
	
	
	void backtrack(int32_t goTo = -1);
	
	
	solution_t restart();
	
	
	solution_t simplify();
	
	
	void propagate_intern();
	
	
	bool timeLeft();
	

	
	void set_parameter( const StringMap& commandline );
	
	
	
	void levelSortClause( CLAUSE& cl );
	
	
	void show_stat();


	
	bool setAssumptions( VEC_TYPE(lit_t)& assumpts);	
	
	
	bool addAssumption( lit_t l);
	
	
	void clearAssumptions();
	
	const StringMap& cmdline;

	
	VEC_TYPE( CL_REF ) solution_conflicts; 
	VEC_TYPE( conflictStruct ) conflict_clauses;	
	VEC_TYPE( CL_REF ) learned_clauses;	
	VEC_TYPE( lit_t ) used_literals;
	VEC_TYPE( var_t ) undefed_vars;
	VEC_TYPE( CL_REF ) restart_learnts;	
	VEC_TYPE( CL_REF ) clauses_to_remove;
	VEC_TYPE(CL_REF) used_clauses;
	VEC_TYPE(CL_REF) otfClauses;	
	VEC_TYPE(CL_REF) networkClauses;
	
	
	DEH heuristic_data;	
	UPR propagation_data;	
	RSE restart_event;	
	RME removal_event;	
	RMH<CAH> removal_data;	
	ANA analyzer_data;	
	PRP& pp;
	BacktrackHeuristic backjumper; 
	Network& nw;	

	
	int32_t topLevel;	
	lit_t descision_literal;	
	assi_t assi_inverse;	
	assi_t assi_backup;	
	VEC_TYPE(lit_t) probing_units; 
	VEC_TYPE(lit_t) probing_equals; 
	std::vector<lit_t> replacedBy;	
	VEC_TYPE(assi_t)* solutions;	
	solution_t solution;	

	
	uint32_t decisions;
	uint32_t conflicts;
	uint32_t restarts;
	uint32_t rejected_restarts;
	uint32_t removals;
	uint32_t removed_clauses;
	uint32_t otfc;
	
	
	CONST_PARAM int64_t timeOut;	
	CONST_PARAM bool verbose;	
	CONST_PARAM uint32_t max_solutions;	
	CONST_PARAM int32_t dump_learned;	
	bool use_learnts; 
	CONST_PARAM bool SCC_analysis; 
	CONST_PARAM bool doSimplifying;	
	CONST_PARAM uint32_t initSimplifyRounds; 
	CONST_PARAM bool reset_backup;	
	CONST_PARAM uint32_t probingMode; 
	CONST_PARAM float restartAgilityThreshold;	

	ClauseHandler* ch;	
};




#include <libpd/libpd.h>

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::CdclSearch( searchData& data, VEC_TYPE( CL_REF )* formula,PRP& prepro,  const StringMap& commandline, Network& _nw, int64_t timeO)
 : startTime( get_microseconds() ),
	searchTimedOut(false),
	sd( data ),
	cmdline( commandline ),
	heuristic_data( formula, sd, commandline ),	
	propagation_data( sd, formula, 0, commandline),	
	restart_event(  (char*)"restart_event", formula, commandline),	
	removal_event( (char*)"removal_event", formula, commandline),	
	removal_data( sd.var_cnt, commandline ),	
	analyzer_data( sd.var_cnt, commandline ),	
	pp( prepro), 
	backjumper( commandline ),	
	nw( _nw),
	timeOut( timeO),
	verbose ( false),
	max_solutions ( 1),
	dump_learned ( false),
	use_learnts ( false),
	SCC_analysis ( false),
	doSimplifying ( false),
	initSimplifyRounds ( 0),
	reset_backup( false),
	probingMode( 0),
	restartAgilityThreshold(0.22f)
{
	this->formula = formula;
#ifndef PARALLEL
#ifndef COMPETITION
	std::cerr << "c init all components done" << std::endl;
#endif
#endif
	descision_literal = NO_LIT;
	topLevel = 0;

	
	decisions = 0;
	conflicts = 0;
	restarts = 0;
	rejected_restarts = 0;
	removals = 0;
	removed_clauses = 0;
	otfc = 0;
	
	solution = UNKNOWN;
	
	assi_inverse = assi_create(sd.var_cnt);
	assi_backup = assi_create(sd.var_cnt);
	VEC_CREATE( lit_t, probing_units );
	VEC_CREATE( lit_t, probing_equals );
	
	replacedBy.resize( sd.var_cnt + 1, NO_LIT );
	for( var_t var1 = 0 ; var1 < sd.var_cnt+1; var1 ++ ){
		replacedBy[ var1 ] = lit( var1, POS );
	}
	
	set_parameter( commandline );
	
	if( dump_learned != 0 || use_learnts ) ch = new ClauseHandler( (const char*)"learnts.cnf" );
	else ch = 0;
	
	if( commandline.contains( (const char*)"plotRestart" ) ){
		uint32_t max = atoi( commandline.get( (const char*)"plotRestart" ).c_str() );
		uint32_t count = 0;
		for( uint32_t i = 0 ; i < max; ++i ){
			if( restart_event.required( i, decisions, sd ) ){
				count++;
				std::cerr << "c restartPlot " << count << " " << i << std::endl;
				restart_event.restart_done();
			}
		}
		exit(0);
	}
	
	
	solution = (formula == 0 ) ? UNSAT : UNKNOWN;	
	VEC_CREATE_IN_HEAP( assi_t, solutions );
	VEC_CREATE(CL_REF, solution_conflicts );	
	
	VEC_CREATE( conflictStruct, conflict_clauses );
	VEC_CREATE( CL_REF, learned_clauses );
	VEC_CREATE_SPACE(lit_t, used_literals, sd.var_cnt);	
	VEC_CREATE_SPACE(var_t, undefed_vars, sd.var_cnt/4);	
	VEC_CREATE_SPACE( CL_REF, restart_learnts, 10000);
	VEC_CREATE_SPACE(CL_REF, otfClauses, 20 );
	VEC_CREATE_SPACE(CL_REF, networkClauses, 20 );

	
	
	VEC_CREATE_SPACE( CL_REF, clauses_to_remove, 1000 );
	VEC_CREATE_SPACE( CL_REF, used_clauses, 100 );
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::~CdclSearch()
{
	
	assi_destroy(assi_inverse, sd.var_cnt);
	assi_destroy(assi_backup, sd.var_cnt);
	VEC_DESTROY( lit_t, probing_units);
	VEC_DESTROY( lit_t, probing_equals);
	
	
	
	VEC_DESTROY(var_t, undefed_vars);
	VEC_DESTROY(lit_t, used_literals);
}


#include "dummies.h"

LIBPD_IMPORT(conflicts, libpd_integer_t)
LIBPD_IMPORT(decisions, libpd_integer_t)
LIBPD_IMPORT(restarts, libpd_integer_t)

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::timedOut(){
	return searchTimedOut;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::timeLeft(){
	if( timeOut == -1 ) return true;
	searchTimedOut = (startTime + timeOut * 1000 < get_microseconds() );
	return !searchTimedOut;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline solution_t CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::init()
{
	
	if( cmdline.contains( (const char*)"-h" ) || cmdline.contains( (const char*)"--help" ) ) exit( 0 );
	
	
	if( use_learnts ){
		ch->read(learned_clauses);
#ifndef COMPETITION
		std::cerr << "c loaded " << VEC_SIZE( CL_REF, learned_clauses ) << " from external learnt file" << std::endl;
#endif
		
		use_learnts = false;
		
		if( CdclSearch::clauses_learnt() == false ) solution = UNSAT;
	}
#ifndef COMPETITION	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, (*formula) ); ++i ){
		if( CL_IS_IGNORED( gsa.get((*formula)[i]) ) ) std::cerr << "c found ignored clause in formula" << std::endl;
	}
#endif

	uint32_t tc = 0;
	for( var_t var1 = 0 ; var1 <= sd.var_cnt; var1++ ){
		if( !assi_variable_equal( sd.assi, var1, UNDEF ) ){
			tc ++;
			
			STACK_PUSH(lit_t, sd.trail, lit(var1, assi_get_polarity( sd.assi, var1 )) );
			sd.VAR_LEVEL( var1 ) = 0;
			sd.VAR_REASON( var1 ) = reasonStruct();
		}
	}
#ifndef SILENT
	cerr << "c " << tc << " variables already set by preprocessor" << endl;
#endif
	
	
	propagation_data.propagate( sd, conflict_clauses);
	if( VEC_SIZE( CL_REF, conflict_clauses ) != 0){
		
		solution = UNSAT;
	}
	
	for( uint32_t i = 0 ; i < initSimplifyRounds; ++i )
	{
		std::cerr << "c initial simplify round " << i << std::endl;
		
		CdclSearch::simplify();

		
		propagation_data.propagate( sd, conflict_clauses );
		if( VEC_SIZE( CL_REF, conflict_clauses ) != 0){
			
			solution = UNSAT;
		}
	}
	return solution;
}




template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
inline VEC_TYPE( assi_t )* CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::search()
{

	if ( UNSAT == init() ) return solutions;	

#ifndef PARALLEL
	std::cerr << "c ====================" << std::endl;
#ifndef COMPETITION
	std::cerr << "c trail size: " << STACK_SIZE( lit_t, sd.trail ) << std::endl;
#endif
#endif
	
	MON_REG( "restart", DataMonitor::TIME, 0);
	MON_REG( "removal", DataMonitor::TIME, 0);
	MON_REG( "level", DataMonitor::MEAN, 'i' );	
	MON_REG( "conflicts", DataMonitor::MEAN, 'i' );
	MON_REG( "decisions", DataMonitor::MEAN, 'i' );


	
	
	while(solution == UNKNOWN && timeLeft() )
	{
		 MON_WAIT_GUI();
		 
		 MON_EVENT( "level", &sd.current_level );
		 
#ifdef PARALLEL
		 
		 pthread_testcancel();
#endif

		
		if( VEC_SIZE( CL_REF, conflict_clauses ) == 0 ){

				if( restart_event.required( conflicts, decisions, sd ) ){
#ifdef ASSI_AGILITY
					
					if ( restartAgilityThreshold <= assi_get_agility() ) {
						rejected_restarts++;
					} else {
#endif
						
						if( UNSAT == CdclSearch::restart() ) continue;
						
						sd.resetLocalCounter();
#ifdef ASSI_AGILITY
					}
#endif
				}
				
				if( removal_event.required( conflicts, decisions, sd ) ){
					CdclSearch::remove();
				}
				
				
#ifdef PARALLEL
				
				VEC_CLEAR(CL_REF, networkClauses);
				nw.giveClauses(networkClauses,sd);
				if( VEC_SIZE(CL_REF,networkClauses) > 0 ){
					int32_t backLevel = sd.current_level;
					for( uint32_t i = 0 ; i < VEC_SIZE(CL_REF,networkClauses); ++i){
						
						const CL_REF netCl = networkClauses[i];
						
						CLAUSE& cl = gsa.get( netCl );
						uint32_t unsatL = 0;
						int32_t maxLevel = 0;	
						int32_t sndLevel = 0;	
						int32_t maxSLevel = 0;	
						uint32_t ud = 0,us=0;	
							
						
						for( uint32_t j = 0; j < CL_SIZE(cl); ++j ){
							const lit_t literal = CL_GET_LIT(cl,j);
							const int32_t lev = sd.VAR_LEVEL( var(literal) );
							if( boolarray_get( sd.eliminated, var(literal) ) ){
								cerr << "c received clause contains eliminated literal " << nr(literal) << endl;
								
								VEC_ERASE_NO_ORDER( CL_REF, networkClauses, i);
								i--;
							}
							if( assi_is_undef( sd.assi, var(literal) ) ) ud ++; 
							else if( assi_is_unsat( sd.assi, literal ) ){
								if( lev > maxLevel ){ sndLevel = maxLevel; maxLevel = lev; }
								else sndLevel = sndLevel >= lev ? sndLevel : lev;
								us ++;
							} else {
								maxSLevel = maxSLevel >= lev ? maxSLevel : lev;
							}
						}
						
						stringstream s;
						
						if( us == CL_SIZE(cl) ){
							backLevel = backLevel <= sndLevel ? backLevel : sndLevel;
						} else {
							if( us + 1 == CL_SIZE(cl)) {
								
								
								if( ud == 1 || maxSLevel > maxLevel){
									backLevel = backLevel <= maxLevel ? backLevel : maxLevel;
								} 
							}
						}
						
						if( CL_SIZE(cl) == 1 ){
							backLevel = 0;
						}
					}
			
					
					if( backLevel != sd.current_level ){
						
						CdclSearch::backtrack( backLevel );
					}
					
					if( !addClauses(networkClauses) ){
						stringstream s;
						s << "c failed to add network clauses" << endl;
						cerr << s.str();
						continue;
					}
			
					for( uint32_t i = 0 ; i < VEC_SIZE(CL_REF,networkClauses); ++i){
						CLAUSE& cl = gsa.get( networkClauses[i] );
						uint32_t j = 0;
						for(  ; j < CL_SIZE(cl); ++j ){
							if( !assi_is_unsat( sd.assi, CL_GET_LIT(cl,j) ) ) break;
						}
						if( j == CL_SIZE(cl) ) cerr << "c UNSAT clause among NW clauses" << endl;
					}
					
					propagate_intern();
					continue;
				}
#endif
				
				
				descision_literal = heuristic_data.get_literal(sd);
				if( descision_literal == NO_LIT ){
					
					VEC_PUSH_BACK(assi_t, (*solutions), assi_copy( sd.assi, sd.var_cnt ) );
				
					
					solution = SAT;
					break;
				}
				
				
				
				
				decisions++;
				MON_EVENT( "decisions", &decisions );
				STAT_DECIDED( descision_literal );
				LIBPD_COUNTER_INC(decisions);
				sd.current_level++;
				
				
				
				
				propagation_data.enqueue_decision(sd, descision_literal);
				
		
		} else {
				
				descision_literal = NO_LIT;
				
				if( sd.current_level == topLevel ){ solution = UNSAT; break; }
				
				conflicts ++;
				MON_EVENT( "conflicts", &conflicts );
				LIBPD_COUNTER_INC(conflicts);
				
				
				CdclSearch::analyze();

				

				backjumper.backtrack_to( sd, learned_clauses );
				STAT_LEARNED( learned_clauses );
				
				heuristic_data.literals_conflicted(sd, used_literals );
				removal_data.inc_analysis_activity(sd, used_clauses );

				
				
				for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, learned_clauses ); ++i ){
					CLAUSE& cl = gsa.get(learned_clauses[i]);
					sd.globalLearntAvg+=CL_SIZE(cl);
					sd.currentLearntAvg+=CL_SIZE(cl);
					sd.globalLearntMin= sd.globalLearntMin <= CL_SIZE(cl) ? sd.globalLearntMin : CL_SIZE(cl);
					sd.currentLearntMin= sd.currentLearntMin <= CL_SIZE(cl) ? sd.currentLearntMin : CL_SIZE(cl);
					sd.currentLearnts++;
				}


				CdclSearch::backtrack();

				if( CdclSearch::clauses_learnt() == false ) solution = UNSAT;
				if( solution == UNSAT ) break;					

		}
		
		
		propagate_intern();
		
	}	

	std::cerr << "c ====================" << std::endl;

#ifndef COMPETITION
	
	if( nw.doPrint() ) show_stat();
#endif
	
	if( timedOut() ) std::cerr << "c search timed out" << endl;
	
	
	VEC_DESTROY( CL_REF, clauses_to_remove);
	
	
	for( int32_t i = VEC_SIZE( CL_REF,  solution_conflicts ) - 1; i >= 0; --i ){
		CL_DESTROY( gsa.get(solution_conflicts[i] ) );
		gsa.release( solution_conflicts[i] );
		
	}
	VEC_DESTROY( CL_REF,  solution_conflicts );
	VEC_DESTROY( CL_REF, restart_learnts);

	return solutions;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::addClauses(VEC_TYPE(CL_REF)& newClauses){
	
	if( !this->propagation_data.clauses_added(this->sd, newClauses) ){
		int32_t blevel = this->sd.current_level + 1;
		CL_REF clause = 0;
		uint32_t csize = 0;
		
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, newClauses ); ++i ){
			CLAUSE& cl = gsa.get( newClauses[i] );
			uint32_t j = 0;
			int32_t tl = -1;
			for(  ;j < CL_SIZE(cl); ++j ){
				const lit_t l = CL_GET_LIT( cl, j );
				if ( !assi_is_unsat( this->sd.assi, l) ) continue;
				tl = ( this->sd.VAR_LEVEL( var(l) ) > tl ) ? this->sd.VAR_LEVEL( var(l) ) : tl;
			}
			if( j == CL_SIZE(cl) ){	
				if( blevel > tl ){	
					clause = newClauses[i];blevel = tl;csize = CL_SIZE(cl);
				} else { 
					if( blevel == tl && csize > CL_SIZE(cl) ){
						clause = newClauses[i];blevel = tl;csize = CL_SIZE(cl);
					}
				}
			}
		}
		
		if( this->sd.current_level > blevel ){
			this->sd.current_level = blevel;
			VEC_CLEAR( lit_t, this->undefed_vars );
			this->propagation_data.backtrack(this->sd, blevel, this->undefed_vars );
			this->heuristic_data.variables_unassigned( this->undefed_vars );	
		}
		
		this->solution = UNKNOWN;
		
		VEC_CLEAR( conflictStruct, this->conflict_clauses );
		VEC_PUSH_BACK( conflictStruct, this->conflict_clauses, conflictStruct(clause) );
		return false;
	} else {
		return true;
	}
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::clauses_learnt(){
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, learned_clauses ); i++ )
	{
		
		if( ! use_learnts ){
			
			if( (int32_t)( CL_SIZE( gsa.get(learned_clauses[i]) ) ) < dump_learned ) ch->write(learned_clauses[i]);
			else if( dump_learned == -1 ){
				
				ch->write(learned_clauses[i]);
			}
		}
		
		if( CL_SIZE( gsa.get(learned_clauses[i]) ) == 0 ) return false;
	}
	
	
	heuristic_data.clauses_learned( sd, learned_clauses);
	
	propagation_data.clauses_added(sd, learned_clauses );	
	nw.addClauses( learned_clauses, sd);
#ifdef OTFSS
	if( VEC_SIZE(CL_REF, otfClauses) > 0 ){
		for( uint32_t tj = 0 ; tj < VEC_SIZE(CL_REF, otfClauses); ++tj){
			const CLAUSE& otfCL = gsa.get(otfClauses[tj]);
			
			if( !CL_IS_LEARNT( otfCL ) ) VEC_PUSH_BACK(CL_REF, (*formula), otfClauses[tj]);
		}
		
		propagation_data.clauses_added(sd, otfClauses );
		nw.addClauses( otfClauses,sd );
		
		removal_data.clauses_learnt( sd, otfClauses );
	}
#endif
	
	removal_data.clauses_learnt( sd, learned_clauses );

	
	restart_event.clauses_added( learned_clauses, sd );
	removal_event.clauses_added( learned_clauses, sd );
	
	return true;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::remove()
{
	MON_EVENT( "removal", 0 );
	
#ifndef SILENT
	std::cerr << "c removal at " << conflicts << " conflicts and level " << sd.current_level
		<< " learnt: avg=" << sd.globalLearntAvg / (double) conflicts << "["
		<< sd.currentLearntAvg / sd.currentLearnts << "]"
		<< " min= " << sd.globalLearntMin << "["
		<< sd.currentLearntMin << "]"
#ifndef COMPETITION
		 << " lits on trail: " << STACK_SIZE( lit_t, sd.trail ) 
#endif
		 << std::endl;
#endif
	removals++;
	VEC_CLEAR( CL_REF, clauses_to_remove );
	
	removal_data.remove( sd, clauses_to_remove);
	
	const uint32_t size = VEC_SIZE( CL_REF, clauses_to_remove);
	
	removed_clauses += size;
	
	propagation_data.clauses_removed(sd, clauses_to_remove );

	restart_event.clauses_removed(clauses_to_remove);
	removal_event.clauses_removed(clauses_to_remove);

	
	for( uint32_t i = 0 ; i < size; ++i ){
		CLAUSE& cl = gsa.get(clauses_to_remove[i]); 	
		if( CL_IS_LEARNT(cl) && CL_SIZE(cl) == 2 ){
			cerr << "c remove learned binary clause in PP" << endl;
		}
		CL_DESTROY( cl );
		gsa.release( clauses_to_remove[i] );
	}
	if( verbose && sd.current_level != 0 ) { std::cerr << "do removal" << std::endl; show_stat(); }
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
solution_t CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::restart()
{

	MON_EVENT( "restart", 0 );

#ifndef SILENT
	std::cerr << "c do restart at " << conflicts << " conflicts and level " << sd.current_level
	<< " learnt: avg=" << sd.globalLearntAvg / (double) conflicts << "["
	<< sd.currentLearntAvg / sd.currentLearnts << "]"
	<< " min=" << sd.globalLearntMin << "["
	<< sd.currentLearntMin << "]"
#ifndef COMPETITION
	 << " and trailsize " << STACK_SIZE( lit_t, sd.trail ) << " and assi_agi " << assi_get_agility()
#endif
	 << std::endl;
#endif
	STAT_DORESTART();
	if(verbose)	show_stat();
	restarts++;
	LIBPD_COUNTER_INC(restarts);
	
	
	VEC_CLEAR( var_t, undefed_vars );
	
	sd.current_level = topLevel;
	propagation_data.backtrack(sd, topLevel, undefed_vars );
	heuristic_data.variables_unassigned( undefed_vars );
	VEC_CLEAR( var_t, undefed_vars );
	
	solution = CdclSearch::simplify();
	
	if( reset_backup ){
		for( var_t v = 1; v <= sd.var_cnt; ++v ){
			if( assi_variable_equal( sd.assi, v, UNDEF ) ){
				assi_undef_variable_backup( sd.assi, v);
			}
		}
	}
	
	
	removal_data.restart_done();
	
	restart_event.restart_done();
	removal_event.restart_done();
	return solution;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
solution_t CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::simplify()
{
	
	if( !doSimplifying ) return UNKNOWN;
	
	if( sd.current_level != 0 || !pp.doSimplify(sd) ) return UNKNOWN;
	
	
	if( true ){
		removal_data.get_all_clauses( restart_learnts );
		

		
		
		solution_t tsol = pp.simplifysearch(sd, (*formula), restart_learnts );
		if( tsol == UNSAT ) return UNSAT;
		

		for( var_t var1 = 0 ; var1 <= sd.var_cnt; var1++ ){
			if( !assi_variable_equal( sd.assi, var1, UNDEF ) && sd.VAR_LEVEL( var1 ) != 0 ){
				
				
				STACK_PUSH(lit_t, sd.trail, lit(var1, assi_get_polarity( sd.assi, var1 )) );
				sd.VAR_LEVEL( var1 ) = 0;
				sd.VAR_REASON( var1 ) = reasonStruct();
			}
		}

		
		
		
		removal_data.reinit( sd, restart_learnts );
		propagation_data.reinit( sd, (*formula), restart_learnts, (uint32_t)sd.var_cnt);
		VEC_CLEAR( CL_REF,  restart_learnts );
	}
	
	if(SCC_analysis)
	{
		if( true )
		{
			FormulaSCCFinder f;
			VEC_TYPE( CL_REF ) all_clauses;
			VEC_CREATE( CL_REF, all_clauses );
			removal_data.get_all_clauses( all_clauses );
		
			VEC_PUSH_BACK_ANOTHER( CL_REF, all_clauses, (*formula), VEC_SIZE( CL_REF, (*formula) ) );
			f.SCC( &all_clauses, sd.var_cnt, true, sd.assi );
			VEC_DESTROY( CL_REF, all_clauses );
		} else {
			FormulaLitSCC f;
			VEC_TYPE( CL_REF ) all_clauses;
			VEC_CREATE( CL_REF, all_clauses );
			removal_data.get_all_clauses( all_clauses );
		
			VEC_PUSH_BACK_ANOTHER( CL_REF, all_clauses, (*formula), VEC_SIZE( CL_REF, (*formula) ) );
			f.analyze( &all_clauses, sd.var_cnt, true, sd.assi );
			VEC_DESTROY( CL_REF, all_clauses );		
		}
	}
	return UNKNOWN;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::analyze()
{
	VEC_CLEAR( CL_REF, used_clauses );
	VEC_CLEAR( CL_REF, learned_clauses );
	VEC_CLEAR(CL_REF, otfClauses);
	VEC_CLEAR(lit_t, used_literals);
	analyzer_data.analyze(sd, used_clauses, conflict_clauses, learned_clauses, used_literals, otfClauses );
	otfc += VEC_SIZE(CL_REF, otfClauses);
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::backtrack(int32_t goTo)
{
	if( goTo != -1 ){
		sd.current_level = goTo;
	}
	
	VEC_CLEAR( var_t, undefed_vars );
	propagation_data.backtrack(sd, sd.current_level, undefed_vars );
	heuristic_data.variables_unassigned( undefed_vars );
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::propagate_intern(){
	
	if(  probingMode == 0 ||  sd.current_level != 1 || descision_literal == NO_LIT ){
		
		VEC_CLEAR( CL_REF, conflict_clauses );
		propagation_data.propagate(sd ,conflict_clauses );

		return;
	}
#ifndef COMPETITION
	if( probingMode == 2 ){
		std::cerr << "c ATTENTION =========" << std::endl;
		std::cerr << "c probingMode 2 is not yet implemented!" << std::endl;
		std::cerr << "c set to mode 1" << std::endl;
		std::cerr << "c ===================" << std::endl;
		probingMode = 1;
	}
#endif
	if( probingMode == 1 || probingMode == 2){
		
		lit_t first_literal = descision_literal;
		
		propagation_data.backtrack(sd, 0, used_literals);
		
		sd.current_level = 1;
		
		assi_copy_to( assi_backup, sd.assi, sd.var_cnt);

		
		propagation_data.enqueue_decision(sd, inv_lit(first_literal) );
		VEC_CLEAR( CL_REF, conflict_clauses);
		propagation_data.propagate(sd ,conflict_clauses);

		
		if( VEC_SIZE( CL_REF, conflict_clauses ) > 0 ) return;

		assi_copy_to( assi_inverse, sd.assi, sd.var_cnt); 
		propagation_data.backtrack(sd, 0, used_literals);
		sd.current_level = 1;

		
		propagation_data.enqueue_decision(sd, first_literal);
		VEC_CLEAR( CL_REF, conflict_clauses);
		propagation_data.propagate(sd ,conflict_clauses);
		
		if( VEC_SIZE( CL_REF, conflict_clauses ) > 0 ) return;
		
		
		VEC_CLEAR( lit_t, used_literals );
		
		assi_undef_variable(sd.assi, var(first_literal) );
		assi_undef_variable(assi_inverse, var(first_literal) );
		
		VEC_CLEAR( lit_t, probing_units );
		VEC_CLEAR( lit_t, probing_equals );
		int count = assi_get_equal_lits(sd.assi, assi_inverse, assi_backup, probing_units, probing_equals, sd.var_cnt);

		
		assi_set_polarity( sd.assi, var(first_literal), pol(first_literal) );

		
		if( count > 0 ){
			
			propagation_data.backtrack(sd, 0, used_literals);
			sd.current_level = 0;
			VEC_CLEAR( lit_t, used_literals );
			for( uint32_t i = 0 ; i < VEC_SIZE( lit_t, probing_units ); ++i ){
				if( assi_is_undef( assi_backup, probing_units[i]) ){
					propagation_data.enqueue_decision(sd, probing_units[i]);
				}
			}
			
			VEC_CLEAR( CL_REF, conflict_clauses);
			propagation_data.propagate(sd ,conflict_clauses);
			
			if( VEC_SIZE( CL_REF, conflict_clauses ) > 0 ){
				std::cerr << "c probing: unsat due to found units in a assignment" << std::endl;
				return;
			}
			
			nw.addUnits( probing_units, sd);
			
			sd.current_level = 1;
			propagation_data.enqueue_decision(sd, first_literal);
			VEC_CLEAR( CL_REF, conflict_clauses);
			propagation_data.propagate(sd ,conflict_clauses);
			VEC_CLEAR( var_t, used_literals);
			return;
		}

		VEC_CLEAR( lit_t, used_literals ); 
		if( VEC_SIZE( lit_t, probing_equals ) > 0 ){
			
			pp.addEquis( probing_equals );
			
			
		}
	}

}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::show_stat()
{
	std::cerr << "c implication/decision " << (((double)STACK_SIZE( lit_t, sd.trail ) / (double)sd.current_level) * 100.0) / (double) sd.var_cnt << "%" << "(" << (double)STACK_SIZE( lit_t, sd.trail ) / (double)sd.current_level << ")" << std::endl;					
	std::cerr << "c decisions:           " << decisions << std::endl;
	std::cerr << "c last level:          " << sd.current_level << std::endl;
	std::cerr << "c conflicts:           " << conflicts << std::endl;
	std::cerr << "c restarts:            " << restarts << std::endl;
	std::cerr << "c rejected_restarts    " << rejected_restarts << std::endl;
	std::cerr << "c removals:            " << removals << std::endl;
	std::cerr << "c removed clauses:     " << removed_clauses << std::endl;
	std::cerr << "c oft clauses:         " << otfc << std::endl;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::setAssumptions( VEC_TYPE(lit_t)& assumpts ){
	restart();
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, assumpts ); ++i )
	{
		lit_t l = assumpts[l];
		topLevel ++;
		propagation_data.enqueue_decision(sd, l);
		VEC_CLEAR( CL_REF, conflict_clauses);
		propagation_data.propagate(sd ,conflict_clauses);
		if( VEC_SIZE( CL_REF, conflict_clauses ) > 0 ){
			topLevel --;
			VEC_CLEAR( CL_REF, conflict_clauses);
			restart();
			return false;
		} else return true;
	}
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
bool CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::addAssumption( lit_t l){
	restart();
	topLevel ++;
	propagation_data.enqueue_decision(sd, l );
	VEC_CLEAR( CL_REF, conflict_clauses);
	propagation_data.propagate(sd ,conflict_clauses);
	if( VEC_SIZE( CL_REF, conflict_clauses ) > 0 ){
		topLevel --;
		VEC_CLEAR( CL_REF, conflict_clauses);
		restart();
		return false;
	} else return true;
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::clearAssumptions(){
	topLevel = 0;
	restart();
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::levelSortClause( CLAUSE& cl ){
	for( uint32_t i = 0 ; i < CL_SIZE(cl); i++ ){
		int32_t lev = sd.VAR_LEVEL( var( CL_GET_LIT(cl,i) ) );
		uint32_t p = i;
		for( uint32_t j = i+1 ; j < CL_SIZE(cl); j++ ){
			const int32_t lev2 = sd.VAR_LEVEL( var( CL_GET_LIT(cl,j) ) );
			if( assi_is_undef(sd.assi, var(CL_GET_LIT(cl,j)) ) ){ p = j; break; }
			if( lev2 > lev ) { lev = lev2; p = j; }
		}
		if( i != p ) CL_SWAP_LITS( cl, p,i );
	}
}

template <class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH >
void CdclSearch<UPR, DEH, RSE, RME, ANA, PRP, CAH, RMH >::set_parameter( const StringMap& commandline )
{
	
	if( commandline.contains( (const char*)"sol" ) != 0 )  max_solutions = atoi( commandline.get( (const char*)"sol" ).c_str() );
	else  max_solutions = 1;
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"cdcl_verbose" ) ) verbose = true;
	if( commandline.contains( (const char*)"cdcl_dump_learnts" ) ) dump_learned = atoi( commandline.get( (const char*)"cdcl_dump_learnts" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_use_learnts" ) ) use_learnts = ( 1 == atoi( commandline.get( (const char*)"cdcl_use_learnts" ).c_str() ) );
	if( commandline.contains( (const char*)"cdcl_SCC" ) ) SCC_analysis = ( 1 == atoi( commandline.get( (const char*)"cdcl_SCC" ).c_str() ) );
	if( commandline.contains( (const char*)"cdcl_simp" ) ) doSimplifying = ( 1 == atoi( commandline.get( (const char*)"cdcl_simp" ).c_str() ) );
	if( commandline.contains( (const char*)"cdcl_init_simps" ) ) initSimplifyRounds = ( 1 == atoi( commandline.get( (const char*)"cdcl_init_simps" ).c_str() ) );
	if( commandline.contains( (const char*)"cdcl_reset_backup" ) ) reset_backup = ( 1 == atoi( commandline.get( (const char*)"cdcl_reset_backup" ).c_str() ) );
	if( commandline.contains( (const char*)"cdcl_probing_mode" ) ) probingMode = atoi( commandline.get( (const char*)"cdcl_probing_mode" ).c_str() );
	if( commandline.contains( (const char*)"cdcl_res_max_agi" ) ) restartAgilityThreshold = atof( commandline.get( (const char*)"cdcl_res_max_agi" ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) ){
		std::cerr << "=== cdcl search information ===" << std::endl;
		std::cerr << " parameter          pos.value info" << std::endl;
		
		std::cerr << " cdcl_verbose       0,1       print more information while searching" << std::endl;
		std::cerr << " cdcl_dump_learnts  0-n       write learnt clauses smaller than number to learnt.cnf, -1 = all clauses" << std::endl;
		std::cerr << " cdcl_use_learnts   0,1       treat clauses in learnt.cnf as learnt clauses, add them before search" << std::endl;
		std::cerr << " cdcl_SCC           0,1       calculate number of variable SCCs at every restart" << std::endl;
		std::cerr << " cdcl_simp          0,1       simplify tree at restart" << std::endl;
		std::cerr << " cdcl_init_simps    0-n       simplification rounds before search" << std::endl;
		std::cerr << " cdcl_reset_backup  0,1       reset backup polarities at restart" << std::endl;
		std::cerr << " cdcl_probing_mode  0,1       propagate using probing 0=none, 1=lit+inv, 2=all binary clauses" << std::endl;
#ifdef ASSI_AGILITY
		std::cerr << " cdcl_res_max_agi   0-1       upper agility to still allow a restart" << std::endl;
#endif
		std::cerr << std::endl;
	}
#endif
#endif
}

#endif
