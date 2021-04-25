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


void print_defines();

#include <fstream>
#include <iostream>
#include "stdlib.h"

#include <libpd/libpd.h>

#include "defines.h"
#include "macros/clause_macros.h"
#include "sat/searchdata.h"
#include "utils/commandlineparser.h"

#include "info.h"
#include "utils/statistics.h"
#include "utils/monitor.h"

#include "utils/varfileparser.h"

#ifdef CSPSOLVER
	#include "csp/crissspmain.h"
#else
	#include "sat/rissmain.h"
#endif

using namespace std;

int32_t main(int32_t argc, char* argv[])
{

	
	
	
	

	if( false ){
		VarFileParser vfp("elim.var");
		VEC_TYPE(var_t) vars;
		VEC_CREATE(var_t,vars);

		vfp.extract(vars);
		cerr << "extracted vars:";
		for( uint32_t i = 0 ; i < VEC_SIZE(var_t,vars); ++i ){
			cerr << " " << vars[i];
		} cerr << endl;
		exit(0);
	}

	LIBPD_INIT();

	cerr << "c *****************************************************" << endl;
#ifdef CSPSOLVER
	cerr << "c *      CrissSP, Copyright 2010 Norbert Manthey      *" << endl;
#endif
#ifdef SATSOLVER
	cerr << "c *        riss, Copyright 2009 Norbert Manthey       *" << endl;
#endif
	cerr << "c *   This program may be redistributed or modified   *" << endl;
	cerr << "c * under the terms of the GNU General Public License *" << endl;
	cerr << "c *****************************************************" << endl;
	cerr << "c process id: " << getpid() << endl;

	StringMap commandline;
	CommandLineParser cmp;
	commandline = cmp.parse(argc, argv);
	

	if( argc < 2 && !commandline.contains( (const char*)"-h" ) 
			&& !commandline.contains( (const char*)"--help" ) 
			&& !commandline.contains( (const char*)"file" ) 
		)
	{
		cerr << "use the -h parameter for more help." << endl << endl;
#ifndef COMPETITION
		if( commandline.contains( (const char*)"-i" ) ) print_build_info();
		if( commandline.contains( (const char*)"-v" ) ) print_defines();
#endif
		exit( 0 );
	}

	
#ifndef COMPETITION
	if( commandline.contains( (const char*)"-i" ) ) print_build_info();
	if( commandline.contains( (const char*)"-v" ) ) print_defines();
#endif

	int32_t ret = 0;
#ifdef CSPSOLVER
	ret = crissspmain( commandline );
#else
	ret = rissmain( commandline );
#endif
	MON_WAIT_WINDOW();
	return ret;
}


