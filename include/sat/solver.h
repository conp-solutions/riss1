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

#include "defines.h"
#ifdef SATSOLVER

#ifndef _SOLVER
#define _SOLVER


#include "types.h"

#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "structures/c_assignment.h"

#include "utils/statistics.h"
#include "utils/microtime.h"
#include "utils/stringmap.h"

#include "utils/cnfinfo.h"
#include "utils/network.h"

#include "component.h"

#include "search/cdclsearch.h"

#include <libpd/libpd.h>

LIBPD_IMPORT(preprocesstime, libpd_time_t)
LIBPD_IMPORT(searchtime, libpd_time_t)
LIBPD_IMPORT(postprocesstime, libpd_time_t)


class Solver
{
public:
	
	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH>
	VEC_TYPE( assi_t )* solve( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw )
	{
		
		
		VEC_TYPE(assi_t)* solutions = 0;
	
		
		solution_t solution = UNKNOWN; 

		
		PRP preprocessor( clause_set, var_cnt, commandline );
		
		
		searchData sd( var_cnt );
		
		if( !(commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) ) ){
			if( clause_set != 0 ){

				LIBPD_START_TIMER( preprocesstime );
				uint64_t pp_time = get_microseconds();
				solution = preprocessor.preprocess( sd, clause_set);
				pp_time = get_microseconds() - pp_time;
				LIBPD_END_TIMER( preprocesstime );
#ifdef PARALLEL
				cerr << "c preprocessor returned " << sd.var_cnt << " variables" << endl;
				switch( solution )
				{
					case UNKNOWN: std::cerr << "c preprocessing returned UNKNOWN" << std::endl; break;
					case SAT: std::cerr << "c preprocessing returned SAT" << std::endl; break;
					case UNSAT: std::cerr << "c preprocessing returned UNSAT" << std::endl; break;
				}
				
				uint32_t c = 0 ;
				for( var_t v = 0 ; v <=var_cnt; ++v ){
					c = ( assi_is_undef( sd.assi, v) ) ? c : c+1;
				}
				std::cerr << "c PP set " << c << " variables" << std::endl;
				
				std::cerr << "c pp time: " << pp_time << "us (" << pp_time/(uint64_t)1000000 << "s)" << std::endl;
#endif
			}
			
			if( commandline.contains( (const char*)"info_level" ) ){
				CnfInfo ci(commandline);
				ci.analyze( clause_set, var_cnt, sd.assi );
			}
		}
		
		
		uint64_t solve_time = get_microseconds(); 
		if( solution != UNSAT ){
			{
				STAT_PREPRO((clause_set!=0) ? VEC_SIZE( CL_REF,  (*clause_set) ) : 0 , var_cnt );
			}
			
			if( commandline.contains( (const char*)"-print_formula" )){
				std::cout << "p cnf " << var_cnt << " " << VEC_SIZE( CL_REF, (*clause_set) ) << std::endl;
				for(uint32_t ind_clause = 0; ind_clause < VEC_SIZE( CL_REF, (*clause_set) ); ++ind_clause)
				{
					CLAUSE& cl = gsa.get((*clause_set)[ ind_clause ]);
					for( uint32_t i = 0; i < CL_SIZE( cl ); ++i ) std::cout << nr( CL_GET_LIT( cl, i ) ) << " ";
					std::cout << "0" << std::endl;
				}
				exit(12);
			}
			LIBPD_START_TIMER( searchtime );
			CdclSearch< UPR,DEH,RSE,RME,ANA,PRP,CAH, RMH > search( sd, clause_set, preprocessor, commandline, nw, -1 );

			solutions = search.search();	
			LIBPD_END_TIMER( searchtime );
			
		}
		else {
			STAT_PREPRO( 0, var_cnt );
			VEC_CREATE_IN_HEAP( assi_t, solutions );	
		}
		solve_time = get_microseconds() - solve_time;

		if( nw.doPrint() )	std::cerr << "c solvetime: " << solve_time << "us (" << solve_time/(uint64_t)1000000 << "s)" << std::endl;

		#ifdef TRACK_CLAUSE
			for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clause_set)); ++i) {
				const CL_REF clause = (*clause_set)[i];
				CLAUSE & cl = gsa.get (clause);
				if( cl.getFlag4() ) {
					cerr << " tracked clause is still in formula " << endl;
					for( uint32_t j =0; j < CL_SIZE(cl); ++j ){
						if( assi_is_sat( (*solutions)[0],
							 CL_GET_LIT(cl,j) ) ) cerr << "is satisfied by literal " << nr(CL_GET_LIT(cl,j)) << endl;
					}
				}
			}
		#endif


		LIBPD_START_TIMER( postprocesstime );
		
		if( VEC_SIZE( assi_t, (*solutions) ) > 0 )	preprocessor.postprocess( solutions );  
		LIBPD_END_TIMER( postprocesstime );

		return solutions;
	}
};

#endif

#endif 
