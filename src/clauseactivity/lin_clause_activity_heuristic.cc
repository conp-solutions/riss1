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


#include "clauseactivity/lin_clause_activity_heuristic.h"

LinClauseActivity::LinClauseActivity( const StringMap& commandline) :
	init_mode(1),
	inc(0),
	inc_decay(1.1),
	care_undefined( true )
{
	set_parameter( commandline );
}

LinClauseActivity::~LinClauseActivity()
{

}

void LinClauseActivity::set_learned_activity(searchData& search, VEC_TYPE( CL_REF )& clauses )
{

	if( init_mode == 1 )
	{
		
		float implied = (search.current_level == 0) ? 1 : STACK_SIZE( lit_t, search.trail ) / (float)(search.current_level);
		implied = (implied == 0) ? 1 : implied;

			
		
		for( uint32_t i = 0 ; i <VEC_SIZE( CL_REF, clauses ); ++i )
		{
			CLAUSE& cl = gsa.get(clauses[i]);
			
			int levels[ CL_SIZE(cl) ];
			for( uint32_t j = 0 ; j < CL_SIZE(cl); j++ ) levels[j] = search.VAR_LEVEL( var( CL_GET_LIT( cl, j ) ) );
			
			int count = 1;
			mergesort<int>( levels, CL_SIZE(cl) );
			
			int minusOneCount = 0;
			for( uint32_t j = 0 ; j < CL_SIZE(cl) - 1; j++ )
			{
				if( levels[j] == -1 ) minusOneCount++;
				else if( levels[j] != levels[j+1] ) count++;
			}

			
			if( care_undefined ) count += (float)minusOneCount/implied;
			count = (count == 0 ) ? 1 : count;
			
			CL_SET_ACTIVITY( cl, 1.0f / (float)count );
		}
	} else {
		
		for( uint32_t i = 0 ; i <VEC_SIZE( CL_REF, clauses ); ++i )
		{
			CLAUSE& cl = gsa.get(clauses[i]);
			CL_SET_ACTIVITY( cl, 1.0f / (float)CL_SIZE(cl) );
		}
	}

}

bool LinClauseActivity::inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  )
{

	bool ret = false;
	for( uint32_t i = 0 ; i <VEC_SIZE( CL_REF, clauses ); ++i )
	{
		CLAUSE& cl = gsa.get( clauses[i] );
		
		if( CL_IS_LEARNT( cl ) ){
			CL_INC_ACTIVITY( cl, inc );
			if(  CL_GET_ACTIVITY( cl ) > overflow_divisor() ) ret = true;
		}
	}
	
	inc *= inc_decay;
	
	if(ret == true )
	{
		inc /= overflow_divisor();
	}
	return ret;
}

void LinClauseActivity::restart_done()
{
	
}

void LinClauseActivity::set_parameter( const StringMap& commandline )
{
	

#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"lin_activity_init_mode" ) ) init_mode = 0 != atoi( commandline.get( (const char*)"lin_activity_init_mode" ).c_str() );
	if( commandline.contains( (const char*)"lin_activity_inc" ) ) inc = 0 != atof( commandline.get( (const char*)"lin_activity_inc" ).c_str() );
	if( commandline.contains( (const char*)"lin_activity_inc_decay" ) ) inc_decay = 0 != atof( commandline.get( (const char*)"lin_activity_inc_decay" ).c_str() );
	if( commandline.contains( (const char*)"lin_activity_care_undef" ) ) care_undefined = 0 != atoi( commandline.get( (const char*)"lin_activity_care_undef" ).c_str() );
	
#endif
#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== lin clause activity heuristic information ===" << std::endl;
		std::cerr << " parameter              pos.value  info" << std::endl;
		std::cerr << " lin_activity_init_mode  0,1        how to set init activity" << std::endl;
		std::cerr << "                                    0 inverse of clause length" << std::endl;
		std::cerr << "                                    1 inverse of number of decision levels in clause" << std::endl;
		std::cerr << " lin_activity_inc        0-n        inc of activity if used in reasoning" << std::endl;
		std::cerr << " lin_activity_inc_decay  0-n        at restart inc is increased by this value" << std::endl;		
		std::cerr << " lin_activity_care_undef 0,1        take undefined variables into account in learnt clauses" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
}

#ifdef COMPILE_LIBRARY

extern "C" LinClauseActivity* create_clauseactivityheuristic( const StringMap& commandline){
	return new LinClauseActivity( commandline );
}

extern "C" void destroy_clauseactivityheuristic( LinClauseActivity* heuristic ){
	delete heuristic;
}
#endif