void print_defines()
{
	cerr << "c --------------------" << endl;
#ifdef CSPSOLVER
	cerr << "c CrissSP" << endl;
#endif
#ifdef SATSOLVER
	cerr << "c riss" << endl;
#endif
	cerr << "c datastructure built parameter:" << endl;
	cerr << "c vector type:         " << VEC_TXT << endl;
	cerr << "c stack type:          " << STACK_TXT << endl;
	cerr << "c heap type:           " << HEAP_TXT << endl;
	cerr << "c ringbuffer type:     " << RINGBUFFER_TXT << endl;
	cerr << "c assignment type:     " << ASSIGNMENT_TXT << endl;
	cerr << "c boolarray type:      " << BOOLARRAY_TXT << endl;
	cerr << "c clause type:         " << CL_TXT << endl;
#ifdef COMPRESS_CLAUSE
	cerr << "c clause padding:      no" << endl;
#else
	cerr << "c clause padding:      yes" << endl;
#endif
#ifdef USE_C_CLAUSE	
	cerr << "c clause size:         " << "flatten" << endl;
#else
	cerr << "c clause size:         " << CL_OBJSIZE << endl;
#endif	
#ifdef COMPRESS_WATCH_STRUCTS
	cerr << "c watch struct padding:no" << endl;
#else
	cerr << "c watch struct padding:yes" << endl;
#endif
#ifdef USE_SLAB_MALLOC
	cerr << "c slab malloc lits:    yes" << endl;
#else
	cerr << "c slab malloc lits:    no" << endl;
#endif
	cerr << "c max slab malloc:     " << SLAB_MALLOC_MAXSIZE << endl;
#ifndef USE_ALLIGNED_MALLOC
	cerr << "c malloc allignment:   none" << endl;
#else
	cerr << "c malloc allignment:   64" << endl;
#endif
#ifdef BLOCKING_LIT
	cerr << "c blocking literals:   1" << endl;
#else
	cerr << "c blocking literals:   0" << endl;
#endif
#ifdef USE_IMPLICIT_BINARY
	cerr << "c implicit binaries:   1" << endl;
#else
	cerr << "c implicit binaries:   0" << endl;
#endif
#ifndef KEEP_SAT_IN_WATCH
	cerr << "c keep sat in watch:   no" << endl;
#else
	cerr << "c keep sat in watch:   yes" << endl;
#endif
#ifdef USE_PREFETCHING
	cerr << "c prefetch in dualprop:yes" << endl;
#else
	cerr << "c prefetch in dualprop:no" << endl;
#endif
#ifdef USE_DUAL_PREFETCHING
	cerr << "c prefetch dual watchs:yes" << endl;
#else
	cerr << "c prefetch dual watchs:no" << endl;
#endif
#ifdef PREFETCHINGMETHOD1
	cerr << "c prefetch:            current" << endl;
#else 
	cerr << "c prefetch:            some ahead" << endl;
#endif
#ifndef USE_CONFLICT_PREFETCHING
	cerr << "c prefetch analysis:   no" << endl;
#else
	cerr << "c prefetch analysis:   yes" << endl;
#endif
#ifndef USE_SAME_LEVEL_PREFETCHING
	cerr << "c prefetch same level: no" << endl;
#else
	cerr << "c prefetch same level: yes" << endl;
#endif
#ifdef COMPACT_VAR_DATA
	cerr << "c compact var data:    yes" << endl;
#else
	cerr << "c compact var data:    no" << endl;
#endif
	cerr << "c reason struct size:  " << sizeof(reasonStruct) << endl;
#ifdef BAD_STRUCTURES
	cerr << "c data structures:     bad" << endl;
#endif
#ifdef GOOD_STRUCTURES
	cerr << "c data structures:     good" << endl;
#endif
#ifdef OTFSS
	cerr << "c oftss:               yes" << endl;
#else
	cerr << "c oftss:               no" << endl;
#endif
#ifndef ASSI_AGILITY
	cerr << "c assignment agility:  no" << endl;
#else
	cerr << "c assignment agility:  yes" << endl;
#endif
#ifdef USE_ALL_COMPONENTS
	cerr << "c component system:    all" << endl;
#else
	#ifdef USE_SOME_COMPONENTS
	cerr << "c component system:    some" << endl;
	#endif
#endif
#ifndef USE_ALL_COMPONENTS
	#ifndef USE_SOME_COMPONENTS
	cerr << "c component system:    none" << endl;
	#endif
#endif
#ifdef USE_COMMANDLINEPARAMETER
	cerr << "c parameter system:    enabled" << endl;
#else
	cerr << "c parameter system:    disabled" << endl;
#endif
#ifdef COLLECT_STATISTICS
	cerr << "c statistic system:    enabled" << endl;
#else
	cerr << "c statistic system:    disabled" << endl;
#endif
#ifndef TRACK_CLAUSE
	cerr << "c track clauses:       no"  << endl;
#else
	cerr << "c track clauses:       yes"  << endl;
#endif
#ifdef PARALLEL
	cerr << "c use max threads:     " << MAX_THREADS << endl;
#else
	cerr << "c use threads:         " << 1 << endl;
#endif
	cerr << "c thread info size     " << sizeof(Network::threadInfo) << endl;
	cerr << "c --------------------" << endl;
}
