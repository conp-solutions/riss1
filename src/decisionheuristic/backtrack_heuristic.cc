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



#include "decisionheuristic/backtrack_heuristic.h"


#include <iostream>


BacktrackHeuristic::BacktrackHeuristic(const StringMap& commandline):
backtrack_mode(0),
condition_mode(0),
recalcDistance(2000),
shrinkLast(false)
{
	
	meanSum = 0;
	squareSum = 0;
	realConflictClauses = 0;
	threshold = 95;
	lastWasShrinked = false;

	set_parameter( commandline );
}

BacktrackHeuristic::~BacktrackHeuristic()
{}


int32_t BacktrackHeuristic::backtrack_to( searchData& sd, VEC_TYPE(CL_REF)& clauses){

	int32_t backjumpLevel = sd.current_level + 1;
	
	if( backtrack_mode == 2 || backtrack_mode == 3 ) backjumpLevel = shrink( sd, clauses);

	
	if( backtrack_mode == 1 ){
		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
			int32_t L1 = sd.current_level;	
			const CLAUSE& cl = gsa.get( clauses[j] );
			for (uint32_t i = 1; i < CL_SIZE(cl); i ++) {
				const lit_t lit1 = CL_GET_LIT(cl, i);
				const int32_t level = sd.VAR_LEVEL( var(lit1) );
				if (level < L1)	L1 = level;
			}
			backjumpLevel = (L1-1<backjumpLevel) ? L1-1 : backjumpLevel;
		}
	}

	
	if( backtrack_mode == 0 || (backjumpLevel == sd.current_level + 1) ){	
		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
			int32_t L1 = -1;
			int32_t L2 = -1;
			const CLAUSE& cl = gsa.get( clauses[j] );
			for (uint32_t i = 0; i < CL_SIZE(cl); i ++) {
				const lit_t lit1 = CL_GET_LIT(cl, i);
				const int32_t level = sd.VAR_LEVEL( var(lit1) );
				if (level > L1) {
					L2 = L1;
					L1 = level;
				} else {
					if( level > L2 ) L2 = level;
				}
			}
			backjumpLevel = (L2<backjumpLevel) ? L2 : backjumpLevel;
		}
	}
	
	
	backjumpLevel = (backjumpLevel < 0 ) ? 0 : backjumpLevel;

	

	sd.current_level = backjumpLevel;	
	return backjumpLevel;
}

bool BacktrackHeuristic::check_shrink_condition( searchData& sd, VEC_TYPE(CL_REF)& clauses){
	
	if( !shrinkLast && lastWasShrinked ) return false;
	
	updateData( clauses );
	
	if( condition_mode == 0 ){	
		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
			const CLAUSE& cl = gsa.get( clauses[j] );
			int32_t levels[CL_SIZE(cl)];
			for (uint32_t i = 1; i < CL_SIZE(cl); i ++) {
				levels[i] = sd.VAR_LEVEL( var( CL_GET_LIT(cl, i) ) );
			}
			insertionsort<int32_t>(levels, CL_SIZE(cl) );
			uint32_t i = 0;
			for( ; i < CL_SIZE(cl) - 1; ++i ){
				if( levels[i] == levels[i+1] )break;	
			}
			if( i == CL_SIZE(cl) ){	
				if( CL_SIZE(cl) > threshold ) return true;	
			}
		}
		return false;
	}
	if( condition_mode == 1 ){
		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
			const CLAUSE& cl = gsa.get( clauses[j] );
			if( CL_SIZE(cl) > threshold ) return true;	
		}
		
	}
	
	return false;
}

