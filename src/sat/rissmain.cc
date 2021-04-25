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

#include "sat/rissmain.h"

#include "utils/network.h"

#ifdef SATSOLVER

void printMemory(){
	
	char buf[100];
	uint32_t memory = 0;
	snprintf(buf, 99, "/proc/%u/stat", (unsigned)getpid() );
	FILE* pf = fopen(buf, "r");
	if (pf) {
		if (fscanf(pf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*e %u", &memory) != 1)
		memory = 0;
	}
	std::cerr << "c memory usage (vsize): " << memory / 1024 / 1024 << " MB" << std::endl;
 	fclose(pf);
}

int32_t rissmain( StringMap& commandline )
{
	uint32_t var_cnt = 0;							
	solution_t solution = UNKNOWN;	
	
	VEC_TYPE(assi_t)* solutions = 0;	

	
	ComponentManager cm( commandline );
	Network nw( commandline );

	
	int32_t seed = 1201742309; 
	if( commandline.contains( "seed" ) ){
		seed = atoi( commandline.get( "seed" ).c_str() );
	}
	srandom( seed );

	
	VEC_TYPE( CL_REF )* clause_set;
	VEC_CREATE_SPACE_IN_HEAP( CL_REF, (clause_set), 32000 );
	
	uint64_t parse_time = get_microseconds();
	
	if( !commandline.contains( (const char*)"file" ) ){
		std::cerr << "c waiting for file on stdin" << std::endl;
		commandline.insert( (char*)"file_reader", (char*)"stdin" );
	}
	
	solution = cm.parse_file( (char*)commandline.get( (const char*)"file" ).c_str(), var_cnt, clause_set, commandline );
	parse_time = get_microseconds() - parse_time;
	if( solution == UNSAT ) std::cerr << "c file parser recognized an empty clause" << std::endl;

	#ifdef USE_CPP_COMPRESS_CLAUSE
		if( solution==UNKNOWN && 1048575 < var_cnt ){
			std::cerr << "compress clause can not handle such high numbers of variables!" << std::endl;
			exit( 0 );
		}
	#endif
	std::cerr << "c parsetime: " << parse_time << "us" << std::endl;

	if( clause_set != 0 )	std::cerr << "c parsed " << VEC_SIZE( CL_REF,  (*clause_set) ) << " clauses with " << var_cnt << " variables" << std::endl;
	{
		STAT_INIT( commandline, (clause_set != 0) ? VEC_SIZE( CL_REF,  (*clause_set) ) : 0, var_cnt );
	}
	
	
	if( solution==UNKNOWN ){
		solutions = nw.solve( clause_set, var_cnt, commandline, cm);
	}
	
	printMemory();

	if( solution == UNSAT || VEC_SIZE(assi_t,  (*solutions) ) == 0 ){
		std::cout << "s UNSATISFIABLE" << std::endl;
		if( commandline.contains( (const char*)"outfile" ) ){
			std::ofstream solutionFile( commandline.get( (const char*)"outfile" ).c_str() );
			solutionFile << "s UNSATISFIABLE" << std::endl;
		}
		solution = UNSAT;
	} else {
		std::cout << "s SATISFIABLE" << std::endl;
		cerr << "c solutions: " << VEC_SIZE(assi_t,  (*solutions) ) << endl;
		
		if( !commandline.contains( (const char*)"-q" ) && !commandline.contains( (const char*)"outfile" ))
		{
			for( int32_t i = VEC_SIZE(assi_t,  (*solutions) ) ; i > 0; --i )
			{
				assi_t solution = (*solutions)[i-1];
				std::cout << "v";
				for( uint32_t i = 1 ; i<=var_cnt; ++i ) std::cout << " " << nr( lit(i, assi_get_polarity(solution, i) ) );
				std::cout << " 0" << std::endl; 
			}
		} else {
			if( commandline.contains( (const char*)"outfile" ) ){
				std::ofstream solutionFile( commandline.get( (const char*)"outfile" ).c_str() );
				solutionFile << "s SATISFIABLE" << std::endl;
				if( !commandline.contains( (const char*)"-q" ) ){
					for( int32_t i = VEC_SIZE(assi_t,  (*solutions) ) ; i > 0; --i ){
						assi_t solution = (*solutions)[i-1];
						solutionFile << "v";
						for( uint32_t i = 1 ; i<=var_cnt; ++i ) solutionFile << " " << nr( lit(i, assi_get_polarity(solution, i) ) );
						solutionFile << " 0" << std::endl; 
					}
				}
			}
		}
		solution = SAT;
	}
	

	STAT_SAT( solution == SAT );
#ifndef PARALLEL
#ifndef COMPETITION
	if( false ){ 
		
		if( solutions != 0 ){
			for( int32_t i = VEC_SIZE(assi_t,  (*solutions) ) ; i > 0; --i ){
				assi_destroy( (*solutions)[i-1],var_cnt );
			}
			VEC_DESTROY_IN_HEAP(assi_t, solutions );
		}

		if( clause_set != 0 ){
			for(uint32_t i = 0; i < VEC_SIZE( CL_REF,  (*clause_set) ); ++i ){
				CL_DESTROY( gsa.get( (*clause_set)[i] ) );
				gsa.release( (*clause_set)[i] );
			}
			VEC_DESTROY_IN_HEAP(CL_POINTER,  clause_set );
		}
	}
	std::cerr << "c return value: " << (int)solution << std::endl;
	
	STAT_STOREDATA( "statistic_data" );
#endif
#endif

#ifndef COMPETITION
	exit( (int)solution );

#else
	exit( (int)solution );
#endif
}

#endif 
