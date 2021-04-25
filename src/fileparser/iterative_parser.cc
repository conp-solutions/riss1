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


#include "fileparser/iterative_parser.h"

IterativeFileParser::IterativeFileParser()
{
	print_dimacs = true;
}

IterativeFileParser::IterativeFileParser( const StringMap& commandline)
{
	print_dimacs = false;
	parse = !( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) );
#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"Fp_print" ) ) print_dimacs = 0 != atoi( commandline.get( (const char*)"Fp_print" ).c_str() );
#endif
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		cerr << "=== satellike preprocessor information ===" << endl;
		cerr << " parameter values info" << endl;
		cerr << " Fp_print  0,1    printes file with according p Line" << endl;
		cerr << endl;
	}	
#endif
}

solution_t IterativeFileParser::parse_file(char* filename, uint32_t & var_cnt, VEC_TYPE( CL_REF )* clauses )
{
	if(!parse) return UNKNOWN;
	
	#ifdef TRACK_CLAUSE
		VEC_TYPE( lit_t ) track_lits;
		int32_t i = 0;
		cerr << "c type clause that has to be checked (ending symbol is a 0)" << endl;
		do {
			cin >> i;
			if( i != 0 )
			VEC_PUSH_BACK( lit_t, track_lits, lit(i) );
		} while ( i != 0 );

		if( VEC_SIZE(lit_t,  track_lits ) > 0 ) selectionsort<lit_t>( &(track_lits[0]) , VEC_SIZE(lit_t,  track_lits ) );
		
		cerr << "c clause to track:";
		for( uint32_t i = 0; i < VEC_SIZE(lit_t,track_lits); ++i ) cerr << " " << nr(track_lits[i]);
		cerr << endl;
	#endif

	
	VEC_TYPE( CL_REF )& clause_set = *clauses;
	istream* fileptr;

	
	if( filename == 0 || strlen(filename) == 0 )
	{
		cerr << "c no problem file specified, use stdin" << endl;
		
		fileptr = &cin;
	} else {
		fileptr = new ifstream( filename, ios_base::in);
	}

	istream& file = *fileptr;

	if( !file )
	{
		cerr << "c can not open file " << filename << endl;
		exit( 0 );
	}

	VEC_TYPE( lit_t ) clause_lits;
	VEC_CREATE_SPACE(lit_t, clause_lits, 120 );	
	string line;		
	uint32_t ind = 0;	
	uint32_t lines = 0;	
	
	while(getline (file, line)){
	 
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
		
		if(line[ind] == '{' ){	
			while( line[ind] != '}' ) ind ++;		
		}
		
		
		if(line[ind] == '0'){
			
			for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF,  clause_set ); ++i ){
				CL_DESTROY( gsa.get(clause_set[i]) );
				gsa.release( clause_set[i] );				
			}
			VEC_DESTROY( CL_REF, clause_set);
			if( fileptr != &cin ) delete fileptr;
			return UNSAT;
		}

		
		if( line[ind] != '-' && ( line[ind] < '0' || line[ind] > '9' ) )
		{
			cerr << "invalid file ( symbol[" << (int)line[ind] << ", " << line[ind] << " ] in line " << lines << " at col " << ind << " : " << line << " )" << endl;
			exit(0);
		}

		
		VEC_CLEAR(lit_t,  clause_lits );
		while(line.size() > ind)	
		{
			
			int32_t number = 0;
			bool negative = false;
		
			if(line[ind] == '-')
			{
				negative = true;
				ind++;
			}
			
			while( line[ind] >= '0' && line[ind] <= '9' )
			{
				number *=10;
				number += line[ind++] - '0';
			}
			
			if( number == 0 ) break;	
			
			
			number = (negative) ? 0 - number : number;

			const int32_t lit1 = lit( number );
			VEC_PUSH_BACK(lit_t,  clause_lits, lit1 );
			
			if( var_cnt < var(lit1) ) var_cnt = var( lit1 );
		
			
			while(line[ind] == ' ')
			 ind++;		
		}
		
		
		if( VEC_SIZE( lit_t, clause_lits ) == 0 ){
			if( fileptr != &cin ) delete fileptr;
			return UNSAT;
		}
		
		const CL_REF ref = addLiterals( clause_lits );
		if( ref != 0 ){
			VEC_PUSH_BACK( CL_REF, clause_set, ref );
			
			#ifdef TRACK_CLAUSE
				CLAUSE& tempC = gsa.get(ref);
				if( VEC_SIZE( lit_t, track_lits ) == CL_SIZE( tempC ) ){
					uint32_t i = 0;
					for( ; i < CL_SIZE( tempC ); ++i ){
						if( track_lits[i] != CL_GET_LIT(tempC, i ) ) break;
					}
					if( i == CL_SIZE( tempC ) ){
						cerr << "c found clause to track" << endl;
						tempC.setFlag4(true);
					}
				}
			#endif
		}
	 }
	 
	  if(print_dimacs){
		cout << "p cnf " << var_cnt << " " << VEC_SIZE( CL_REF, (*clauses) ) << endl;
		for(uint32_t ind_clause = 0; ind_clause < VEC_SIZE( CL_REF, (*clauses) ); ++ind_clause)
		{
			CLAUSE& cl = gsa.get((*clauses)[ ind_clause ] );
			for( uint32_t i = 0; i < CL_SIZE( cl); ++i ) cout << nr( CL_GET_LIT( cl, i ) ) << " ";
			cout << "0" << endl;
		}
		exit(12);
	  }
 
	VEC_DESTROY(lit_t, clause_lits);
	if( fileptr != &cin ) delete fileptr;
	return UNKNOWN;

}

CL_REF IterativeFileParser::addLiterals( lit_t* clause_lits, uint32_t size ) const {
	
	selectionsort<lit_t>( clause_lits , size );
	uint32_t i = 2;
	for( i = size - 1; i > 0; i-- ){
		
		if( clause_lits[i] == inv_lit( clause_lits[i-1] ) ) break;
		
		if( clause_lits[i] == clause_lits[i-1] ){
			for( uint32_t j = i+1; j < size - 1; ++j ) clause_lits[j] = clause_lits[j+1];
			--size;
		}
	}
	if( i == 0 ){
		return gsa.create( CL_CREATE_NO_LEARNT( clause_lits , size ) );
	}
	return 0;
}

CL_REF IterativeFileParser::addLiterals( VEC_TYPE(lit_t)& clause_lits ) const
{
	
	selectionsort<lit_t>( &(clause_lits[0]) , VEC_SIZE(lit_t,  clause_lits ) );
	uint32_t i = 2;
	for( i = VEC_SIZE(lit_t,  clause_lits ) - 1; i > 0; i-- ){
		
		if( clause_lits[i] == inv_lit( clause_lits[i-1] ) ) break;
		
		if( clause_lits[i] == clause_lits[i-1] ){
			VEC_ERASE(lit_t,  clause_lits, i );
		}
	}
	if( i == 0 ){
		return gsa.create( CL_CREATE_NO_LEARNT( &(clause_lits[0]), VEC_SIZE(lit_t,  clause_lits ) ) );
	}
	return 0;
}
