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


#ifndef _CSTACK_H
#define _CSTACK_H

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <assert.h>






template <typename T>
T* stack_create();


template <typename T>
T* stack_copy( T*& dst, const T* src);


template <typename T>
void stack_destroy( T* stack );


template <typename T>
void stack_reserve( T*& stack, const uint32_t elements );


template <typename T>
void stack_reserve_another( T*& stack, const uint32_t more_elements );


template <typename T>
T stack_top(const T* stack);


template <typename T>
void stack_push( T*& stack, const T& element );


template <typename T>
void stack_pop( T* stack);


template <typename T>
void stack_pop_to( T*& stack, const uint32_t left_elements );


template <typename T>
void stack_pop_last( T*& stack, const uint32_t pop_elements );


template <typename T>
uint32_t stack_size( const T* stack );


template <typename T>
uint32_t stack_capacity( const T* stack );


template <typename T>
uint32_t stack_clear( T* stack );


template <typename T>
void stack_minimize( T*& stack );










template <typename T>
inline T* stack_create()
{
	
	T* stack = (T*) MALLOC( 2*sizeof(uint32_t ) + 2 * sizeof( T ) );
	
	((uint32_t *)stack)[0] = 2;
	
	((uint32_t *)stack)[1] = 0;
	
	stack = (T*)&(((uint32_t *)stack)[2]);
	return stack;
}

template <typename T>
inline T* stack_copy( T*& dst, const T* src)
{
	if(dst!=0) stack_destroy<T>(dst);
	assert( src != 0 );
	
	T* newsrc = (T*) MALLOC( 2*sizeof(uint32_t ) + stack_size<T>(src) * sizeof( T ) );
	
	memcpy( newsrc, & (((uint32_t *)src)[-2]), 2*sizeof(uint32_t ) + stack_size<T>(src) * sizeof( T ) );
	
	return (T*)(&((uint32_t *)newsrc)[2]);
}

template <typename T>
inline void stack_destroy( T* stack )
{
	assert( stack != 0 );
	free( &(((uint32_t *)stack)[-2]) );
	stack = 0;
}

template <typename T>
inline void stack_reserve( T*& stack, const uint32_t elements )
{
	assert( stack != 0 );
	
	if( elements < stack_size<T>( stack ) ) return;

	
	stack = (T*) realloc( &(((uint32_t *)stack)[-2]), 2*sizeof(uint32_t ) + elements * sizeof( T ) );
	
	((uint32_t *)stack)[0] = elements;
	
	
	stack = (T*)&(((uint32_t *)stack)[2]);	
}

template <typename T>
inline void stack_reserve_another( T*& stack, const uint32_t more_elements )
{
	assert( stack != 0 );
	stack_reserve<T>( stack, stack_size<T>( stack ) + more_elements );
}

template <typename T>
inline void stack_push( T*& stack, const T& element )
{
	assert( stack != 0 );
	
	if( stack_size<T>( stack ) < stack_capacity<T>( stack ) )
	{
		
		stack[ stack_size<T>( stack ) ] = element;
		((uint32_t *)stack)[-1] ++;
	} else {
		
		
		stack_reserve_another<T>( stack, stack_size<T>( stack ) / 2 + 8 );
		
		stack[ stack_size<T>( stack ) ] = element;
		((uint32_t *)stack)[-1] ++;
	}
}


template <typename T>
inline T stack_top(const T* stack)
{
	assert( stack != 0 );
	assert( stack_size<T>(stack) > 0 );	
	return stack[ stack_size<T>( stack )-1 ];
}


template <typename T>
inline void stack_pop( T* stack)
{
	assert( stack != 0 );
	assert( stack_size<T>(stack) > 0 );
	
	
	((uint32_t *)stack)[-1]--;
}


template <typename T>
inline void stack_pop_to( T*& stack, const uint32_t left_elements )
{
	assert( stack != 0 );
	assert( left_elements <= stack_size<T>(stack) );
	((uint32_t *)stack)[-1] = left_elements;
}


template <typename T>
inline void stack_pop_last( T*& stack, const uint32_t pop_elements )
{
	assert( stack != 0 );
	assert( pop_elements <= stack_size(stack) );
	((uint32_t *)stack)[-1] -= pop_elements;	
}


template <typename T>
inline uint32_t stack_size( const T* stack )
{
	assert( stack != 0 );
	return ((uint32_t *)stack)[-1];
}

template <typename T>
inline uint32_t stack_capacity( const T* stack )
{
	assert( stack != 0 );
	return ((uint32_t *)stack)[-2];
}

template <typename T>
inline uint32_t stack_clear( T* stack )
{
	assert( stack != 0 );
	return ((uint32_t *)stack)[-1] = 0;
}

template <typename T>
inline void stack_minimize( T*& stack )
{
	assert( stack != 0 );
	stack_reserve<T>( stack, stack_size<T>( stack ) );
}




#endif
