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



#include "utils/commandlineparser.h"

StringMap CommandLineParser::parse(int32_t argc, char* argv[], bool binary)
{
	map.clear();

	
	
	map.insert(  (char*)"restart_event", (char*)"geometric");
	map.insert(  (char*)"restart_event_geometric_max", (char*)"150");
	map.insert(  (char*)"restart_event_geometric_nested", (char*)"1");
	map.insert(  (char*)"restart_event_geometric_inc", (char*)"1.3");
	map.insert(  (char*)"rem_keep_last", (char*)"1");
	map.insert(  (char*)"CP_er", (char*)"1" );

	int32_t scan_begin = 0;
	if( binary){
		
		map.insert( (char*)"binary", argv[0] );
		scan_begin++;
	}
	
	
	if( scan_begin == 1 && argc > 1 )
	{
		if( argv[1][0] != '-' )
		{
			map.insert(  (char*)"file", argv[1] );
			scan_begin = 2;
		}
	}
	
	
	if( scan_begin == 2 && argc > 2 ){
		if( argv[2][0] != '-' )
		{
			map.insert(  (char*)"outfile", argv[2] );
			scan_begin = 3;
		}
	}

	
	for( int32_t i = scan_begin; i < argc; ++i )
	{
		if( strlen( argv[i] ) == 1 )
		{
			map.insert( argv[i], 0 );	
			continue;
		}
		
		if( argv[i][0] == '-' && argv[i][1] == 'P' )
		{
			if( i == argc - 1 )
			{
				std::cerr << "syntax error in last command line parameter. parameter value is missing." << std::endl;
				map.insert(  &(argv[i][2]), 0 );
				continue;
			}
			map.insert(  &(argv[i][2]), argv[i+1] );
			i++;
			continue;
		}
		
		map.insert(  argv[i], 0 );
	}
	
	
	if( map.contains( (const char*)"configfile" ) || map.contains( (const char*)"cf" ))
	{
		parseFile( map.get( (const char*)"configfile" ), map );
	}
	
	
	if( map.contains( (const char*)"-h" ) || map.contains((const char*)"--help" ) ){
#ifdef CSPSOLVER
		print_csp_help();
#else
		print_sat_help();
#endif
		if( !map.contains((const char*)"--help" ) ) exit( -1 );
	}


	if( map.contains((const char*)"conftime" ) ){
		
		std::cerr << "c setup for " << map.get((const char*)"conftime") << "s" << std::endl;
		uint32_t seconds = atoi( map.get((const char*)"conftime").c_str() );
		if( seconds < 1250 ){
			map.insert(  (char*)"cdcl_probing_mode", (char*)"1" );
		}
	}



	if( map.contains((const char*)"confcores" ) ){
		
		std::cerr << "c setup for " << map.get((const char*)"confcores") << "cores" << std::endl;
#ifdef PARALLEL
		

		
		map.insert( "NW_recv0", "1" );

#else
		
#endif
	}



#ifdef COMPETITION
	map.remove( "-h" );
	map.remove( "--help" );
	map.remove( "-v" );
	map.remove( "-i" );
#endif


#ifdef TESTBINARY
	map.remove( "-v" );
	map.remove( "-i" );
#endif
	
	
	if(  map.contains( (const char*)"-v" ) )
	{
		std::cerr << "c commandline parameter:" << std::endl;
		for( uint32_t i = 0 ; i < map.size(); ++i )
		{
			std::cerr << "c " << map[i].key << " -- ";
			if( map[i].value.size() > 0 ) std::cerr << map[i].value;
			std::cerr << std::endl;
		}
	}
	
	return map;
}


bool CommandLineParser::parseFile( string filename, StringMap& thisMap ){
	
	std::vector<std::string> fileargv;
	std::ifstream file;

	
	file.open(  filename.c_str() , std::ios_base::in);
	if( !file ){
		std::cerr << "c can not open configfile " << filename << std::endl;
		return false;
	}
	std::string line;
	while(getline (file, line)){
		fileargv.push_back(line);
	}
	
	for( uint32_t i = 0; i < fileargv.size(); ++i ){
		if( fileargv[i].size() == 1 ){
			thisMap.insert( (char*)fileargv[i].c_str(), 0 );	
			continue;
		}
		
		if( fileargv[i][0] == '-' && fileargv[i][1] == 'P' ){
			if( i == fileargv.size() - 1 ){
				std::cerr << "syntax error in last config file parameter. parameter value is missing." << std::endl;

				continue;
			}
			thisMap.insert(  (char*)&(fileargv[i][2]), (char*)fileargv[i+1].c_str() );
			i++;
			continue;
		}
		
		thisMap.insert(  (char*)fileargv[i].c_str(), 0 );
	}
	return true;
}

