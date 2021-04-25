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


#ifndef _HEAP_HH
#define _HEAP_HH

#include <limits.h>		
#include <assert.h>		
#include <string.h>		
#include "structures/c_vector.h"	



#include <iostream>


#define PARENT(__ind)	(((__ind) - 1) / 2)
#define CHILD_LEFT(__ind)		(2 * (__ind) + 1)
#define CHILD_RIGHT(__ind)		(2 * (__ind) + 2)


#define HEAP_COMPARE(_value1, _value2 ) ( (_value1) > (_value2) )





template <typename T>
struct maxheap_t {
		unsigned*	sorting;	
		unsigned* indices;			
		T *values;			
		unsigned max_value;			
};

template <typename T>
maxheap_t<T> maxheap_create( T *values_ref, unsigned size );

template <typename T>
void maxheap_destroy( maxheap_t<T>& heap );

template <typename T>
void maxheap_update(maxheap_t<T>& heap, unsigned item , bool ins, bool rem );

template<typename T>
bool maxheap_contains(const maxheap_t<T>& heap, const unsigned item );

template<typename T>
unsigned maxheap_item_at(const maxheap_t<T>& heap, unsigned ind);

template<typename T>
bool maxheap_is_empty( const maxheap_t<T>& heap );

template<typename T>
unsigned maxheap_size( const maxheap_t<T>& heap );

template<typename T>
void maxheap_swap_items( maxheap_t<T>& heap, unsigned ind1, unsigned ind2);

template<typename T>
void maxheap_clear( maxheap_t<T>& heap );

template<typename T>
void maxheap_insert_item( maxheap_t<T>& heap, unsigned item );

template<typename T>
void maxheap_update_item( maxheap_t<T>& heap, unsigned item );

template<typename T>
void maxheap_remove_item( maxheap_t<T>& heap, unsigned item);













template <typename T>
inline maxheap_t<T> maxheap_create( T *values_ref, unsigned size )
{
	maxheap_t<T> heap;
	heap.values = values_ref;
	heap.max_value = size;

	heap.sorting = vector_create<unsigned>();
	vector_reserve<unsigned>( heap.sorting, size );

	heap.indices = (unsigned*)MALLOC( sizeof( unsigned ) * size );
	memset( heap.indices, UINT_MAX, sizeof( unsigned ) * size );
	return heap;
}

template <typename T>
inline void maxheap_destroy( maxheap_t<T>& heap )
{
	vector_destroy< unsigned >( heap.sorting );
	if( heap.indices != 0 ) free( heap.indices );
}

template <typename T>
inline void maxheap_update(maxheap_t<T>& heap, unsigned item , bool ins, bool rem )
{
	
	unsigned ind, size, max, old_item;
	
	if (ins && !rem) {
		std::cerr << "insert " << item << " with new ind " << vector_size<unsigned >( heap.sorting ) << std::endl;
		heap.indices[item] = vector_size<unsigned >( heap.sorting );
		vector_push_back<unsigned>( heap.sorting, item );
	}
	
	if (rem && !ins) {
		old_item = item;
		ind = heap.indices[item];
		
		item = heap.sorting[ vector_size<unsigned>( heap.sorting ) - 1];
		maxheap_swap_items(heap, ind, vector_size<unsigned>( heap.sorting ) - 1);
		
		vector_pop_back<unsigned>( heap.sorting );
		heap.indices[old_item] = UINT_MAX;
	}
	
	ind = heap.indices[item];
	size = vector_size<unsigned>( heap.sorting );

	
	if (size == 0)
		return;

	assert(CHILD_LEFT(PARENT(ind)) == ind || CHILD_RIGHT(PARENT(ind)) == ind);



	
	while (ind > 0 && HEAP_COMPARE( (heap.values)[heap.sorting[ ind ]], (heap.values)[heap.sorting[ PARENT(ind) ] ] ) ) {
		maxheap_swap_items(heap, ind, PARENT(ind));
		ind = PARENT(ind);
	}

	assert (ind==0 || !HEAP_COMPARE((heap.values)[heap.sorting[ ind ]], (heap.values)[heap.sorting[ PARENT(ind) ] ] ) );

	
	while (1) {
		if (CHILD_LEFT(ind) < size)
			max = CHILD_LEFT(ind);
		else
			break;

		if (CHILD_RIGHT(ind) < size && HEAP_COMPARE((heap.values)[heap.sorting[ CHILD_RIGHT(ind) ]], (heap.values)[heap.sorting[ CHILD_LEFT(ind) ] ] ) )
			max = CHILD_RIGHT(ind);

		if (! HEAP_COMPARE((heap.values)[heap.sorting[ max ]], (heap.values)[heap.sorting [ind] ] ) )
			break;

		maxheap_swap_items(heap,ind, max);
		ind = max;
	}
	
	
	assert (ind==0 || !HEAP_COMPARE((heap.values)[heap.sorting[ind ]], (heap.values)[heap.sorting[ PARENT(ind) ] ] ) );
	assert (CHILD_LEFT(ind) >= size || !HEAP_COMPARE((heap.values)[heap.sorting[CHILD_LEFT(ind) ]], (heap.values)[heap.sorting[ ind ] ] ) );
	assert (CHILD_RIGHT(ind) >= size || !HEAP_COMPARE((heap.values)[heap.sorting[CHILD_RIGHT(ind) ]], (heap.values)[heap.sorting[ ind ] ] ) );
	
}

template<typename T>
inline bool maxheap_contains(const maxheap_t<T>& heap, const unsigned item )
{
	return ( heap.indices[item] < vector_size<unsigned>( heap.sorting ) );
}

template<typename T>
inline unsigned maxheap_item_at(const maxheap_t<T>& heap, unsigned ind)
{
	assert( ind < vector_size<unsigned>( heap.sorting ) );
	return heap.sorting[ind];
}

template<typename T>
inline bool maxheap_is_empty( const maxheap_t<T>& heap )
{
	return vector_size<unsigned>( heap.sorting ) == 0;
}

template<typename T>
inline unsigned maxheap_size( const maxheap_t<T>& heap )
{
	return vector_size<unsigned>( heap.sorting );
}

template<typename T>
inline void maxheap_swap_items( maxheap_t<T>& heap, unsigned ind1, unsigned ind2)
{
	unsigned tmp;

	
	tmp = heap.sorting[ind1];
	heap.sorting[ind1] = heap.sorting[ind2];
	heap.sorting[ind2] = tmp;

	
	tmp = heap.indices[heap.sorting[ind1]];
	heap.indices[ heap.sorting[ind1]] = heap.indices[ heap.sorting[ind2]];
	heap.indices[ heap.sorting[ind2]] = tmp;
}

template<typename T>
inline void maxheap_clear( maxheap_t<T>& heap )
{
	vector_clear<unsigned>( heap.sorting );
	memset( heap.indices, UINT_MAX, sizeof( unsigned ) * vector_size<unsigned>( heap.sorting ) );	
}

template<typename T>
inline void maxheap_insert_item( maxheap_t<T>& heap, unsigned item )
{
	
	maxheap_update(heap, item, true, false);
}

template<typename T>
inline void maxheap_update_item( maxheap_t<T>& heap, unsigned item )
{
	
	maxheap_update( heap, item, true, true);
}

template<typename T>
inline void maxheap_remove_item( maxheap_t<T>& heap, unsigned item)
{
	
	maxheap_update(heap, item, false, true); 
}


#endif
