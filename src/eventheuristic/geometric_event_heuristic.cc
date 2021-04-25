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



#include "eventheuristic/geometric_event_heuristic.h"



GeometricEventHeuristic::GeometricEventHeuristic( char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline ):
	last_event ( 0),
	current_max (1000),
	current_increment ( (4.0 / 3.0)),
	inner_start (1000),
	mode(0),
	strict(false),
	nested(false)
{
	
	event = event_name;
	
	set_parameter(event_name, commandline );
}


GeometricEventHeuristic::~GeometricEventHeuristic()
{
}


bool GeometricEventHeuristic::required( uint32_t conflicts, uint32_t decisions,  const searchData& sd)
{
	
	if ( ! (conflicts >= current_max ) ) return false;

	
	current_max -= last_event;
		
	if( !nested ){
		current_max = (unsigned)( (float)current_max * current_increment);
	} else {
		if (current_max >= outer_max){
			outer_max = (unsigned)( (float)outer_max * current_increment);
			current_max = inner_start;
		}
		else current_max = (unsigned)( (float)current_max * current_increment);
	}

	
	last_event = conflicts;	
	current_max += last_event;

	
	switch( mode )
	{
		case 0:	
			break;
			
		default: assert( 1==0 );
	}
	
	
	return true;
}

void GeometricEventHeuristic::clauses_added( const VEC_TYPE(CL_REF)& clauses, const searchData& search )
{
}

void GeometricEventHeuristic::clauses_removed( const VEC_TYPE(CL_REF)& clauses )
{

}

void GeometricEventHeuristic::restart_done()
{

}

void GeometricEventHeuristic::set_parameter( char* event_name, const StringMap& commandline )
{
	const uint32_t size = strlen(event_name) + 80;
	char* param_name = (char*)MALLOC( sizeof(char) * ( size + 1 ) );



#ifdef USE_COMMANDLINEPARAMETER
	strcpy( param_name, event_name );
	strcat( param_name, "_geometric_max" );
	if( commandline.contains( param_name ) ) current_max = atoi( commandline.get( param_name ).c_str() );
	inner_start = current_max;
	last_event = 0;

	strcpy( param_name, event_name );
	strcat( param_name, "_geometric_inc" );
	if( commandline.contains( param_name ) ) current_increment = atof( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_geometric_mode" );
	if( commandline.contains( param_name ) ) mode = atof( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_geometric_outer_max" );
	if( commandline.contains( param_name ) ) outer_max = atoi( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_geometric_nested" );
	if( commandline.contains( param_name ) ) nested = 0 != atoi( commandline.get( param_name ).c_str() );

#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== " << event_name << " geometric event information ===" << std::endl;
		std::cerr << " parameter     pos.value  info" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_geometric_max" );		
		std::cerr << " " << param_name <<"    1 - n   if this counter is reached, event is fired and max is set higher" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_geometric_inc" );
		std::cerr << " " << param_name <<"    1 - n   geometric_max = geometric_max * geometric_inc at fired events" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_geometric_mode" );
		std::cerr << " " << param_name <<"   0       geometric_inc is kept at every restart" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_geometric_nested" );
		std::cerr << " " << param_name <<"   0,1     nest a smaller inner geometric sery in a bigger one" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_geometric_outer_max" );
		std::cerr << " " << param_name <<"   0       start value for the bigger geometric sery" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
	
	free(param_name);
}

#ifdef COMPILE_LIBRARY

extern "C" GeometricEventHeuristic* create_eventheuristic(char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline){
	return new GeometricEventHeuristic(  event_name, clause_set, commandline );
}

extern "C" void destroy_eventheuristic( GeometricEventHeuristic* eventheuristic ){
	delete eventheuristic;
}
#endif
