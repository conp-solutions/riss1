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


#ifndef _CLAUSEHANDLER_H
#define _CLAUSEHANDLER_H

#include <iostream>

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>

#include <inttypes.h>

#include "macros/vector_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "structures/literal_system.h"

#include "utils/sort.h"


class ClauseHandler{
	std::fstream file;
public:
	ClauseHandler(const char* filename);
	~ClauseHandler();
	
	void write( const CL_REF clause );
	void read( VEC_TYPE( CL_REF )& clauses );
};




inline ClauseHandler::ClauseHandler(const char* filename)
{
	file.open(filename, std::ios::app | std::ios_base::out  |  std::ios_base::in );
}

inline  ClauseHandler::~ClauseHandler()
{
	file.close();
}

inline void ClauseHandler::write( const CL_REF clause )
{
	CLAUSE& cl = gsa.get(clause);
	for( uint32_t i = 0; i < CL_SIZE( cl ); ++i )
	{
		file << nr( CL_GET_LIT( cl, i ) ) << " ";
	}
	file << "0" << std::endl;
}

inline void ClauseHandler::read( VEC_TYPE( CL_REF )& clauses )
{
	file.seekg( 0 );
	std::string line;
	uint32_t lines = 0;
	uint32_t ind = 0;
	VEC_TYPE( lit_t ) clause_lits;
	VEC_CREATE_SPACE(lit_t, clause_lits, 120 );	
	uint32_t var_cnt = 0;
	while(getline (file, line))
	{
		lines ++;
		ind = 0;
		if(line.size() == 0)
		 continue;
		
		while(line[ind] == ' ')
		 ind++;
		
		if(line[ind] == 10 || line[ind] == 13 || line[ind] == 0)
			continue;
		if(line[ind] == 'p')	
		 continue;
		if(line[ind] == 'c' || line[ind] == 'C')	
		 continue;
		
		if(line[ind] == '0'){
			VEC_PUSH_BACK( CL_REF, clauses, gsa.create( CL_CREATE_EMPTY() ) );
		}
		
		if( line[ind] != '-' && ( line[ind] < '0' || line[ind] > '9' ) ){
			std::cerr << "invalid learnt file ( symbol[" << (int)line[ind] << ", " << line[ind] << " ] in line " << lines << " at col " << ind << " : " << line << " )" << std::endl;
			continue;
		}
		VEC_CLEAR(lit_t,  clause_lits );
		while(line.size() > ind)	
		{
			int32_t number = 0;
			bool negative = false;
			if(line[ind] == '-'){
				negative = true;
				ind++;
			}
			
			while( line[ind] >= '0' && line[ind] <= '9' ){
				number *=10;
				number += line[ind++] - '0';
			}
			if( number == 0 ) break;	
			
			number = (negative) ? 0 - number : number;

			const int32_t lit1 = lit( number );
			VEC_PUSH_BACK(lit_t,  clause_lits, lit1 );
			
			if( var_cnt < (uint32_t)var(lit1) ) var_cnt = var( lit1 );
		
			
			while(line[ind] == ' ') ind++;		
		}
		
		selectionsort<lit_t>( &(clause_lits[0]) , VEC_SIZE(lit_t,  clause_lits ) );
		uint32_t i = 2;
		for( i = VEC_SIZE(lit_t,  clause_lits ) - 1; i > 0; i-- )
		{
			
			if( clause_lits[i] == inv_lit( clause_lits[i-1] ) ) break;
			
			if( clause_lits[i] == clause_lits[i-1] )
			{
				VEC_ERASE(lit_t,  clause_lits, i );
			}
		}
		
		if( i == 0 )	
		{
	 		VEC_PUSH_BACK( CL_REF, clauses, gsa.create( CL_CREATE( &(clause_lits[0]), VEC_SIZE(lit_t,  clause_lits ) ) ) );
		}
	}
	file.clear();
}

#endif