int32_t BacktrackHeuristic::shrink( searchData& sd, VEC_TYPE(CL_REF)& clauses){
	int32_t backjumpLevel = sd.current_level + 1;

	
	if( check_shrink_condition( sd, clauses ) ){
		
		if( backtrack_mode == 2){
			for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
				int32_t L1 = sd.current_level;	
				const CLAUSE& cl = gsa.get( clauses[j] );
				for (uint32_t i = 1; i < CL_SIZE(cl); i ++) {
					const lit_t lit1 = CL_GET_LIT(cl, i);
					const int32_t level = sd.VAR_LEVEL( var(lit1) );
					if (level < L1)	L1 = level;
				}
				backjumpLevel = (L1-1<backjumpLevel) ? L1-1 : backjumpLevel;
			}
		}
		
		if( backtrack_mode == 3){
			
			for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
				const CLAUSE& cl = gsa.get( clauses[j] );
				int32_t levels[CL_SIZE(cl)];
				int32_t L1 = sd.current_level+1;
				levels[0]=sd.current_level;
				for (uint32_t i = 1; i < CL_SIZE(cl); i ++) {
					levels[i] = sd.VAR_LEVEL( var( CL_GET_LIT(cl, i) ) );
				}
				insertionsort<int32_t>(levels, CL_SIZE(cl) );
				for( uint32_t i = CL_SIZE(cl) - 1; i > 1; --i ){
					
					if( levels[i] > levels[i-1] + 1 ) {L1 = levels[i-1]; break;}
				}

				backjumpLevel = (L1-1<backjumpLevel) ? L1 : backjumpLevel;
			}
		}
	}
	
	
	lastWasShrinked = (backjumpLevel != sd.current_level + 1);

	return backjumpLevel;
}

void BacktrackHeuristic::updateData( VEC_TYPE(CL_REF)& clauses ){
	realConflictClauses += VEC_SIZE( CL_REF, clauses );;
	for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF, clauses ); j++ ){
		const CLAUSE& cl = gsa.get( clauses[j] );
		meanSum += CL_SIZE( cl );
		squareSum += CL_SIZE( cl ) * CL_SIZE( cl );
	}
	conflicts ++;
	if( conflicts >= recalcDistance ) recalc();
}


void BacktrackHeuristic::recalc(){

	double mean = meanSum / ( (double)conflicts );
	double stdev = squareSum  / ( (double)conflicts ) - mean;
	float center = mean + 0.5 * stdev;
	float ulimit = mean + stdev;
	if ( threshold > center ) threshold -= 5;
	else { if ( threshold < center ) threshold += 5; }
	if( threshold > ulimit ) threshold = ulimit;
	if( threshold < 5 ) threshold = 5;

	
	conflicts = 0;
	meanSum = 0;
	squareSum = 0;
	realConflictClauses = 0;
}

void BacktrackHeuristic::set_parameter( const StringMap& commandline )
{

#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"bj_sh_last" ) )   shrinkLast = 0 != atoi( commandline.get( (const char*)"bj_sh_last" ).c_str() );
	if( commandline.contains( (const char*)"bj_level" ) )     backtrack_mode = atoi( commandline.get( (const char*)"bj_level" ).c_str() );
	if( commandline.contains( (const char*)"bj_sh_cond" ) )   condition_mode = atoi( commandline.get( (const char*)"bj_sh_cond" ).c_str() );
	if( commandline.contains( (const char*)"bj_sh_recalc" ) ) recalcDistance = atoi( commandline.get( (const char*)"bj_sh_recalc" ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== backjump heuristic information ===" << std::endl;
		std::cerr << " parameter    pos.value info" << std::endl;
		std::cerr << " bj_level     0-3       choose backjump level" << std::endl;
		std::cerr << "                        0 shortest backjump distance" << std::endl;
		std::cerr << "                        1 longest backjump distance" << std::endl;
		std::cerr << "                        2 longest shrinking" << std::endl;
		std::cerr << "                        3 dynamic shrinking (chaff 2004)" << std::endl;
		std::cerr << " bj_sh_cond   0,1       shrinking condition" << std::endl;
		std::cerr << "                        0 variable per desc.-level" << std::endl;
		std::cerr << "                        1 clause length" << std::endl;
		std::cerr << " bj_sh_last   0,1       shrink, if last was shrinked" << std::endl;
		std::cerr << " bj_sh_recalc 1-n       conflicts until threshold is recalced" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
}
