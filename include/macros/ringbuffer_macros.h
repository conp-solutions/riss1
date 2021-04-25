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


#ifndef _RINGBUFFER_MACROS_H
#define _RINGBUFFER_MACROS_H



#include "defines.h"

#ifdef USE_C_RINGBUFFER

	#error "there is no ringbuffer C implementation any more"

#else

#ifdef USE_STL_RINGBUFFER

#include <deque>


#define RINGBUFFER_TYPE( type ) std::deque< type >

#define RINGBUFFER_CREATE( type, max_elements ) std::deque<type>()
#define RINGBUFFER_DESTROY( type, ringbuffer ) {}
#define RINGBUFFER_CAPACITY( type, ringbuffer ) ringbuffer.max_size()
#define RINGBUFFER_SIZE( type, ringbuffer ) ringbuffer.size()
#define RINGBUFFER_IS_EMPTY( type, ringbuffer ) ( ringbuffer.empty() ) 
#define RINGBUFFER_IS_FULL( type, ringbuffer ) ringbuffer.size() == ringbuffer.max_size() 
#define RINGBUFFER_PUSH_BACK( type, ringbuffer, element ) (ringbuffer).push_back( element )
#define RINGBUFFER_PUSH_FRONT( type, ringbuffer, element ) (ringbuffer).push_front( element ) 
#define RINGBUFFER_POP_FRONT( type, ringbuffer, element ) { element = (ringbuffer).front(); (ringbuffer).pop_front(); }
#define RINGBUFFER_POP_BACK( type, ringbuffer, element ) { element = (ringbuffer).back(); (ringbuffer).pop_back(); }

#define RINGBUFFER_GET_FRONT( type, ringbuffer, ind, element ) element = (ringbuffer).at(ind)
#define RINGBUFFER_GET_BACK( type, ringbuffer, ind, element ) element = (ringbuffer).at( (ringbuffer).size() - ind - 1)

#define RINGBUFFER_CLEAR( type, ringbuffer ) (ringbuffer).clear() 

#endif

#endif

#endif
