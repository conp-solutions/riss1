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


#ifndef _MALLOC_MACROS_H
#define _MALLOC_MACROS_H



#include "defines.h"

#include <iostream>

#ifdef USE_ALLIGNED_MALLOC	
	#include <cstdlib>
	#include <malloc.h>

	#define MALLOC( size ) memalign ( MALLOC_ALLIGNMENT, size)
#else
	#define MALLOC( size ) malloc( size )
#endif


#ifdef USE_SLAB_MALLOC

	#include "structures/slab_malloc.h"

	#define MALLOC_SIZE( size )slab_malloc( size )
	#define REALLOC_SIZE( old_ptr, new_size, old_size ) slab_realloc( old_ptr, new_size,old_size )
	#define FREE_SIZE( ptr, size ) slab_free( ptr, size )

#else

	#include <cstdlib>

	#define MALLOC_SIZE( size ) MALLOC( size )
	#define REALLOC_SIZE( old_ptr, new_size, old_size ) realloc( old_ptr, new_size )

	#ifdef JAVALIB
		
		#define FREE_SIZE( ptr, size ) free(ptr)

	#else
		#define FREE_SIZE( ptr, size ) free(ptr)
	#endif

#endif

#endif
