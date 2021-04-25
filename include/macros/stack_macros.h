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


#ifndef _STACK_MACROS_H
#define _STACK_MACROS_H



#include "defines.h"

#ifdef USE_C_STACK

#include "structures/c_stack.h"


#define STACK_TYPE( type ) type*

#define STACK_CREATE( type ) stack_create<type>() 
#define STACK_COPY( type, src,dst ) stack_copy<type>( dst, src )

#define STACK_DESTROY( type, stack ) stack_destroy<type>( stack ) 
#define STACK_RESIZE( type, stack, elements, new_element ) stack_resize<type>( stack, elements, new_element ) 
#define STACK_RESERVE( type, stack, elements ) stack_reserve<type>( stack, elements ) 
#define STACK_RESERVE_ANOTHER( type, stack, more_elements ) stack_reserve_another<type>( stack, more_elements ) 
#define STACK_SIZE( type, stack ) stack_size<type>( stack ) 
#define STACK_CAPACITY( type, stack ) stack_capacity<type>( stack ) 
#define STACK_CLEAR( type, stack ) stack_clear<type>( stack ) 
#define STACK_MINIMIZE( type, stack ) stack_minimize<type>( stack ) 

#define STACK_PUSH( type, stack, element ) stack_push<type>( stack, element ) 
#define STACK_POP( type, stack ) stack_pop<type>( stack ) 
#define STACK_POP_TO( type, stack, left_elements ) stack_pop_to<type>( stack, left_elements ) 
#define STACK_POP_LAST( type, stack, pop_elements ) stack_pop_last<type>( stack, pop_elements ) 
#define STACK_TOP( type, stack ) stack_top<type>( stack ) 

#else

#ifdef USE_STL_STACK


#define STACK_TYPE( type ) std::vector<type> 

#define STACK_CREATE( type ) std::vector< type >() 
#define STACK_COPY( type, dst, src ) src = std::vector<type>( dst )
#define STACK_DESTROY( type, stack ) {} 
#define STACK_RESIZE( type, stack, elements, new_element ) stack.resize(elements,new_element )
#define STACK_RESERVE( type, stack, elements ) stack.reserve( elements ) 
#define STACK_RESERVE_ANOTHER( type, stack, more_elements ) stack.reserve( stack.size() +  more_elements )
#define STACK_SIZE( type, stack ) (uint32_t)stack.size() 
#define STACK_CAPACITY( type, stack ) (uint32_t)stack.capacity() 
#define STACK_CLEAR( type, stack ) stack.clear() 
#define STACK_MINIMIZE( type, stack ) {} 

#define STACK_PUSH( type, stack, element ) stack.push_back( element ) 
#define STACK_POP( type, stack ) stack.pop_back() 
#define STACK_POP_TO( type, stack, left_elements ) { while( stack.size() > left_elements ) stack.pop_back(); } 
#define STACK_POP_LAST( type, stack, pop_elements ) { for( uint32_t i = 0; i < pop_elements; ++i ) stack.pop_back(); } 
#define STACK_TOP( type, stack ) stack[ stack.size() - 1 ]


#endif

#endif

#endif
