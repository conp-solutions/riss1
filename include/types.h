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


#ifndef _TYPES_H
#define _TYPES_H


#include <inttypes.h>




typedef uint32_t lit_t;


typedef uint32_t var_t;


typedef enum {
	POS = 1,
	NEG = 2,
	UNDEF = 0
} pol_t;


typedef enum {
 SAT = 10,
 UNSAT = 20,
 UNKNOWN = 0
} solution_t;




#ifndef COMPRESS_CLAUSE
	#define CL_PACK __attribute__((packed))
#else
	#define CL_PACK
#endif

#ifdef COMPRESS_WATCH_STRUCTS
	#define WATCH_STRUCT_PACK __attribute__((packed))
#else
	#define WATCH_STRUCT_PACK 
#endif

#endif
