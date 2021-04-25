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



#include "eventheuristic/dynamic_event_heuristic.h"



DynamicEventHeuristic::DynamicEventHeuristic( char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline ):
windowSize(100),
factor(0.7f),
windowLevels(0),
currentIndex(0),
windowSum(0),
recentMean(0),
recentConflicts(0),
lastConflict(0)
{
	
	event = event_name;
	
	set_parameter(event_name, commandline );
	
	windowLevels = (int32_t*) MALLOC( sizeof(int32_t) * windowSize );
	memset( windowLevels, 0, sizeof(int32_t) * windowSize );
}


DynamicEventHeuristic::~DynamicEventHeuristic(){
	if(windowLevels != 0 ) free( windowLevels );
	windowLevels = 0;
}


bool DynamicEventHeuristic::required( uint32_t conflicts, uint32_t decisions,  const searchData& sd){
	
	if( lastConflict == conflicts ) return false;
	else lastConflict = conflicts;
	recentConflicts++;	
	
	uint32_t tmp = currentIndex;

	currentIndex = (currentIndex + 1 >= windowSize) ? 0 : currentIndex + 1;
	windowSum -= windowLevels[tmp];	
	windowSum += sd.current_level;
	windowLevels[tmp] = sd.current_level;
	
	
	recentMean += sd.current_level;


	
	if( recentConflicts <= windowSize ) return false;

	
	if( (double)windowSum / (double)windowSize <= recentMean / (double)recentConflicts ){
		return false;
	} else {
		
		resetValues();
		return true;
	}
}

void DynamicEventHeuristic::resetValues(){
	memset( windowLevels, 0, sizeof(int32_t) * windowSize );
	windowSum = 0;
	recentMean = 0;
	currentIndex = 0;
	recentConflicts = 0;
}

void DynamicEventHeuristic::clauses_added( const VEC_TYPE(CL_REF)& clauses, const searchData& search )
{
}

void DynamicEventHeuristic::clauses_removed( const VEC_TYPE(CL_REF)& clauses )
{

}

void DynamicEventHeuristic::restart_done()
{
}

void DynamicEventHeuristic::set_parameter( char* event_name, const StringMap& commandline )
{
	const uint32_t size = strlen(event_name) + 80;
	char* param_name = (char*)MALLOC( sizeof(char) * ( size + 1 ) );


#ifdef USE_COMMANDLINEPARAMETER
	strcpy( param_name, event_name );
	strcat( param_name, "_dynamic_size" );
	if( commandline.contains( param_name ) ) windowSize = atoi( commandline.get( param_name ).c_str() );

	strcpy( param_name, event_name );
	strcat( param_name, "_dynamic_factor" );
	if( commandline.contains( param_name ) ) factor = atof( commandline.get( param_name ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== " << event_name << " dynamic event information ===" << std::endl;
		std::cerr << " parameter     pos.value  info" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_dynamic_size" );		
		std::cerr << " " << param_name <<"      1 - n   number of conflict levels that are taken into account" << std::endl;
		strcpy( param_name, event_name );
		strcat( param_name, "_dynamic_factor" );
		std::cerr << " " << param_name <<"    0-1     windowMean * factor > mean -> restart" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
	
	free(param_name);
}

#ifdef COMPILE_LIBRARY

extern "C" DynamicEventHeuristic* create_eventheuristic(char* event_name, const VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline){
	return new DynamicEventHeuristic(  event_name, clause_set, commandline );
}

extern "C" void destroy_eventheuristic( DynamicEventHeuristic* eventheuristic ){
	delete eventheuristic;
}
#endif
