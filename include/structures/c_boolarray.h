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



#ifndef _BOOLARRAY_H
#define _BOOLARRAY_H

#include "defines.h"
#include "macros/malloc_macros.h"


#include <inttypes.h>	
#include <string.h>
#include <stdlib.h>
#include <assert.h>



#ifdef EXPERIMENTAL_BOOLARRAY



#else


typedef void* boolarray_t;


boolarray_t boolarray_create(const uint32_t elements);

boolarray_t boolarray_copy(const boolarray_t boolarray);

void boolarray_destroy(boolarray_t& boolarray);

void boolarray_clear(boolarray_t& boolarray, const uint32_t elements);

bool boolarray_get( const boolarray_t boolarray, const uint32_t ind );

void boolarray_set( boolarray_t boolarray, const uint32_t ind, const bool value );




inline boolarray_t boolarray_create(const uint32_t elements)
{
	
	void* memory = MALLOC( elements * sizeof(bool) );
	memset( memory, 0, elements * sizeof(bool) );
	return memory;
}

inline boolarray_t boolarray_copy(const boolarray_t boolarray, const uint32_t elements)
{
	
	void* memory = MALLOC( elements * sizeof(bool));
	memcpy( memory, boolarray, elements * sizeof(bool) );
	return memory;
}

inline void boolarray_extend(boolarray_t& boolarray, const uint32_t elements, const uint32_t newElements){
	
	boolarray = (uint8_t*)realloc( boolarray, (newElements ) * sizeof(bool) );
	
	
	if( newElements > elements ){
		const uint32_t diff = newElements - elements;
		memset( ((bool*)boolarray)+elements, 0, diff * sizeof(bool) );
	}
}

inline void boolarray_destroy(boolarray_t& boolarray)
{
	if( boolarray != 0 )
	{
		free(boolarray);
		boolarray = 0;
	}
}

inline void boolarray_clear(boolarray_t& boolarray, const uint32_t elements){
	memset( boolarray, 0, elements * sizeof(bool) );
}

inline bool boolarray_get( const boolarray_t boolarray, const uint32_t ind )
{
 return ((bool*)boolarray)[ ind ];
}

inline void boolarray_set( boolarray_t boolarray, const uint32_t ind, const bool value )
{
	((bool*)boolarray)[ ind ] = value;
}

#endif

#endif
