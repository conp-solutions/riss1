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



#include "eventheuristic/cls_ratio_event_heuristic.h"



ClsRatioEventHeuristic::ClsRatioEventHeuristic( char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline ) :
	learntsize_factor ( (double)1 / (double)3 ), 
	learntsize_inc ( 1.1 )		
{
	
	event = event_name;
	
	set_parameter(event_name, commandline );
	
	formula_cls_cnt = VEC_SIZE( CL_REF, (*clause_set) );
	
	
	if( formula_cls_cnt > 100000 ){
		formula_cls_cnt = 100000;
	}
	
	learnt_cnt = 0;
}


ClsRatioEventHeuristic::~ClsRatioEventHeuristic()
{
}


bool ClsRatioEventHeuristic::required( uint32_t conflicts, uint32_t decisions, const searchData& sd)
{

  return ( (double)learnt_cnt - (double)STACK_SIZE(lit_t, sd.trail ) >= (double)formula_cls_cnt * learntsize_factor );
}

void ClsRatioEventHeuristic::clauses_added( const VEC_TYPE(CL_REF)& clauses, const searchData& search )
{
	learnt_cnt += VEC_SIZE( CL_REF, clauses );
}

void ClsRatioEventHeuristic::clauses_removed( const VEC_TYPE(CL_REF)& clauses )
{

	learnt_cnt -= VEC_SIZE( CL_REF, clauses );
}

void ClsRatioEventHeuristic::restart_done()
{
	learntsize_factor *= learntsize_inc;
}

void ClsRatioEventHeuristic::set_parameter( char* event_name, const StringMap& commandline )
{
	const uint32_t size = strlen(event_name) + 80;
	char* param_name = (char*)MALLOC( sizeof(char) * ( size + 1 ) );

#ifdef USE_COMMANDLINEPARAMETER
	strcpy( param_name, event_name );
	strcat( param_name, "_size_factor" );
	if( commandline.contains( param_name ) ) learntsize_factor = atof( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_factor_inc" );
	if( commandline.contains( param_name ) ) learntsize_inc  = 0 != atof( commandline.get( param_name ).c_str() );

#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== " << event_name << " clause ratio event information ===" << std::endl;
		std::cerr << " parameter     pos.value  info" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_size_factor" );
		std::cerr << " " << param_name <<"    0 - n   ratio of learnt to formula clauses to schedule event" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_factor_inc" );
		std::cerr << " " << param_name <<"   0 - n   increment value for size_factor" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
	
	free(param_name);
}

#ifdef COMPILE_LIBRARY

extern "C" ClsRatioEventHeuristic* create_eventheuristic(char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline){
	return new ClsRatioEventHeuristic(  event_name, clause_set, commandline );
}

extern "C" void destroy_eventheuristic( ClsRatioEventHeuristic* eventheuristic ){
	delete eventheuristic;
}
#endif
