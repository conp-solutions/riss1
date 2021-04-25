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


#ifndef _DEBUG_H
#define _DEBUG_H


#include <iostream>
#include <sstream>
using namespace std;

#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "sat/searchdata.h"




#ifdef DODEBUG
	#define DO(x) x
#else
	#define DO(x)
#endif


inline void printClause(  const searchData& sd, CL_REF clause ){
	CLAUSE& cl = gsa.get(clause);
	stringstream s;
	uint32_t size = CL_SIZE(cl);
	if( cl.isXor() ) s << "XOR";
	for( uint32_t i = 0 ; i < size; i++ ){
		lit_t literal = CL_GET_LIT(cl,i) ;
		int level = sd.VAR_LEVEL( var(literal) );
		s << " " << nr( CL_GET_LIT(cl,i) ) << "@" << level << "p";
		if( assi_is_sat( sd.assi, literal ) ) s << "+";
		else if( assi_is_unsat( sd.assi, literal) ) s << "-";
		else s << "u";
	}
	s << endl;
	cerr << s.str();
}


inline void printClause( CL_REF clause ){
	CLAUSE& cl = gsa.get(clause);
	const uint32_t size = CL_SIZE(cl);
	if( cl.isXor() ) cerr << "XOR";
	for( uint32_t i = 0 ; i < size; i++ ){
		cerr << " " << nr( CL_GET_LIT(cl,i) );
	}
}


inline void dumpTrail( const searchData& sd ){
	cerr << "c ===================" << endl;
	cerr << "c level: " << sd.current_level;
	cerr << " size:  " << STACK_SIZE( lit_t, sd.trail ) << endl;
	cerr << "c trail: " << endl;
	for( uint32_t i = 0 ; i < STACK_SIZE( lit_t, sd.trail ); i++ ){
		lit_t literal = sd.trail[i];
		int level = sd.VAR_LEVEL( var(literal) );
		reasonStruct reason = sd.VAR_REASON( var(literal) );
		cerr << "c  " << nr( sd.trail[i] ) << " @ " << level;
		if( reason.isLit() ) cerr << " L " << nr( reason.getLit() );
		else {
			cerr << " C " << reason.getCL() << " | .";
			
			printClause( sd, reason.getCL());
		}
		cerr << endl;
	
	}
	cerr << "c ===================" << endl;
}


#endif
