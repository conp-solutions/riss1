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


#ifndef _DEFINES_H
#define _DEFINES_H

#include <string>
#include <sstream>


std::string getBuildFlagList();




#ifndef SATSOLVER
	#define SATSOLVER
#endif

#ifdef CSPSOLVER
	#undef SATSOLVER
#endif


#ifndef PARALLEL
 
#endif


#ifndef TRACK_CLAUSE

#endif


#ifndef COMPETITION

#endif


#ifndef SILENT

#endif


#ifndef TESTBINARY
	
#endif

#ifndef GOOD_STRUCTURES
	#define GOOD_STRUCTURES
#endif

#ifdef BAD_STRUCTURES
	#undef GOOD_STRUCTURES
#endif

#ifndef BAD_STRUCTURES
	#ifndef GOOD_STRUCTURES
		#define GOOD_STRUCTURES
	#endif
#else
	
#endif


#ifdef GOOD_STRUCTURES
	#define USE_IMPLICIT_BINARY
	#define USE_CONFLICT_PREFETCHING
	#define USE_PREFETCHING
	#define PREFETCHINGMETHOD3
	#define COMPACT_VAR_DATA
	#define USE_RISS_CLAUSE 
#endif




#ifndef USE_LIBRARY

#endif



#ifndef USE_ALL_COMPONENTS

#endif


#ifndef USE_SOME_COMPONENTS

#endif

#ifdef USE_ALL_COMPONENTS
#ifndef USE_SOME_COMPONENTS
	#define USE_SOME_COMPONENTS
#endif
#endif



#ifndef USE_COMMANDLINEPARAMETER
	#define USE_COMMANDLINEPARAMETER
#endif


#ifdef COMPETITION
	#ifdef USE_COMMANDLINEPARAMETER
		#undef USE_COMMANDLINEPARAMETER
	#endif
	#ifdef USE_SOME_COMPONENTS
		#undef USE_SOME_COMPONENTS
	#endif
	#ifdef USE_ALL_COMPONENTS
		#undef USE_ALL_COMPONENTS
	#endif
#endif

#ifdef USE_COMMANDLINEPARAMETER
	#define CONST_PARAM 
#else
	#define CONST_PARAM const
#endif




#ifndef BLOCKING_LIT

#endif

#ifdef USE_IMPLICIT_BINARY
	
#endif


#ifndef KEEP_SAT_IN_WATCH

#endif

#ifndef USE_CONFLICT_PREFETCHING

#endif

#ifndef USE_SAME_LEVEL_PREFETCHING

#endif


#ifndef USE_PREFETCHING

#endif

#ifdef USE_DUAL_PREFETCHING
	
#endif


#ifndef PREFETCHINGMETHOD1
	#define PREFETCHINGMETHOD1
#endif


#ifdef  PREFETCHINGMETHOD2
	#undef  PREFETCHINGMETHOD1
#endif


#ifdef PREFETCHINGMETHOD3
	#undef  PREFETCHINGMETHOD1
#endif


#ifndef COMPACT_VAR_DATA

#endif

#ifdef COMPACT_VAR_DATA
	#define VAR_REASON(x) var1[ x ].reason
	#define VAR_LEVEL(x) var1[ x ].level
#else
	#define VAR_REASON(x) reason[ x ]
	#define VAR_LEVEL(x) level[ x ]
#endif




#ifndef OTFSS
	
#endif


#ifndef ASSI_AGILITY
	
#endif




#ifndef COMPRESS_CLAUSE
 
#endif


#ifndef COMPRESS_WATCH_STRUCTS
	
#endif


#ifndef USE_ALLIGNED_MALLOC
	
#endif

#ifdef USE_ALLIGNED_MALLOC
	#ifndef MALLOC_ALLIGNMENT
		#define MALLOC_ALLIGNMENT 64
	#endif
#endif


#ifndef USE_SLAB_MALLOC
	
#endif


#ifndef SLAB_MALLOC_MAXSIZE
	#define SLAB_MALLOC_MAXSIZE 32
#endif





#ifndef COLLECT_STATISTICS
	
#endif











#ifndef USE_STL_STRUCTURES
	#define USE_STL_STRUCTURES
#endif

