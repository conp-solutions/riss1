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


#ifndef _CVEC_H
#define _CVEC_H

#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <assert.h>






template <typename T>
T* vector_create() __attribute__ ((malloc));


template <typename T>
T* vector_create(uint32_t elements) __attribute__ ((malloc));


template <typename T>
T* vector_copy( const T* vector) __attribute__ ((malloc));


template <typename T>
inline void vector_assign( T* dest, const T* src);


template <typename T>
void vector_destroy( T*& vector ) ;


template <typename T>
void vector_resize( T*& vector, const uint32_t elements, const T new_element );


template <typename T>
void vector_reserve( T*& vector, const uint32_t elements );


template <typename T>
void vector_reserve_another( T*& vector, const uint32_t more_elements );


template <typename T>
void vector_push_back( T*& vector, const T& element );


template <typename T>
void vector_push_back_another( T*& vector, const T* elements, const uint32_t number );


template <typename T>
void vector_pop_back( T* vector);


template <typename T>
void vector_erase( T* vector, const uint32_t ind );


template <typename T>
void vector_erase_no_order( T* vector, const uint32_t ind );


template <typename T>
uint32_t vector_size( const T* vector );
template <typename T>
uint32_t vector_size( const T*& vector );


template <typename T>
uint32_t vector_capacity( const T* vector );


template <typename T>
void vector_clear( T* vector );


template <typename T>
void vector_minimize( T*& vector );


template <typename T>
void vector_delete_first_elements( T* vector, const uint32_t elements );


template <typename T>
void vector_delete_last_elements( T* vector, const uint32_t elements );






template <typename T>
inline T* vector_create()
{
	
	uint32_t * tmp = (uint32_t *) MALLOC( 2*sizeof(uint32_t ) + 2 * sizeof( T ) );
	
	tmp[0] = 2;
	
	tmp[1] = 0;
	
	T* vector = (T*)&(tmp[2]);
	return vector;
}


template <typename T>
T* vector_create(uint32_t elements)
{
	
	uint32_t * tmp = (uint32_t *) MALLOC( 2*sizeof(uint32_t ) + elements * sizeof( T ) );
	
	tmp[0] = elements;
	
	tmp[1] = 0;
	
	T* vector = (T*)&(tmp[2]);
	return vector;
}

template <typename T>
inline T* vector_copy( const T* vector)
{
	assert( vector != 0);
	
	uint32_t * newvector = (uint32_t *)MALLOC( 2 * sizeof( uint32_t ) + vector_capacity<T>(vector) * sizeof(T) );
	memcpy( newvector, & (((uint32_t *)vector)[-2]), 2 * sizeof( uint32_t ) + vector_capacity<T>(vector) * sizeof(T) );
	newvector = &(newvector[2]);	
	
	
	assert( vector_size<T>( (T*)newvector ) == vector_size<T>( vector ) );
	assert( vector_capacity<T>( (T*)newvector ) <= vector_capacity<T>( vector ) );	
	
	return (T*)newvector;
}

template <typename T>
inline void vector_assign( T* dest, const T* src)
{
	assert( src != 0);
	assert( dest != 0 );
	
	vector_clear<T>(dest);
	vector_push_back_another<T>(dest, src, vector_size<T>( src ));
}

template <typename T>
inline void vector_destroy( T*& vector )
{
	assert( vector != 0);
	free( &(((uint32_t *)vector)[-2]) );
	vector = 0;
}


template <typename T>
void vector_resize( T*& vector, const uint32_t elements, const T new_element )
{
	assert( vector != 0);
	
	
	if( vector_size<T>( vector ) == elements ) return;
	
	if( elements < vector_size<T>( vector ) )	
	{
		
		((uint32_t *)vector)[-1] = elements;
	} else {	

		
		uint32_t * tmp = (uint32_t *) realloc( &(((uint32_t *)vector)[-2]), 2*sizeof(uint32_t ) + elements * sizeof( T ) );
		
		tmp[0] = elements;
		
		
		vector = (T*)&(tmp[2]);
		
		
		for( uint32_t i = vector_size<T>( vector ); i < elements; ++i )
			vector[i] = new_element;

		
		((uint32_t *)vector)[-1] = elements;
	}
	
	assert( vector_size<T>( vector ) == elements );
}

template <typename T>
inline void vector_reserve( T*& vector, const uint32_t elements )
{
	assert( vector != 0);
	
	if( elements <= vector_size<T>( vector ) ) return;

	const uint32_t size = vector_size<T>( vector );
	
	
	
	vector = (T*) realloc( & (((uint32_t *)vector)[-2]), 2*sizeof(uint32_t ) + elements * sizeof( T ) );
	
	
	((uint32_t *)vector)[0] = elements;
	((uint32_t *)vector)[1] = size;
	
	
	vector = (T*) & (((uint32_t *)vector)[2]);
	
	assert( vector_capacity<T>( vector ) == elements );
	assert( vector_size<T>( vector ) == size );
}

