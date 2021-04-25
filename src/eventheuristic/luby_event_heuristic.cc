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



#include "eventheuristic/luby_event_heuristic.h"



LubyEventHeuristic::LubyEventHeuristic( char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline ):
	base ( 32),
	group (1),
	group_element ( 0),
	last_event ( 0),
	strict ( true)
{
	
	event = event_name;
	
	set_parameter(event_name, commandline );
}


LubyEventHeuristic::~LubyEventHeuristic()
{
}

void LubyEventHeuristic::restart_done()
{

}

bool LubyEventHeuristic::required( uint32_t conflicts, uint32_t decisions, const searchData& sd)
{
	
	uint64_t threshold = group_element == 0 ? base + last_event : (1 << (group_element-1)) * base + last_event;
	if ( conflicts < threshold ) return false;

	last_event = (strict) ? threshold : conflicts;

	if( group >= group_element ){
		group_element++;
	} else {
		group++;
		group_element = 0;
	}

	
	return true;
}

void LubyEventHeuristic::clauses_added( const VEC_TYPE(CL_REF)& clauses, const searchData& search )
{
}

void LubyEventHeuristic::clauses_removed( const VEC_TYPE(CL_REF)& clauses )
{

}

void LubyEventHeuristic::set_parameter( char* event_name, const StringMap& commandline )
{
	const uint32_t size = strlen(event_name) + 80;
	char* param_name = (char*)MALLOC( sizeof(char) * ( size + 1 ) );



#ifdef USE_COMMANDLINEPARAMETER
	strcpy( param_name, event_name );
	strcat( param_name, "_luby_base" );
	if( commandline.contains( param_name ) ) base = atoi( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_luby_strict" );
	if( commandline.contains( param_name ) ) strict = 0 != atoi( commandline.get( param_name ).c_str() );

#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== " << event_name << " luby event information ===" << std::endl;
		std::cerr << " parameter     pos.value  info" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_luby_base" );
		std::cerr << " " << param_name <<"    1 - n   base of the luby serie" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_luby_strict" );
		std::cerr << " " << param_name <<"   1 - n   strictly follow luby or take current event counter into account" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
	
	free(param_name);
}

#ifdef COMPILE_LIBRARY

extern "C" LubyEventHeuristic* create_eventheuristic(char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline){
	return new LubyEventHeuristic(  event_name, clause_set, commandline );
}

extern "C" void destroy_eventheuristic( LubyEventHeuristic* eventheuristic ){
	delete eventheuristic;
}
#endif
