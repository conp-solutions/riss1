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




#ifndef _SLAB_MEMORY_H_
#define _SLAB_MEMORY_H_

#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <iostream>

#include "defines.h"
#include "macros/malloc_macros.h"

#define SLAB_PAGE_SIZE (4096 - 24) 

template <typename T>
class SlabAllocator{
	union element{
		char data[ sizeof(T) ];	
		element* next;
	};
	
	element* _head;	
	element* _lastPage; 
	
	element* _nextCell;	
	element* _lastCell;	
	
	void* initPage();
	
public:
	SlabAllocator() : _head(0) {
		_lastPage = (element*)valloc( SLAB_PAGE_SIZE );
		memset( _lastPage, 0, SLAB_PAGE_SIZE );
	
		const uint32_t pageElements = ( SLAB_PAGE_SIZE/sizeof(element) );
		_nextCell = &( _lastPage[1] );
		_lastCell = &( _lastPage[pageElements - 1 ] );
		
		_head = _lastPage;
	}
	
	~SlabAllocator(){
		while( _head != _lastPage )
		{
			element* tmp = _head->next;
			free( _head );
			_head = tmp;
		}
		
		free( _head );
	}
	
	void* get();
	void release( void* adress );

};

template <typename T>
inline void* SlabAllocator<T>::initPage()
{
	_lastPage->next = (element*)valloc( SLAB_PAGE_SIZE );
	_lastPage = _lastPage->next;
	memset( _lastPage, 0, SLAB_PAGE_SIZE );
	
	const uint32_t pageElements = ( SLAB_PAGE_SIZE/sizeof(element) );
	_nextCell = &( _lastPage[1] );
	_lastCell = &( _lastPage[pageElements - 1 ] );
	return _lastPage;
}


template <typename T>
inline void* SlabAllocator<T>::get()
{
	element* t = _nextCell;
	if( _nextCell != _lastCell ){
		_nextCell = (_nextCell->next==0) ? _nextCell+1 : _nextCell->next;
	} else {
		initPage();
	}
	return t;
}


template <typename T>
inline void SlabAllocator<T>::release( void* adress )
{
	_lastCell->next = (element*)adress;
	_lastCell = (element*)adress;
}








class SlabSizeAllocator{

	template <int T>
	union element{
		char data[ T ];	
		element* next;
	};
	
	void** _head[SLAB_MALLOC_MAXSIZE];	
	void** _lastPage[SLAB_MALLOC_MAXSIZE]; 
	
	void** _nextCell[SLAB_MALLOC_MAXSIZE];	
	void** _lastCell[SLAB_MALLOC_MAXSIZE];	
	
	void* initPage(uint32_t size);
	
public:
	SlabSizeAllocator(){

		for( uint32_t i = sizeof( void* ) ; i < SLAB_MALLOC_MAXSIZE; ++i ){
			
			_head[i] = 0;

	
		_lastPage[i] = (void**)valloc( SLAB_PAGE_SIZE );
		memset( _lastPage[i], 0, SLAB_PAGE_SIZE );
	
		const uint32_t pageElements = ( (SLAB_PAGE_SIZE-8)/i); 
		_nextCell[i] = &( _lastPage[i][1] );
		_lastCell[i] = reinterpret_cast<void **>( reinterpret_cast<unsigned long>( &(_lastPage[i][1]) ) + (pageElements - 1) * i );
		
		_head[i] = _lastPage[i];
		}
	}
	
	~SlabSizeAllocator(){
		for( uint32_t i = sizeof( void* ) ; i < SLAB_MALLOC_MAXSIZE; ++i ){
			while( _head[i] != _lastPage[i] )
			{
				void** tmp = (void**)(*(_head[i]));
				free( (void*)_head[i] );
				_head[i] = tmp;
			}
			
			free( _head[i] );
		}
	}
	
	void* get(uint32_t size);
	void release( void* adress, uint32_t size );
	void* resize( void* adress, uint32_t new_size, uint32_t size );

};

inline void* SlabSizeAllocator::initPage(uint32_t size)
{
	
	*(_lastPage[size]) = (void**)valloc( SLAB_PAGE_SIZE );
	_lastPage[size] = (void**) *(_lastPage[size]);
	memset( _lastPage[size], 0, SLAB_PAGE_SIZE );
	
	const uint32_t pageElements = ( (SLAB_PAGE_SIZE-8)/size); 
	_nextCell[size] = &( _lastPage[size][1] );
	_lastCell[size] = reinterpret_cast<void **>( reinterpret_cast<unsigned long>( &(_lastPage[size][1]) ) + (pageElements - 1) * size );
		
	
	return _lastPage[size];
}


inline void* SlabSizeAllocator::get(uint32_t size)
{
	
	if( size >= SLAB_MALLOC_MAXSIZE ) return MALLOC(size);
	if( size < sizeof( void* ) ) size = sizeof( void* );

	void** t = _nextCell[size];
	if( _nextCell[size] != _lastCell[size] ){
		_nextCell[size] = ( (*(_nextCell[size]))==0 ) 
			?  

					reinterpret_cast<void **>( reinterpret_cast<unsigned long>(_nextCell[size]) + size)
			
				: (void**)(*(_nextCell[size]));
	} else {
		initPage(size);
	}
	
	
	return t;
}


inline void SlabSizeAllocator::release( void* adress, uint32_t size )
{
	
	
	if( size >= SLAB_MALLOC_MAXSIZE ) return free( adress );
	if( size < sizeof( void* ) ) size = sizeof( void* );

	
	*(_lastCell[size]) 
						= adress;
	
	_lastCell[size] 
						= (void**)adress;
}

inline void* SlabSizeAllocator::resize( void* adress, uint32_t new_size, uint32_t size )
{
	
	
	void* mem = get(new_size);
	uint32_t smaller = ( new_size < size ) ? new_size : size;
	
	memcpy( mem, adress, smaller );
	
	release( adress, size );
	
	return mem;
}
#endif
