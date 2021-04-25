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




#ifndef _ASSIGNMENT_H
#define _ASSIGNMENT_H

#include <inttypes.h>	
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "defines.h"
#include "types.h"
#include "macros/vector_macros.h"
#include "structures/literal_system.h"

#include "defines.h"
#include "macros/malloc_macros.h"

#include <iostream>	








typedef void* assi_t;

assi_t assi_create(const uint32_t varCnt);


void assi_extend(assi_t& assignment, const uint32_t varCnt, const uint32_t newVars);

assi_t assi_copy(const assi_t assignment, const uint32_t variables);

void assi_copy_to(assi_t destination, const assi_t source, const uint32_t variables);

void assi_destroy(assi_t& assignment, const uint32_t variables);

void assi_clear(assi_t destination, const uint32_t varCnt);

pol_t assi_get_polarity( const assi_t assignment, const var_t variable );

void assi_set_polarity( assi_t assignment, const var_t variable, const pol_t polarity );

pol_t assi_get_backup_polarity( const assi_t assignment, const var_t variable );

void assi_set_polarity_backup( assi_t assignment, const var_t variable, const pol_t polarity );

void assi_invert_variable(assi_t assignment, const var_t variable);

void assi_undef_variable(assi_t assignment, const var_t variable);

void assi_undef_variable_backup(assi_t assignment, const var_t variable);

bool assi_variable_equal(const assi_t assignment, const var_t variable, pol_t value);




inline float& assi_get_agility(){
	static float a = 0;
	return a;
}

inline float& assi_get_aging(){
	static float g = 0.9999;
	return g;
}

inline float& assi_get_naging(){
	static float g = 1-assi_get_aging();
	return g;
}

inline void assi_update_agility(const bool newValue = false){

	assi_get_agility() *= assi_get_aging();
	if( newValue ) assi_get_agility() += assi_get_naging();

}

inline bool assi_is_sat( assi_t assignment, const lit_t l ){
	return assi_variable_equal(assignment, var(l), pol(l) );
}

inline bool assi_is_unsat( assi_t assignment, const lit_t l){
	return assi_variable_equal(assignment, var(l), pol(inv_lit(l)) );
}

inline bool assi_is_undef( assi_t assignment, const var_t v){
	return assi_variable_equal(assignment, v, UNDEF );
}


inline int32_t assi_get_equal_lits(const assi_t a, const assi_t b, assi_t before, VEC_TYPE(lit_t)& units, VEC_TYPE(lit_t)& equals, const uint32_t varCnt);



inline assi_t assi_create(const uint32_t varCnt)
{
	

	void* memory = MALLOC( (varCnt + 1) * sizeof( uint8_t ) * 2 );

	
	memset( memory, 0, (varCnt + 1) * sizeof( uint8_t ) * 2 );

	return memory;
}

inline void assi_clear(assi_t assignment, const uint32_t varCnt){
	memset( assignment, 0, (varCnt + 1) * sizeof( uint8_t ) * 2 );
}

inline assi_t assi_copy(const assi_t assignment, const uint32_t varCnt)
{
	
	void* memory = MALLOC( (varCnt + 1) * sizeof( uint8_t ) * 2);
	memcpy( memory, assignment, (varCnt + 1) * sizeof( uint8_t ) * 2 );
	return memory;
}

inline void assi_copy_to(assi_t destination, const assi_t source, const uint32_t varCnt)
{
	memcpy( destination, source, (varCnt + 1) * sizeof( uint8_t ) * 2 );
}

inline void assi_extend(assi_t& assignment, const uint32_t varCnt, const uint32_t newVars){
	
	assignment = realloc( assignment, (newVars + 1) * sizeof( uint8_t ) * 2);

	uint8_t* memory = (uint8_t*)assignment;
	
	if( newVars > varCnt ){
		
		const uint32_t diff = newVars - varCnt;
		memset( memory+ (2 * (varCnt + 1)), 0, (diff) * sizeof( uint8_t ) * 2 );
	}
}

