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


#ifndef _STATISTICS_H
#define _STATISTICS_H


#include "macros/vector_macros.h"

#include "defines.h"

#ifdef COLLECT_STATISTICS


#define STATISTIC_MODE 1

#define STAT_INIT( commandline, cls_count, var1_count ) Statistics::inst().setBenchmark( commandline, cls_count, var1_count )
#define STAT_SNAP() Statistics::inst().getSnapshot()
#define STAT_PREPRO( cls_count, var1_count ) Statistics::inst().setPreprocessed( cls_count, var1_count )
#define STAT_SAT( sat ) Statistics::inst().setSatisfiable( sat)

#define STAT_LEARNED( cls ) Statistics::inst().learned_clause( (VEC_TYPE( uint32_t )&) cls )
#define STAT_DECIDED( lit1 ) Statistics::inst().decision( lit1 )

#define STAT_READLIT( pos ) Statistics::inst().read_lit1( pos )
#define STAT_WRITELIT( pos ) Statistics::inst().write_lit1( pos )
#define STAT_SWAPLITS( pos1, pos2 ) Statistics::inst().swap_lits( pos1, pos2)
#define STAT_ACCCLAUSE( cls )	Statistics::inst().accessClause( cls )

#define STAT_DORESTART() Statistics::inst().restart()
#define STAT_STARTTIMER( event ) 0
#define STAT_CONTINUETIMER( id )
#define STAT_STOPTIMER Statistics::inst().stopTimer( id ) 0



#define STAT_STOREDATA( directory ) Statistics::inst().addDataToDirectory( directory )
#define STAT_STOREINFO( directory ) Statistics::inst().writeHeaderToDirectory( directory )

#else

#define STAT_INIT( commandline, cls_count, var1_count )
#define STAT_SNAP()
#define STAT_PREPRO( cls_count, var1_count ) 
#define STAT_SAT( sat ) 

#define STAT_LEARNED( cls )
#define STAT_DECIDED( lit1 )

#define STAT_READLIT( pos )
#define STAT_WRITELIT( pos ) 
#define STAT_SWAPLITS( pos1, pos2 )
#define STAT_ACCCLAUSE( cls )
#define STAT_DORESTART()
#define STAT_STARTTIMER( event ) 0
#define STAT_CONTINUETIMER( id )
#define STAT_STOPTIMER Statistics::inst().stopTimer( id ) 0

#define STAT_STOREDATA( directory )
#define STAT_STOREINFO( directory )


#endif




#define CLS_LEN 8

#define TRACE_RESTARTS 10

#include <inttypes.h>	

#include "types.h"

#include "utils/stringmap.h"	
#include "utils/microtime.h"
#include "info.h"	

#include <string>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

struct timer_data{
	std::string event;
	uint64_t nanoseconds;
	uint64_t last_duration;
	timer_data() : nanoseconds(0), last_duration(0){}
};



class Statistics{
	
	struct data{
		uint64_t cls_read_access[CLS_LEN+2];
		uint64_t cls_write_access[CLS_LEN+2];
		uint64_t cls_swap[(CLS_LEN+2)*(CLS_LEN+2)]; 
		
		std::vector<timer_data> timer;
		
		uint64_t clause_accesses;
		double dist_sum;
		double dist_avg;
		double dist_var1;
		double dist_med;
		
		uint64_t foundUnits;
		uint64_t foundDuals;
		
		uint64_t currentRuntime;
		
		uint32_t conflicts;
		uint32_t decisions;
		
		uint32_t currentRestarts;
		uint32_t memory_per_restart[TRACE_RESTARTS];
		data() : clause_accesses(0), dist_sum(0),dist_avg(0),dist_var1(0),dist_med(0),foundUnits(0),foundDuals(0),currentRuntime(0),conflicts(0),decisions(0),currentRestarts(0){
			for( uint32_t i = 0 ; i < CLS_LEN+2; ++i )
			{
				cls_read_access[i]=0;
				cls_write_access[i]=0;
				for( uint32_t j = 0; j < (CLS_LEN+2); ++j ){
					cls_swap[ i * (CLS_LEN+2) + j ] = 0;
				}
			}
			for( uint32_t i = 0 ; i < TRACE_RESTARTS; ++i )
			{
				memory_per_restart[i] = 0;
			}
		}
	};
	
	data my_data;
	
	
	std::string benchmark;
	std::string commandline;

	uint64_t buildtime;
	uint32_t cls;
	uint32_t var1s;
	uint32_t prep_cls;
	uint32_t prep_var1s;
	
	void* last_access;
	bool satisfiable;
private:
	explicit Statistics() {
		my_data.currentRuntime = get_microseconds();
		benchmark="";
		commandline="";
	}
	
	explicit Statistics( const Statistics& );

public:
	
	static Statistics& inst();


	
	Statistics::data getSnapshot();
	
	void setBenchmark( const StringMap& commandline, uint32_t cls_count, uint32_t var1_count );
	
	void setPreprocessed( uint32_t cls_count, uint32_t var1_count );
	
	void setSatisfiable( bool sat);
	
	void learned_clause( VEC_TYPE( uint32_t )& cls );
	
	void decision( lit_t lit1 );
	

	void read_lit1(uint32_t pos);
	void write_lit1(uint32_t pos);
	void swap_lits(uint32_t pos1, uint32_t pos2);
	void accessClause( void* cls );
	void restart();
	

	
	int startTimer( std::string event );
	
	void continueTimer( int id );
	
	uint64_t stopTimer( int id );
	

	
	
	void addDataToDirectory( std::string directory );
	
	void writeHeaderToDirectory( std::string directory );
	
	int handle_solverconfig( const std::string& filename );
	int handle_benchmark( const std::string& filename, int solverid );
	int handle_search( const std::string& filename, int solverid );
	int handle_timer( const std::string& filename, int solverid );
	int handle_clause( const std::string& filename, int solverid );
};




#endif
