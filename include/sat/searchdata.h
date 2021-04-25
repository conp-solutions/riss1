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


#ifndef _SEARCHDATA_H
#define _SEARCHDATA_H

#include <iostream>

#include "types.h"
#include "defines.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/stack_macros.h"
#include "macros/vector_macros.h"
#include "structures/c_assignment.h"
#include "structures/c_boolarray.h"
#include "structures/literal_system.h"

using namespace std;





class reasonStruct{
	uint32_t data;
public:
	
	reasonStruct() : data(0){}
	
	
	explicit reasonStruct(const CL_REF reason) : data( reason << 1 ){}
	
	
	explicit reasonStruct(const lit_t lit, const bool isLit) : data( (lit << 1) + 1 ){}
	reasonStruct& operator=(const reasonStruct& other ){ data = other.data; return *this;}
	
	
	bool isLit() const { return ((data & 1) == 1); }
	lit_t getLit() const { return data >> 1; }
	CL_REF getCL() const { return data >> 1; }
	bool empty() const { return (data == 0); }
} WATCH_STRUCT_PACK ;



class conflictStruct{
	uint32_t data[2];
public:
	
	conflictStruct(){ data[0] = 0; data[1] = 0;}
	
	explicit conflictStruct(const CL_REF clause) { data[0] = clause; data[1] = 0; }
	
	explicit conflictStruct(const lit_t propagated, const lit_t impliedConflict) { data[0] = propagated << 1; data[1] = (impliedConflict << 1) + 1; }
	conflictStruct& operator=(const conflictStruct& other ){ data[0] = other.data[0]; data[1] = other.data[1]; return *this;}
	
	
	bool isLit() const { return ((data[1] & 1) == 1); }
	lit_t getLit(uint32_t ind) const { return data[ind] >> 1; }
	lit_t getPropagated() const { return data[0] >> 1; }
	lit_t getConflicted() const { return data[1] >> 1; }
	CL_REF getCL() const { return data[0]; }
	bool empty() const { return (data[0] == 0 && data[1] == 0); }
} WATCH_STRUCT_PACK ;


struct var1Data{
	int32_t level;
	reasonStruct reason;
}  WATCH_STRUCT_PACK ;


class searchData{
private:
	searchData& operator=(const searchData& other){ return *this; }

public:
	STACK_TYPE( lit_t ) trail;	
#ifndef COMPACT_VAR_DATA
	reasonStruct* reason;	
	int32_t* level;		
#else
	var1Data* var1;	
#endif
	assi_t assi;	
	int32_t current_level;	
	uint32_t var_cnt; 
	uint32_t original_var_cnt; 
	uint32_t assigned_variables; 
	double* activities;	
	boolarray_t eliminated;	
	
	double globalLearntAvg;	
	double currentLearntAvg;	
	double globalLearntMin;	
	double currentLearntMin;	
	double currentLearnts;	
	
	
	