void CommandLineParser::print_csp_help()
{
		std::string binary = map.get((const char*)"binary" );
		std::cerr << "usage:" << std::endl;
		std::cerr << binary << " [File] [Outfile] -PName Value" << std::endl;
		std::cerr << binary << " -Pfile Filename -Poutfile Filename -PName Value" << std::endl;
		std::cerr << binary << " -Pfile Filename -Pconfigfile ConfigFile" << std::endl << std::endl;
		std::cerr << "\t\tName ... name of the parameter." << std::endl;
		std::cerr << "\t\tValue ... value of the according parameter." << std::endl << std::endl;
		std::cerr << "\t\tcf ,configfile ... txt file where further arguments are listed(one per line)" << std::endl << std::endl;
		std::cerr << "\t\t-h     ... shows this info." << std::endl ;
		std::cerr << "\t\t--help ... shows long help" << std::endl ;
		std::cerr << "\t\t-q     ... stops outputing the found satisfying assignment." << std::endl;
		std::cerr << "\t\t-Pconftime t ... setup a good configuration for runtime t (seconds)" << std::endl;
		std::cerr << "\t\t-Pconfcores c ... setup a good configuration for cores c" << std::endl;
#ifndef TESTBINARY
#ifndef COMPETITION
		std::cerr << "\t\t-v  ... shows more detailed info." << std::endl; 
		std::cerr << "\t\t-i  ... shows information about the build process." << std::endl;
#endif
#endif
}

void CommandLineParser::print_sat_help()
{
		std::string binary = map.get((const char*)"binary" );
		std::cerr << "usage:" << std::endl;
		std::cerr << binary << " [File] [Outfile] -PName Value" << std::endl;
		std::cerr << binary << " -Pfile Filename -Poutfile Filename -PName Value" << std::endl;
		std::cerr << binary << " -Pfile Filename -Pconfigfile ConfigFile" << std::endl << std::endl;
		std::cerr << "examples:" << std::endl;
		std::cerr << binary << " sat.cnf solution.cnf" << std::endl;
		std::cerr << binary << " -Prestart_event dynamic -Pfile sat.cnf -Poutfile solution.cnf" << std::endl << std::endl;
		std::cerr << "explanation:" << std::endl;
		std::cerr << "\t\tName ... name of the parameter." << std::endl;
		std::cerr << "\t\tValue ... value of the according parameter." << std::endl << std::endl;
		std::cerr << "\t\tcf ,configfile ... txt file where further arguments are listed(one per line)" << std::endl << std::endl;
		std::cerr << "\t\t-h     ... shows this info." << std::endl ;
		std::cerr << "\t\t--help ... shows long help" << std::endl ;
		std::cerr << "\t\t-q     ... stops outputing the found satisfying assignment." << std::endl;
		std::cerr << "\t\t-sol   ... number of solutions to find (-1 = all)." << std::endl;
#ifndef TESTBINARY
#ifndef COMPETITION
		std::cerr << "\t\t-v  ... shows more detailed info." << std::endl; 
		std::cerr << "\t\t-i  ... shows information about the build process." << std::endl;
		std::cerr << "\t\t-analyze_formula ... dumps formula analysation into formula.csv" << std::endl;
		if( map.contains((const char*)"--help" ) ){
			std::cerr << std::endl;
			std::cerr << std::endl;
			std::cerr << "COMPILE FLAGS:" << std::endl;
			std::cerr << "parameter:" << std::endl;
			std::cerr << "  USE_ALL_COMPONENTS ... enables component switching after compiling(huge binary!)" << std::endl;
			std::cerr << "  USE_SOME_COMPONENTS ... enables some components" << std::endl;
			std::cerr << "  USE_COMMANDLINEPARAMETER ... enables parameter setting via commandline" << std::endl;
			std::cerr << "structures:" << std::endl;
			std::cerr << "  USE_C_XYZ with XYZ { VECTOR,STACK,RINGBUFFER } ... structure type" << std::endl;
			std::cerr << "  USE_C_HEAP, USE_CPP_HEAP ... heap as struct or object" << std::endl;
			std::cerr << "  or combined USE_C_STRUCTURES, USE_STL_STRUCTURES ... sets all structures" << std::endl;
			std::cerr << "  EXPERIMENTAL_SIZE_ASSIGNMENT ( see defines.h for detail! )" << std::endl;
			std::cerr << "  EXPERIMENTAL_PACK_ASSIGNMENT ... compress assignment representation" << std::endl;
			std::cerr << "  EXPERIMENTAL_BOOLARRAY ... store bool as single bits" << std::endl;
			std::cerr << "  USE_C_CLAUSE,USE_CPP_CLAUSE,USE_C_CACHE_CLAUSE,USE_CPP_CACHE_CLAUSE ... choose clause type" << std::endl;
			std::cerr << "  further clause types: USE_CPP_SLAB_CLAUSE, USE_CPP_CACHE_SLAB_CLAUSE" << std::endl;
			std::cerr << "implementation changes:" << std::endl;
			std::cerr << "  USE_PREFETCHING ... enables the prefetch function" << std::endl;
			std::cerr << "     PREFETCHINGMETHOD1 ... prefetch just before propagating" << std::endl;
			std::cerr << "     PREFETCHINGMETHOD1 ... prefetch between enqueing and propagating" << std::endl;
			std::cerr << "  COMPRESS_CLAUSE ... packs the clause structure" << std::endl;
			std::cerr << "  COMPRESS_WATCH_STRUCTS ... packs watch list structures" << std::endl;
			std::cerr << "  ANALYZER_VEC_MEMBER ... creates the vectors for analysation only once" << std::endl;
			std::cerr << std::endl;
		}
#endif
#endif
}