#ifdef USE_STL_STRUCTURES
	#define USE_STL_VECTOR
	#define USE_STL_STACK
	#define USE_CPP_HEAP
	#define USE_STL_RINGBUFFER	
#endif

#ifdef USE_C_STRUCTURES
	#define USE_C_VECTOR
	#define USE_C_STACK
	#define USE_C_HEAP
	#define USE_C_RINGBUFFER
#endif


#ifdef USE_C_VECTOR
	#undef USE_STL_VECTOR
#endif

#ifdef USE_CPP_VECTOR
	#undef USE_STL_VECTOR
#endif

#ifdef USE_C_STACK
	#undef USE_STL_STACK
#endif

#ifdef USE_C_HEAP
	#undef USE_CPP_HEAP
#endif

#ifdef USE_C_RINGBUFFER
	#undef USE_STL_RINGBUFFER
#endif
















#ifndef USE_CPP_CLAUSE
	
#endif




#ifdef USE_CPP_CACHE_CLAUSE
	#undef USE_CPP_CLAUSE
#endif

#ifndef USE_RISS_CLAUSE
	#undef USE_CPP_CLAUSE
#endif






#ifndef USE_C_VECTOR
	#ifdef USE_STL_VECTOR
		#define VEC_TXT "STL_VECTOR"
	#else
		#define VEC_TXT ""
	#endif
#else
	#ifdef USE_C_VECTOR
		#define VEC_TXT "C_VECTOR"
	#else
		#ifdef USE_CPP_VECTOR
			#define VEC_TXT "CPP_VECTOR"
		#else
			#define VEC_TXT ""
		#endif
	#endif
#endif


#ifndef USE_C_STACK
	#ifdef USE_STL_STACK
		#define STACK_TXT "STL_STACK"
	#else
		#define STACK_TXT ""
	#endif
#else
	#define STACK_TXT "C_STACK"
#endif


#ifndef USE_C_HEAP
	#ifdef USE_CPP_HEAP
		#define HEAP_TXT "CPP_HEAP"
	#else
		#define HEAP_TXT ""
	#endif
#else
	#define HEAP_TXT "C_HEAP"
#endif


#ifndef USE_C_RINGBUFFER
	#ifdef USE_STL_RINGBUFFER	
		#define RINGBUFFER_TXT "STL_RINGBUFFER"
	#else
		#define RINGBUFFER_TXT ""
	#endif
#else
	#define RINGBUFFER_TXT "C_RINGBUFFER"
#endif


#ifdef EXPERIMENTAL_ASSIGNMENT

#else
	#define ASSIGNMENT_TXT "NORMAL"
#endif


#ifdef EXPERIMENTAL_BOOLARRAY

#else
	#define BOOLARRAY_TXT "NORMAL"
#endif


#ifdef USE_CPP_CLAUSE
	#define CL_TXT "CPP_CLAUSE"
#else
	#ifdef USE_CPP_CACHE_CLAUSE
	  #define CL_TXT "CPP_CACHE_CLAUSE"
	#else
		#ifdef USE_RISS_CLAUSE
			#define CL_TXT "RISS_CLAUSE"
		#else
		#endif
	#endif
#endif


inline std::string getBuildFlagList()
{
	std::stringstream s;
	s << VEC_TXT << ",";
	s << STACK_TXT << ",";
	s << HEAP_TXT << ",";
	s << RINGBUFFER_TXT << ",";
	s << "ASSIGNMENT(" << ASSIGNMENT_TXT << "),";
	s << "BOOLARRAY(" << BOOLARRAY_TXT << "),";
	s << CL_TXT << ",";
	s << "CLAUSE PADDING(";
#ifdef USE_COMPONENTS
	s << 0;
#else
	s << 1;
#endif
	s << "),";
	s << "COMPONENTS(";
#ifdef USE_COMPONENTS
	s << 1;
#else
	s << 0;
#endif
	s << ")" << ",";
	s << "PARAMETER(";
#ifdef USE_COMMANDLINEPARAMETER
	s << 1;
#else
	s << 0;
#endif
	s << ")" <<",";
	s << "STATISTICS(";
#ifdef COLLECT_STATISTICS
	s << 1;
#else
	s << 0;
#endif
	s << ")";
	return s.str();
}

#endif
