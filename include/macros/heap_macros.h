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


#ifndef _HEAP_MACROS_H
#define _HEAP_MACROS_H



#include "defines.h"

#ifdef USE_C_HEAP

#include "structures/c_heap.h"


#define HEAP_TYPE( type ) maxheap_t< type > 

#define HEAP_CREATE( type, values_ref, size )( maxheap_create<type>( values_ref, size ) )
#define HEAP_DESTROY( type, heap )( maxheap_destroy<type>(  heap ) )
#define HEAP_CONTAINS( type, heap, item )( maxheap_contains<type>(  heap, item ) )
#define HEAP_ITEM_AT( type, heap, ind )maxheap_item_at<type>(  heap,  ind)
#define HEAP_IS_EMPTY( type, heap )( maxheap_is_empty<type>(  heap ) )
#define HEAP_SIZE( type, heap )( maxheap_size<type>(  heap ) )
#define HEAP_CLEAR( type, heap )( maxheap_clear<type>(  heap ) )
#define HEAP_INSERT_ITEM( type, heap, item )( maxheap_insert_item<type>(  heap, item ) )
#define HEAP_UPDATE_ITEM( type, heap, item )( maxheap_update_item<type>(  heap, item ) )
#define HEAP_REMOVE_ITEM( type, heap, item )( maxheap_remove_item<type>(  heap, item ) )

#else

	#ifdef USE_CPP_HEAP

	#include "structures/cpp_heap.h"

		
		#define HEAP_TYPE( type ) cpp_heap< type, std::greater< type > >
		
		#define HEAP_CREATE( type, values_ref, size )( cpp_heap< type , std::greater< type > >(&values_ref, size) )
		#define HEAP_DESTROY( type, heap )( {} )
		#define HEAP_CONTAINS( type, heap, item )( heap.contains( item ) )
		#define HEAP_ITEM_AT( type, heap, ind ) heap.item_at( ind ) 
		#define HEAP_IS_EMPTY( type, heap )( heap.is_empty() )
		#define HEAP_SIZE( type, heap )( heap.size() )
		#define HEAP_CLEAR( type, heap )( heap.clear() )
		#define HEAP_INSERT_ITEM( type, heap, item )( heap.insert_item( item ) )
		#define HEAP_UPDATE_ITEM( type, heap, item )( heap.update_item( item ) )
		#define HEAP_REMOVE_ITEM( type, heap, item )( heap.remove_item( item) )

	#endif

#endif

#endif