template <typename T>
inline void vector_reserve_another( T*& vector, const uint32_t more_elements )
{
	assert( vector != 0);
	vector_reserve<T>( vector, vector_size<T>( vector ) + more_elements );
}

template <typename T>
inline void vector_push_back( T*& vector, const T& element )
{
	assert( vector != 0);
	
	const uint32_t size = vector_size<T>( vector );
	
	
	if( size < vector_capacity<T>( vector ) )
	{
		
		vector[ size ] = element;
		((uint32_t *)vector)[-1] ++;
	} else {
		
		
	
		
		
		
		
		const uint32_t new_capacity = 1 + (uint32_t)((float)1.4 * (float)vector_capacity<T>( vector ));
		vector = (T*) realloc( & (((uint32_t *)vector)[-2]), 2*sizeof(uint32_t ) + new_capacity * (float)sizeof( T ) );
		
		((uint32_t *)vector)[0] = new_capacity;
		
		
		vector = (T*) & (((uint32_t *)vector)[2]);		
		
		
		vector[ vector_size<T>( vector ) ] = element;
		((uint32_t *)vector)[-1] ++;
	}
}

template <typename T>
void vector_push_back_another( T*& vector, const T* elements, const uint32_t number )
{
	assert( vector != 0);

	uint32_t size = vector_size<T>( vector );
	
	if( size + number >= vector_capacity<T>( vector ) )
	{
		
		
		vector = (T*) realloc( & (((uint32_t *)vector)[-2]), 2*sizeof(uint32_t ) + ( size + number ) * sizeof( T ) );

		
		((uint32_t *)vector)[0] = size + number;
		((uint32_t *)vector)[1] = size;
		
		
		vector = (T*) & (((uint32_t *)vector)[2]);
	}
	
	memcpy( &(vector[ vector_size<T>( vector ) ]), elements, sizeof(T) * number );
	((uint32_t *)vector)[-1] += number;
}

template <typename T>
inline void vector_pop_back( T* vector)
{
	assert( vector != 0);
	assert( vector_size<T>( vector ) > 0 );
	
	((uint32_t *)vector)[-1]--;
}

template <typename T>
inline void vector_erase( T* vector, const uint32_t ind )
{
	assert( vector != 0);
	assert( vector_size<T>( vector ) > ind );
	
	
	memcpy( &(vector[ind]), &(vector[ind+1]), ( vector_size<T>( vector ) - ind - 1 ) * sizeof( T ) ); 
	
	
	((uint32_t *)vector)[-1]--;	
}

template <typename T>
inline void vector_erase_no_order( T* vector, const uint32_t ind )
{
	assert( vector != 0 );
	assert( vector_size<T>( vector ) > 0 );
	
	vector[ind] = vector[ vector_size<T>( vector ) - 1 ];
	
	((uint32_t *)vector)[-1]--;	
}

template <typename T>
inline uint32_t vector_size( const T* vector )
{
	assert( vector != 0 );
	return ((uint32_t *)vector)[-1];
}

template <typename T>
inline uint32_t vector_size( const T*& vector )
{
	assert( vector != 0 );
	return ((uint32_t *)vector)[-1];
}

template <typename T>
inline uint32_t vector_capacity( const T* vector )
{
	assert( vector != 0 );
	return ((uint32_t *)vector)[-2];
}

template <typename T>
inline void vector_clear( T* vector )
{
	assert( vector != 0 );
	((uint32_t *)vector)[-1] = 0;
}

template <typename T>
inline void vector_minimize( T*& vector )
{
	assert( vector != 0 );
	vector_reserve<T>( vector, vector_size<T>( vector ) );
}

template <typename T>
void vector_delete_first_elements( T* vector, const uint32_t elements )
{
	assert( vector != 0);
	assert( vector_size<T>( vector ) > elements );
	
	
	
	uint32_t i = elements;
	const uint32_t size = vector_size<T>( vector );
	while( i > size )
	{
		memcpy( &( vector[ i - elements] ), &(vector[ i ]),  elements * sizeof( T ) );
		i+=elements;
	}
	memcpy( &( vector[ i - elements ] ), &(vector[ i ]), ( size - (i - elements) )  * sizeof( T ) );
	
	
	
	((uint32_t *)vector)[-1] = vector_size<T>( vector ) - elements;
}


template <typename T>
void vector_delete_last_elements( T* vector, const uint32_t elements )
{
	assert( vector != 0);
	assert( vector_size<T>( vector ) > elements );
	
	((uint32_t *)vector)[-1] = vector_size<T>( vector ) - elements;
}


#endif
