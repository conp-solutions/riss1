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




#ifndef _GROW_SLAB_MEMORY_H_
#define _GROW_SLAB_MEMORY_H_

#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <iostream>

#include "defines.h"

#include "macros/malloc_macros.h"
#include "structures/grow_thread_memory.h"


template <typename T>
class GrowSlabAllocator{
	union element{
		char data[ sizeof(T) ];	
		uint32_t next;
	};
	
	element* storage;	
	uint32_t eles;	
	uint32_t next_free; 
	uint32_t given_eles;	

	
	void enlarge(){
		uint32_t oldSize = eles;
		eles = (eles * 5) / 4;
		storage = (element*)realloc(storage, sizeof(element) * eles);
		memset( &(storage[oldSize]),0, sizeof(element) * (eles - oldSize) );
	}
	
public:

	GrowSlabAllocator() :
	storage(0), eles(1000000), next_free(1), given_eles(1)	
	{
		storage = (element*)MALLOC( sizeof(element) * eles);
		memset( storage ,0, sizeof(element) * eles );
	}
	
	GrowSlabAllocator(uint32_t elements) :
	storage(0), eles(elements), next_free(1), given_eles(1)	
	{
		storage = (element*)MALLOC( sizeof(element) * eles);
		memset( storage ,0, sizeof(element) * eles );
	}
	
	~GrowSlabAllocator(){
		free( (void*)storage );
	}
	
	void printSizes(){
		std::cerr << "sizeof(T): " << sizeof(T) << "sizeof(element): " << sizeof(element) << std::endl;
	}
	
	void printInfo(){
		std::cerr << "cap: " << eles << " used: " << given_eles << " next_free: " << next_free << " acc.nextField: " << storage[next_free].next << std::endl;
	}
	
	void printFreeChain(){
		std::cerr << "free chain:";
		uint32_t tmp = next_free;
		std::cerr << " " << next_free;
		for( uint32_t i = 0 ; i < eles - given_eles - 1; ++i ){
			tmp = (storage[tmp].next==0) ? tmp + 1 : storage[tmp].next;
			std::cerr << " " << tmp;
		}
		std::cerr << std::endl;
	}
	
	
	uint32_t create( const T& t ){
		
		uint32_t tmp = next_free;
		
		next_free = (storage[next_free].next == (uint32_t)0 ) ? next_free + 1 : storage[next_free].next;
		T* ele = (T*)(&storage[tmp]); 

		*ele = t;
		if( ++given_eles == eles ) enlarge();
		
		return tmp;
	}
	
	
	uint32_t createCleared( const T& t ){
		
		uint32_t tmp = next_free;
		
		next_free = (storage[next_free].next == (uint32_t)0 ) ? next_free + 1 : storage[next_free].next;
		T* ele = (T*)(&storage[tmp]); 
		memset( ele, 0, sizeof(element) );
		*ele = t;
		if( ++given_eles == eles ) enlarge();
		
		return tmp;
	}
	
	T& get(uint32_t ref){
		T* tmp = (T*)&(storage[ref].data);
		return (*tmp);
	}
	
	void* addr(uint32_t ref){
		return &(storage[ref].data);
	}
	
	void release( uint32_t i ){

		
		storage[i].next = next_free;
		next_free = i;
		given_eles--;
	
	}

};

#endif
