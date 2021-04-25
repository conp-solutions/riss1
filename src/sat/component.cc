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

#include "sat/component.h"
#ifdef SATSOLVER

#include "sat/solver.h"



ComponentManager::ComponentManager() {}


ComponentManager::ComponentManager( StringMap& commandline )
{
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		
		std::cerr << "=== component information ===" << std::endl;
#ifndef TESTBINARY
	#ifndef USE_ALL_COMPONENTS
			std::cerr << "!!!                                           !!!" << std::endl;
			std::cerr << "!!!    THIS IS NOT FULLY ENABLED RIGHT NOW    !!!" << std::endl;
			std::cerr << "!!!  use the USE_ALL_COMPONENTS compile flag  !!!" << std::endl;		
	#ifdef USE_SOME_COMPONENTS
				std::cerr << "!!!      some components can be choseen       !!!" << std::endl;
		#else
				std::cerr << "!!! use the USE_SOME_COMPONENTS compile flag  !!!" << std::endl;
	#endif
			std::cerr << "!!!                                           !!!" << std::endl;
	#endif
#endif
		std::cerr << " parameter         pos.value" << std::endl;
		
		std::cerr << " clause_activity   no,linear" << std::endl;
		std::cerr << " decision          var_activity" << std::endl;
		std::cerr << " propagation       dual,Pwatch,csp" << std::endl;
		std::cerr << " conflict_analysis cdcl_min" << std::endl;
		std::cerr << " restart_event     geometric,luby,dynamic" << std::endl;
		std::cerr << " removal_event     geometric,luby,cls_ratio" << std::endl;
		std::cerr << " preprocessor      satellike,no,coprocessor" << std::endl;
		std::cerr << " removal_heuristic activity,suffix" << std::endl << std::endl;
#ifndef TESTBINARY
			std::cerr << "here are only parameter of the activated components shown!" << std::endl << std::endl;
	#ifndef USE_COMMANDLINEPARAMETER
			std::cerr << "!!!                                                    !!!" << std::endl;
			std::cerr << "!!!           THIS IS NOT ENABLED RIGHT NOW            !!!" << std::endl;
			std::cerr << "!!!   use the USE_COMMANDLINEPARAMETER compile flag    !!!" << std::endl;		
			std::cerr << "!!!                                                    !!!" << std::endl;
	#endif
#endif
	}
}

solution_t ComponentManager::parse_file(char* filename, uint32_t & var_cnt, VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline)
{
	std::string file_reader = commandline.get( (const char*)"file_reader" );
	
	
	IterativeFileParser fp(commandline);
	return fp.parse_file( filename, var_cnt, clause_set);
}

VEC_TYPE( assi_t )* ComponentManager::solve( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline, Network& nw)
	{
		std::string propagation = commandline.get( (const char*)"propagation" );
		
		if( commandline.contains( (const char*)"-v" ) ){
			std::cerr << "c --------------------" << std::endl;
			std::cerr << "c activated components:" << std::endl;
			if( propagation.empty() ) std::cerr << "c propagation: " << "dual" << std::endl;
			else std::cerr << "c propagation: " << propagation << std::endl;
		}

		if( propagation.empty() || propagation.compare( (const char*)"dual" ) == 0 ){
			return solve_UP<DualUnitPropagation>(  clause_set, var_cnt, commandline, nw);
		}
#ifndef TESTBINARY
		if( propagation.empty() || propagation.compare( (const char*)"Pwatch" ) == 0 ){
			return solve_UP<PWatchedPropagation>( clause_set, var_cnt, commandline, nw);
		}
		if( propagation.empty() || propagation.compare( (const char*)"csp" ) == 0 ){
			return solve_UP<CspPropagation>(  clause_set, var_cnt, commandline, nw);
		}
#endif
#ifdef USE_SOME_COMPONENTS

#ifdef USE_ALL_COMPONENTS
			
#endif
#endif			

		std::cerr << "wrong propagation set! (" << propagation << ")" << std::endl; exit(0);
	}
	
	template<class UPR>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string decision =commandline.get( (const char*)"decision" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( decision.empty() ) std::cerr << "c decision: " << "var_activity" << std::endl;
			else std::cerr << "c decision: " << decision << std::endl;
		}
		if( decision.empty() || decision.compare( (const char*)"var_activity" ) == 0 )
		{
			return solve_UP_DEH<UPR,VariableActivityHeuristic>(  clause_set, var_cnt, commandline, nw);
		}

		 else {
#ifdef USE_SOME_COMPONENTS
#ifdef USE_ALL_COMPONENTS
			
#endif
#endif
			
		}
		std::cerr << "wrong decision heuristic set! (" << decision << ")" << std::endl; exit(0);		
	}
	
	template<class UPR, class DEH>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string restart_event =commandline.get( (const char*)"restart_event" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( restart_event.empty() ) std::cerr << "c restart_event: " << "luby" << std::endl;
			else std::cerr << "c restart_event: " << restart_event << std::endl;
		}
		if( restart_event.empty() || restart_event.compare(  (const char*)"luby" ) == 0 )
		{
			return solve_UP_DEH_RSE<UPR,DEH,LubyEventHeuristic>(  clause_set, var_cnt, commandline, nw);
		}
		if( restart_event.compare(  (const char*)"geometric" ) == 0 )
		{
			return solve_UP_DEH_RSE<UPR,DEH,GeometricEventHeuristic>(  clause_set, var_cnt, commandline, nw);
		}
		if( restart_event.compare(  (const char*)"dynamic" ) == 0 )
		{
			return solve_UP_DEH_RSE<UPR,DEH,DynamicEventHeuristic>(  clause_set, var_cnt, commandline, nw);
		}
