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


#ifndef _VEC_MACROS_H
#define _VEC_MACROS_H



#include "defines.h"

#ifdef USE_C_VECTOR

#include "structures/c_vector.h"


#define VEC_TYPE( type ) type* 

#define VEC_CREATE( type, pvector ) pvector = vector_create<type>() 
#define VEC_CREATE_SPACE( type, pvector , elements ) pvector = vector_create<type>( elements )
#define VEC_CREATE_IN_HEAP( type, adress ) { adress = (type**)MALLOC( sizeof(type*) ); *adress = vector_create<type>(); } 
#define VEC_CREATE_SPACE_IN_HEAP( type, adress, elements ) { adress = (type**)MALLOC( sizeof(type*) ); *adress = vector_create<type>( elements ); }
#define VEC_COPY( type, pvector ) vector_copy<type>( pvector ) 
#define VEC_ASSIGN( type, dest, src ) vector_assign<type>( dest, src)

#define VEC_DESTROY( type, pvector ) vector_destroy<type>( pvector ) 
#define VEC_DESTROY_IN_HEAP( type, adress ) { vector_destroy<type>(*adress); free(adress); } 

#define VEC_RESIZE( type, pvector, elements, new_element ) vector_resize<type>( pvector, elements, new_element ) 
#define VEC_RESERVE( type, pvector, elements ) vector_reserve<type>( pvector, elements ) 
#define VEC_RESERVE_ANOTHER( type, pvector, more_elements ) vector_reserve_another<type>( pvector, more_elements ) 
#define VEC_PUSH_BACK( type, pvector, element ) vector_push_back<type>( pvector, element )
#define VEC_PUSH_BACK_ANOTHER( type, pvector, elements, number ) vector_push_back_another<type>( pvector, elements, number ) 
#define VEC_POP_BACK( type, pvector ) vector_pop_back<type>( pvector ) 
#define VEC_ERASE( type, pvector, ind ) vector_erase<type>( pvector, ind ) 
#define VEC_ERASE_NO_ORDER( type, pvector, ind ) vector_erase_no_order<type>( pvector, ind ) 
#define VEC_SIZE( type, pvector ) vector_size<type>( pvector ) 
#define VEC_CAPACITY( type, pvector ) vector_capacity<type>( pvector ) 
#define VEC_CLEAR( type, pvector ) vector_clear<type>( pvector ) 
#define VEC_MINIMIZE( type, pvector ) vector_minimize<type>( pvector ) 
#define VEC_DELETE_FIRST_ELEMENTS( type, pvector, elements ) vector_delete_first_elements<type>( pvector, elements ) 
#define VEC_DELETE_LAST_ELEMENTS( type, pvector, elements ) vector_delete_last_elements<type>( pvector, elements ) 

#else

#ifdef USE_STL_VECTOR

#include <vector>

#define VEC_TYPE( type ) std::vector<type> 

#define VEC_CREATE( type, pvector ) pvector = std::vector< type >()
#define VEC_CREATE_SPACE( type, pvector, elements ) {pvector = std::vector< type >(); pvector.reserve( elements );}
#define VEC_CREATE_IN_HEAP( type, adress ) adress = new std::vector<type>() 
#define VEC_CREATE_SPACE_IN_HEAP( type, adress, elements ) { adress = new std::vector< type >(); adress->reserve( elements ); }
#define VEC_COPY( type, pvector ) std::vector<type>( pvector )
#define VEC_ASSIGN( type, dest, src ) dest.assign( src.begin(), src.end() )

#define VEC_DESTROY( type, pvector ) {}
#define VEC_DESTROY_IN_HEAP( type, adress ) delete adress 
#define VEC_RESIZE( type, pvector, elements, new_element ) pvector.resize( elements, new_element ) 
#define VEC_RESERVE( type, pvector, elements ) pvector.reserve( elements ) 
#define VEC_RESERVE_ANOTHER( type, pvector, more_elements ) pvector.reserve( vector.size() + more_elements ) 
#define VEC_PUSH_BACK( type, pvector, element ) pvector.push_back( (type)((const type)element) ) 
#define VEC_PUSH_BACK_ANOTHER( type, pvector, elements, number ) { for( uint32_t i = 0 ; i < number; ++i ) pvector.push_back( elements[i] ); }
#define VEC_POP_BACK( type, pvector ) pvector.pop_back() 
#define VEC_ERASE( type, pvector, ind ) pvector.erase( pvector.begin() + ind ) 
#define VEC_ERASE_NO_ORDER( type, pvector, ind ) { pvector[ind] = pvector[ pvector.size() - 1 ]; pvector.pop_back(); } 
#define VEC_SIZE( type, pvector ) (uint32_t)pvector.size() 
#define VEC_CAPACITY( type, pvector ) (uint32_t)pvector.capacity() 
#define VEC_CLEAR( type, pvector ) pvector.clear() 

#define VEC_MINIMIZE( type, pvector ) {}	
#define VEC_DELETE_FIRST_ELEMENTS( type, pvector, elements ) pvector.erase( pvector.begin(), pvector.begin() + elements ) 
#define VEC_DELETE_LAST_ELEMENTS( type, pvector, elements ) pvector.resize( pvector.size() - elements ) 

#else

#ifdef USE_CPP_VECTOR

#include "structures/cpp_vector.h"

#define VEC_TYPE( type ) CppVector<type> 

#define VEC_CREATE( type, pvector ) pvector = CppVector< type >()
#define VEC_CREATE_SPACE( type, pvector, elements ) pvector = CppVector< type >(elements )
#define VEC_CREATE_IN_HEAP( type, adress ) adress = new CppVector<type>() 
#define VEC_CREATE_SPACE_IN_HEAP( type, adress, elements ) adress = new CppVector< type >(elements)
#define VEC_COPY( type, pvector ) CppVector<type>( pvector )
#define VEC_ASSIGN( type, dest, src ) dest.assign( src )

#define VEC_DESTROY( type, pvector ) {}
#define VEC_DESTROY_IN_HEAP( type, adress ) delete adress 
#define VEC_RESIZE( type, pvector, elements, new_element ) pvector.resize( elements, new_element ) 
#define VEC_RESERVE( type, pvector, elements ) pvector.reserve( elements ) 
#define VEC_RESERVE_ANOTHER( type, pvector, more_elements ) pvector.reserve( vector.size() + more_elements ) 
#define VEC_PUSH_BACK( type, pvector, element ) pvector.push_back( (type)((const type)element) ) 
#define VEC_PUSH_BACK_ANOTHER( type, pvector, elements, number ) pvector.push_back_another( elements,number );
#define VEC_POP_BACK( type, pvector ) pvector.pop_back() 
#define VEC_ERASE( type, pvector, ind ) pvector.erase(  ind ) 
#define VEC_ERASE_NO_ORDER( type, pvector, ind ) pvector.erase_no_order(  ind ) 
#define VEC_SIZE( type, pvector ) pvector.size() 
#define VEC_CAPACITY( type, pvector ) pvector.capacity() 
#define VEC_CLEAR( type, pvector ) pvector.clear() 

#define VEC_MINIMIZE( type, pvector ) pvector.minimize()
#define VEC_DELETE_FIRST_ELEMENTS( type, pvector, elements ) pvector.delete_first_elements( elements ) 
#define VEC_DELETE_LAST_ELEMENTS( type, pvector, elements ) pvector.delete_last_elements( elements ) 

#else

	
	#error "no vector defined"
#endif

#endif

#endif

#endif