	searchData():
#ifndef COMPACT_VAR_DATA
	reason(0),
	level(0),
#else
	var1(0),
#endif
	assi(0),
	var_cnt(0),
	original_var_cnt(0),
	assigned_variables(0),
	activities(0)
	{
		trail = STACK_CREATE( lit_t );
		globalLearntAvg=0;
		currentLearntAvg=0;
		globalLearntMin=var_cnt;
		currentLearntMin=var_cnt;
	}

	
	void init( searchData& other){
		var_cnt = other.var_cnt;
		original_var_cnt = other.original_var_cnt;
		STACK_COPY( lit_t, trail, other.trail );
#ifndef COMPACT_VAR_DATA
		reason = (CL_REF*)MALLOC( sizeof(reasonStruct) * (var_cnt + 1) );
		memcpy( reason, other.reason, var_cnt * sizeof(reasonStruct) );
		level = (int*)MALLOC(sizeof(int) * (var_cnt + 1));
		memcpy( level, other.level,sizeof(int) * (var_cnt + 1));
#else
		var1 = (var1Data*)MALLOC(sizeof(var1Data) * (var_cnt + 1));
		memcpy( var1, other.var1, sizeof(var1Data) * (var_cnt + 1) );
#endif
		current_level=0;
		assigned_variables = 0;
		
		assi = assi_copy(other.assi, var_cnt);
		
		
		activities = other.activities;
		
		globalLearntAvg=0;
		currentLearntAvg=0;
		globalLearntMin=var_cnt;
		currentLearntMin=var_cnt;
		currentLearnts=0;
	}

	
	searchData(uint32_t var_cnt)
	{
		this->var_cnt = var_cnt;
		original_var_cnt = var_cnt;
		assi = assi_create(var_cnt);
		trail = STACK_CREATE( lit_t );
#ifndef COMPACT_VAR_DATA
		reason = (CL_REF*)MALLOC(sizeof(reasonStruct) * (var_cnt + 1));
		memset( this->reason, 0, var_cnt * sizeof(reasonStruct) );
		level = (int*)MALLOC(sizeof(int) * (var_cnt + 1));
		for(uint32_t i = 0; i<(var_cnt+1); ++i ) this->level[i] = -1;
#else
		var1 = (var1Data*)MALLOC(sizeof(var1Data) * (var_cnt + 1));
		for(uint32_t i = 0; i<(var_cnt+1); ++i ) {var1[i].level = -1; var1[i].reason = reasonStruct();}
#endif
		current_level=0;
		assigned_variables = 0;
		
		
		activities = (double*) MALLOC( sizeof(double) * (var_cnt+1) );
		memset( activities, 0, sizeof(double) * (var_cnt+1) );
		
		eliminated = boolarray_create(var_cnt + 1);
		
		globalLearntAvg=0;
		currentLearntAvg=0;
		globalLearntMin=var_cnt;
		currentLearntMin=var_cnt;
		currentLearnts=0;
	}

	
	searchData( searchData& other){
		var_cnt = other.var_cnt;
		original_var_cnt = other.original_var_cnt;
		STACK_COPY( lit_t, trail, other.trail );
#ifndef COMPACT_VAR_DATA
		reason = (CL_REF*)MALLOC( sizeof(reasonStruct) * (var_cnt + 1) );
		memcpy( reason, other.reason, var_cnt * sizeof(reasonStruct) );
		level = (int*)MALLOC(sizeof(int) * (var_cnt + 1));
		memcpy( level, other.level,sizeof(int) * (var_cnt + 1));
#else
		var1 = (var1Data*)MALLOC(sizeof(var1Data) * (var_cnt + 1));
		memcpy( var1, other.var1, sizeof(var1Data) * (var_cnt + 1) );
#endif
		current_level=0;
		assigned_variables = 0;
		
		
		activities = other.activities;
		
		globalLearntAvg=0;
		currentLearntAvg=0;
		globalLearntMin=var_cnt;
		currentLearntMin=var_cnt;
		currentLearnts=0;
	}


	
	~searchData(){
		STACK_DESTROY( lit_t, trail );
#ifndef COMPACT_VAR_DATA
		if( reason != 0 ) free(reason);
		if(level!=0) free(level);
#else
		if(var1 != 0) free(var1);
#endif
		if( activities != 0 ) free( activities );
		
		boolarray_destroy(eliminated);
		
		assi_destroy(assi,var_cnt);
	}
	
	
	
	void extend( const uint32_t newVars ){

		
		
	#ifndef COMPACT_VAR_DATA
		reason = (reasonStruct*)realloc(reason, (newVars + 1)* sizeof(reasonStruct) );
		level = (int32_t*) realloc( level, (newVars + 1) * sizeof(int32_t) );
	#else
		var1 = (var1Data*) realloc (var1, (newVars + 1) * sizeof(var1Data) );
	#endif
	
		assi_extend(assi, var_cnt, newVars);
		activities = (double*) realloc(activities, (newVars+1) * sizeof(double) );

		
		if( newVars > var_cnt ){
			const uint32_t diff = newVars - var_cnt;
		#ifndef COMPACT_VAR_DATA
			memset( reason+var_cnt + 1, 0, diff * sizeof( reasonStruct ) );
			for( uint32_t i = 0; i < diff; ++i ){
				level[var_cnt+i] = -1;
			}
		#else
			for( uint32_t i = 0; i < diff; ++i ){
				var1[var_cnt+i].level = -1;
				var1[var_cnt+i].reason = reasonStruct();
			}
		#endif
			memset( activities+var_cnt + 1, 0, diff * sizeof( double ) );

			boolarray_extend(eliminated, var_cnt+1, newVars+1);
		}
		
		
		var_cnt = newVars;
	}
	
	void resetLocalCounter() {
		currentLearntAvg=0;
		currentLearntMin=var_cnt;
		currentLearnts=0;
	}
		
};

#endif
