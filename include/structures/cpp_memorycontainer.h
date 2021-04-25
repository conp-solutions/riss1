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
#include <cstdlib>
#include <inttypes.h>


#define _MEM_CONT_FIELDSIZE	4096

#define _MEM_CONT_SIZE_OFFSET 0xFFFFF000


template <typename T>
class MemoryContainer{
public:

	union element
	{
			char _data[ sizeof(T) ];
			element* next;
	};
	
private:
	element* first;
	element* last;
	
	element* field;
	
	uint32_t fields;
	
	const uint32_t pageElements;
	
	
public:

	
	MemoryContainer()
		: first( 0 ) , last( 0 ), field( 0 ), fields( 1 ), pageElements( ( 4096 / sizeof( T ) ) - 2 )
	{
		
		field = initNextMemory();
	}
	
	~MemoryContainer(){
		element* fieldPointer = field;
		while( fieldPointer != 0 ){
			element* t = fieldPointer;
			fieldPointer = fieldPointer[ pageElements ].next;
			delete [] t;
		}
	}

	element* initNextMemory(){
		element* memory = new element[ pageElements + 1 ];	
		
		memory[ pageElements - 1 ].next = 0;
		for(uint32_t i = 1; i< pageElements - 1; i++) memory[i].next = &( memory[i+1] );
		
		first = &(memory[1]);
		last = &(memory[ pageElements - 1 ]);
		
		memory[ pageElements ].next = 0;
		
		(*(unsigned int*)memory) = 0;
		return memory;
	}

	
	void* newElement(  ){
		if( 0 == first )
		{
			element* fieldPointer = field;
			while( fieldPointer[ pageElements ].next != 0 ) fieldPointer = fieldPointer[ pageElements ].next;

			fieldPointer[ pageElements ].next = initNextMemory();
		}
		T* t = (T*) &(first->_data);	
		first = first->next;	

		
		return (void*)t;
	}

	
	void deleteElement(void *p){
		element* t = (element*) p;
		
		
		
		if(first != 0)	
		{
			
			last->next = t;
			last = last->next;
		}
		else{
			first = t;
			last = t;
		}
		last->next = 0;
	}

};