#ifdef USE_SOME_COMPONENTS
#ifdef USE_ALL_COMPONENTS
			

#endif
#endif
		std::cerr << "wrong restart event heuristic set! (" << restart_event << ")" << std::endl; exit(0);
	}

	template<class UPR, class DEH, class RSE>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string removal_event =commandline.get( (const char*)"removal_event" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( removal_event.empty() ) std::cerr << "c removal_event: " << "geometric" << std::endl;
			else std::cerr << "c removal_event: " << removal_event << std::endl;
		}
		if( removal_event.empty() || removal_event.compare(  (const char*)"geometric" ) == 0 )
		{
				return solve_UP_DEH_RSE_RME<UPR,DEH,RSE,GeometricEventHeuristic>(  clause_set, var_cnt, commandline, nw);
		}
		 else {
#ifdef USE_SOME_COMPONENTS
		if(  removal_event.compare(  (const char*)"cls_ratio" ) == 0 )
		{
			return solve_UP_DEH_RSE_RME<UPR,DEH,RSE,ClsRatioEventHeuristic>(  clause_set, var_cnt, commandline, nw);
		}
#ifdef USE_ALL_COMPONENTS
			
			if( removal_event.compare(  (const char*)"luby" ) == 0 )
			{
					return solve_UP_DEH_RSE_RME<UPR,DEH,RSE,LubyEventHeuristic>(  clause_set, var_cnt, commandline, nw);
			}
			
#endif
#endif
			
		}
		std::cerr << "wrong removal event heuristic set! (" << removal_event << ")" << std::endl; exit(0);
	}

	template<class UPR, class DEH, class RSE, class RME>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE_RME( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw){
		std::string conflict_analysis =commandline.get( (const char*)"conflict_analysis" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( conflict_analysis.empty() ) std::cerr << "c conflict_analysis: " << "cdcl_min" << std::endl;
			else std::cerr << "c conflict_analysis: " << conflict_analysis << std::endl;
		}
		if( conflict_analysis.empty() || conflict_analysis.compare(  (const char*)"cdcl_min" ) == 0 )
		{
			return solve_UP_DEH_RSE_RME_ANA<UPR,DEH,RSE,RME,CdclMinimizeAnalyzer>(  clause_set, var_cnt, commandline, nw);
		}

		 else {
#ifdef USE_SOME_COMPONENTS
#ifdef USE_ALL_COMPONENTS

			
#endif
#endif
		}
		std::cerr << "wrong conflict_analysis set! (" << conflict_analysis << ")" << std::endl; exit(0);
	}

	template<class UPR, class DEH, class RSE, class RME, class ANA>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE_RME_ANA( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string preprocessor =commandline.get( (const char*)"preprocessor" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( preprocessor.empty() ) std::cerr << "c preprocessor: " << "coprocessor" << std::endl;
			else std::cerr << "c preprocessor: " << preprocessor << std::endl;
		}
		if(preprocessor.empty() ||  preprocessor.compare( (const char*)"coprocessor" ) == 0 ){
			return solve_UP_DEH_RSE_RME_ANA_PRP<UPR,DEH,RSE,RME,ANA,Coprocessor>(  clause_set, var_cnt, commandline, nw);
		}
		if( preprocessor.compare(  (const char*)"satellike" ) == 0 ){
			return solve_UP_DEH_RSE_RME_ANA_PRP<UPR,DEH,RSE,RME,ANA,SatellikePreprocessor>(  clause_set, var_cnt, commandline, nw);
		}
		if( preprocessor.compare(  (const char*)"no" ) == 0 )	
		{
			return solve_UP_DEH_RSE_RME_ANA_PRP<UPR,DEH,RSE,RME,ANA,NoPreprocessor>(  clause_set, var_cnt, commandline, nw);
		}
#ifndef TESTBINARY		

#endif
#ifdef USE_LIBRARY
		return solve_UP_DEH_RSE_RME_ANA_PRP<UPR,DEH,RSE,RME,ANA,LibPreprocessor>(  clause_set, var_cnt, commandline, nw);
#endif
	
		 else {
#ifdef USE_SOME_COMPONENTS

#ifdef USE_ALL_COMPONENTS
			
#endif
#endif
			
		}		
		std::cerr << "wrong preprocessor set! (" << preprocessor << ")" << std::endl; exit(0);
	}

	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE_RME_ANA_PRP( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string clause_activity =commandline.get( (const char*)"clause_activity" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( clause_activity.empty() ) std::cerr << "c clause_activity: " << "linear" << std::endl;
			else std::cerr << "c clause_activity: " << clause_activity << std::endl;
		}
		if( clause_activity.empty() || clause_activity.compare( (const char*)"linear" ) == 0 )
		{
			return solve_UP_DEH_RSE_RME_ANA_PRP_CAH<UPR,DEH,RSE,RME,ANA,PRP,LinClauseActivity>(  clause_set, var_cnt, commandline, nw);
		}

		 else {
#ifdef USE_SOME_COMPONENTS
#ifdef USE_ALL_COMPONENTS

			
#endif
#endif

		}

		std::cerr << "wrong clause_activity set! (" << clause_activity << ")" << std::endl; exit(0);
	}

	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE_RME_ANA_PRP_CAH( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		std::string removal_heuristic =commandline.get( (const char*)"removal_heuristic" );
		if( commandline.contains( (const char*)"-v" ) ){
			if( removal_heuristic.empty() ) std::cerr << "c removal_heuristic: " << "activity" << std::endl;
			else std::cerr << "c removal_heuristic: " << removal_heuristic << std::endl;
			std::cerr << "c --------------------" << std::endl;
		}

		if( removal_heuristic.empty() || removal_heuristic.compare(  (const char*)"activity" ) == 0 ){
			return solve_UP_DEH_RSE_RME_ANA_PRP_CAH_RMH<UPR,DEH,RSE,RME,ANA,PRP,CAH,ActivityRemoval >(  clause_set, var_cnt, commandline, nw);
		}
		if( removal_heuristic.compare(  (const char*)"suffix" ) == 0 ){
			return solve_UP_DEH_RSE_RME_ANA_PRP_CAH_RMH<UPR,DEH,RSE,RME,ANA,PRP,CAH,SuffixRemoval >(  clause_set, var_cnt, commandline, nw);
		}
		else {
#ifdef USE_SOME_COMPONENTS
#ifdef USE_ALL_COMPONENTS
			
#endif
#endif
			std::cerr << "wrong removal heuristic set! (" << removal_heuristic << ")" << std::endl; exit(0);
		}
	}
	
	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH>
	VEC_TYPE( assi_t )* ComponentManager::solve_UP_DEH_RSE_RME_ANA_PRP_CAH_RMH( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw)
	{
		
		Solver s;
		return s.solve<UPR,DEH,RSE,RME,ANA,PRP, CAH, RMH >(  clause_set, var_cnt, commandline, nw);
	}
	
#endif 