inline void assi_destroy(assi_t& assignment, const uint32_t variables)
{
	if( assignment != 0 )
	{
		free(assignment);
		assignment = 0;
	}
}

inline pol_t assi_get_polarity( const assi_t assignment, const var_t variable )
{
	
	return (pol_t)( ( (uint8_t*)assignment)[variable * 2] );
}

inline void assi_set_polarity( assi_t assignment, const var_t variable, const pol_t polarity )
{
	assert(polarity != UNDEF);
#ifdef ASSI_AGILITY

	assi_update_agility( ( (uint8_t*)assignment)[variable * 2 + 1] != UNDEF && ( (uint8_t*)assignment)[variable * 2 + 1] != polarity);
#endif
	( ( (uint8_t*)assignment)[variable * 2] ) 
		= (uint8_t)polarity;
	
}

inline pol_t assi_get_backup_polarity( const assi_t assignment, const var_t variable )
{
	
	return (pol_t)( ( (uint8_t*)assignment)[variable * 2 + 1] );
}

inline void assi_set_polarity_backup( assi_t assignment, const var_t variable, const pol_t polarity )
{
	assert(polarity != UNDEF);
#ifdef ASSI_AGILITY

	assi_update_agility( ( (uint8_t*)assignment)[variable * 2 + 1] != UNDEF && ( (uint8_t*)assignment)[variable * 2 + 1] != polarity);
#endif
	( ( (uint8_t*)assignment)[variable * 2 + 1] ) = ( (uint8_t*)assignment)[variable * 2 ];
	( ( (uint8_t*)assignment)[variable * 2] ) = (uint8_t)polarity;
}

inline void assi_invert_variable(assi_t assignment, const var_t variable){
	( ( (uint8_t*)assignment)[variable * 2] ) = inv_pol( (pol_t)( ( (uint8_t*)assignment)[variable * 2] ) );
}

inline void assi_undef_variable(assi_t assignment, const var_t variable)
{
	( ( (uint8_t*)assignment)[variable * 2] ) = (uint8_t)0;
}

inline void assi_undef_variable_backup(assi_t assignment, const var_t variable)
{
	( ( (uint8_t*)assignment)[variable * 2 + 1] ) = ( (uint8_t*)assignment)[variable * 2 ];
	( ( (uint8_t*)assignment)[variable * 2] ) = (uint8_t)0;
}

inline bool assi_variable_equal(const assi_t assignment, const var_t variable, pol_t value)
{
	return ( (uint8_t)value == ( (uint8_t*)assignment)[variable * 2] );
}

inline int32_t assi_get_equal_lits(const assi_t a, const assi_t b, assi_t before, VEC_TYPE(lit_t)& units, VEC_TYPE(lit_t)& equals, const uint32_t varCnt)
{
	uint8_t* pa = (uint8_t*)a;
	uint8_t* pb = (uint8_t*)b;
	uint32_t count = 0;

	
	for( uint32_t i = 1 ; i < varCnt + 1; ++ i){
		
		if( !assi_is_undef(before, i) ) continue;
		
		if( pa[2*i] == pb[2*i] &&  (pol_t)pb[2*i] != UNDEF){
			
			if( assi_variable_equal( before, i, inv_pol( (pol_t)pb[2*i] ) )  ) return -1;
			
			VEC_PUSH_BACK(lit_t,  units, lit( i, (pol_t)pa[2*i] ) );
			count ++;
			continue;
		}
		
		if( pa[2*i] != (uint8_t)UNDEF &&  pb[2*i] == (uint8_t)inv_pol( (pol_t)pa[2*i]) ){
			VEC_PUSH_BACK(lit_t,  equals, lit( i, (pol_t)pa[2*i] ) );
		}
	}
	
	return count;
}


#endif
