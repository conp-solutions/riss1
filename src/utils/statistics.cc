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


#include "utils/statistics.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"


Statistics& Statistics::inst()
{
  static Statistics instanz;
  return instanz;
}




Statistics::data Statistics::getSnapshot(){
	
	uint64_t stoptime = get_microseconds();
	
	Statistics::data thisData = my_data;
	thisData.currentRuntime = stoptime - my_data.currentRuntime;
	return thisData;
}


void Statistics::setBenchmark( const StringMap& commandline, uint32_t cls_count, uint32_t var1_count ){
	std::string tmp = commandline.get( (const char*)"file" );
	std::cerr << "c extreacted long benchmark name: " << tmp << std::endl;
	if( !tmp.empty() )
	{
		int32_t pos = tmp.size() -1 ;
		for( ; pos >= 0; --pos )
		{
			if(tmp.at(pos) =='/'){ pos++; break;}
		}
		
		benchmark = tmp.substr( pos );
	} else {
		benchmark = "";	
	}
	
	std::cerr << "c extracted benchmark name: " << benchmark << std::endl;
	
	
	this->commandline = commandline.toString();
	int32_t pos = this->commandline.find( "|file", 0 );
	if( pos == (int)std::string::npos )
	{
		pos = this->commandline.find( "file", 0 );
	}
	
	this->commandline = this->commandline.substr( 0, pos );
	
	
	buildtime = 0;
	for( uint32_t i = 0 ; i < strlen( get_builttime() ); ++i ){
		buildtime += get_builttime()[i];
		buildtime *= 10;
	}
	
	cls=cls_count;
	var1s=var1_count;
}


void Statistics::setPreprocessed( uint32_t cls_count, uint32_t var1_count ){
	prep_cls = cls_count;
	prep_var1s = var1_count;
	std::cerr << "c " << benchmark << " preprocessed values: cls: " << cls_count << " var1s: " << var1_count << std::endl;
}

void Statistics::setSatisfiable( bool sat ){
	std::cerr << "c " << benchmark << " is sat: " << sat << std::endl;
	satisfiable = sat;
}


void Statistics::learned_clause( VEC_TYPE(uint32_t)& cls ){
	
	my_data.conflicts ++;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( uint32_t, cls ); ++i )
	{
		CLAUSE& p = gsa.get((CL_REF)cls[i]);
		if( CL_SIZE( p ) == 2 )
		{
			my_data.foundDuals++;
		} else {
			if( CL_SIZE( p ) == 1 ) my_data.foundUnits++;
		}
	}
}

void Statistics::decision( lit_t lit1 ){
	my_data.decisions++;
}


void Statistics::read_lit1(uint32_t pos){
	if( pos <= CLS_LEN )
	{
		my_data.cls_read_access[pos]++;	
	} else {
		my_data.cls_read_access[CLS_LEN+1]++; 
	}
}

void Statistics::write_lit1(uint32_t pos){
	if( pos <= CLS_LEN )
	{
		my_data.cls_write_access[pos]++;	
	} else {
		my_data.cls_write_access[CLS_LEN+1]++; 
	}	
}

void Statistics::swap_lits(uint32_t pos1, uint32_t pos2){
	int row = ( pos1 <=	(CLS_LEN+2) ) ? pos1 : (CLS_LEN+1);
	int col = ( pos2 <=	(CLS_LEN+2) ) ? pos2 : (CLS_LEN+1);
	
	my_data.cls_swap[row * (CLS_LEN+2) + col]++;
}

void Statistics::accessClause( void* cls ){
	uint64_t first = reinterpret_cast<uint64_t>(cls);
	uint64_t second =  reinterpret_cast<uint64_t>(last_access);
	my_data.clause_accesses++;
	my_data.dist_sum += ( first < second ) ? ( 1 ) : ( -1 );	
}

void Statistics::restart(){
	
	char* buf = (char*)MALLOC(1024 * sizeof(char) );
	uint32_t memory = 0;
	snprintf(buf, 1023, "/proc/%u/stat", (unsigned)getpid() );
	FILE* pf = fopen(buf, "r");
	if (pf) {
		if (fscanf(pf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*e %u", &memory) != 1)
		memory = 0;
	}
	std::cerr << "c memory usage (vsize): " << memory / 1024 / 1024 << " MB" << std::endl;
 	fclose(pf);
 	
	
 	if( TRACE_RESTARTS > my_data.currentRestarts ) my_data.memory_per_restart[ my_data.currentRestarts] = memory;
	my_data.currentRestarts++;
	free(buf);
}




int Statistics::startTimer( std::string event ){
	struct time{
		std::string event;
		uint64_t nanoseconds;
	};
	int id = my_data.timer.size();
	timer_data this_event;
	this_event.event = event;
	my_data.timer.push_back( this_event );
	my_data.timer[ id ].nanoseconds = get_microseconds();
	return id;
}

void Statistics::continueTimer( int id ){
	my_data.timer[ id ].last_duration += my_data.timer[ id ].nanoseconds;
	my_data.timer[ id ].nanoseconds = get_microseconds();
}


uint64_t Statistics::stopTimer( int id ){
	assert( my_data.timer.size() > id );
	
	uint64_t stoptime = get_microseconds();
	my_data.timer[ id ].nanoseconds = stoptime - my_data.timer[ id ].nanoseconds + my_data.timer[ id ].last_duration;
	return my_data.timer[ id ].nanoseconds;
}





void Statistics::addDataToDirectory( std::string directory ){
	std::cerr << "c add data to directory" << std::endl;

	uint64_t stoptime = get_microseconds();
	my_data.currentRuntime = stoptime - my_data.currentRuntime;

	if( directory[ directory.size() - 1 ] != '/' ) directory += "/";
	
	std::string filename;
	filename = directory + "solverconfigurations.csv";
	
	std::cerr << "c statistic directory: " << directory << std::endl;
	std::cerr << "c benchmark name: " << benchmark << std::endl;
	
	int solverid = handle_solverconfig( filename );
	
	std::cerr << "c write stats using solver id " << solverid << std::endl;
	std::cerr << "c length of used benchmark: " << benchmark.length() << std::endl;
	if( benchmark.length() == 0 ) return;
	std::cerr << "c benchmark: " << benchmark << std::endl;
	
	filename = directory + "benchmarks.csv";
	handle_benchmark( filename, solverid );
	
	std::cerr << "c write single run statistics" << std::endl;
	
	filename = directory + "search/" + benchmark + ".csv";
	handle_search( filename, solverid );

	filename = directory + "clause/" + benchmark + ".csv";
	handle_clause( filename, solverid );
	
	filename = directory + "timer/" + benchmark + ".csv";
	handle_timer( filename, solverid );
}


void Statistics::writeHeaderToDirectory( std::string directory ){	
	if( directory[ directory.size() - 1 ] != '/' ) directory += "/";
	std::string filename;
	filename = directory + "solverconfigurations.csv";
	std::ofstream file(filename.c_str(), std::ios::app);
	file << "id;builttime;revision;buildfalgs;commandline;" << std::endl;
	file.close();

	filename = directory + "benchmarks.csv";	
	file.open(filename.c_str(), std::ios::app);
	file << "name;clauses;variables;satisfiable;preprocessClauses_preprocessVariables;solver;" << std::endl;
	file.close();

	filename = directory + "search/" + benchmark + ".csv";
	file.open(filename.c_str(), std::ios::app);
	file << "solverid;hostname;time;decisions;conflicts;restarts;" << std::endl;
	file.close();

	filename = directory + "clause/" + benchmark + ".csv";
	file.open(filename.c_str(), std::ios::app);
	file << "solverid;detaillevel;";
	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << "R" << i << ";";
	file << "R>" << CLS_LEN << ";";
	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << "W" << i << ";";
	file << "W>" << CLS_LEN << ";";
	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ){
		for(uint32_t j = 0 ; j <= CLS_LEN; ++j ){
			file << "S" << i << "-" << j << ";";
		}
		file << "S" << i << "->" << CLS_LEN << ";";
	}
	file << "S>" << CLS_LEN << "->" << CLS_LEN << ";";
	
	file.close();
	
	filename = directory + "timer/" + benchmark + ".csv";
	file.open(filename.c_str(), std::ios::app);
	file << "solverid;hostname;number of timer;" << std::endl;
	file.close();
}


int Statistics::handle_solverconfig( const std::string& filename ){
	std::cerr << "c write solver config (benchmark: " << benchmark << ")" << std::endl;
	std::fstream file(filename.c_str() ,  std::ios_base::app | std::ios_base::in | std::ios_base::out);
	file.seekg( 0, std::ios_base::end );
	
	if( file.tellg() < 20 ){
		
		file << "id;builttime;revision;buildfalgs;commandline;" << std::endl;
		file.flush();
	}
	file.seekg(0, std::ios_base::beg );
	
	
	int solverid = 0;	
	std::string line;		
	std::string buildline = (const char*)get_builttime();
	buildline += (const char*)";";
	{
		int r = get_revision();
		char tmp[50];
		int l = 0;
		for( ; r > 0; l++ ) r = r / 10;
		r = get_revision();
		tmp[l] = 0;
		l--;
		for( ; l >= 0; l-- ){
			tmp[l] = r%10 + '0';
			r= r / 10;
		}
		buildline += (const char*)tmp;
	}
	buildline += (const char*)";";
	buildline += (const char*)getBuildFlagList().c_str();
	buildline += (const char*)";";
	buildline += (const char*)commandline.c_str();
	buildline += (const char*)";";
	
	bool found = false;
	std::cerr << "c look whether solver has been used previously" << std::endl;
	while( getline (file, line) ){
		solverid ++;
		std::string::size_type pos = line.find( ";", 0 );
		
		if( line.substr(pos+1).compare( buildline ) == 0 )
		{
			found = true;
			solverid--;
			break;
		}
	}

	
	if( ! found ){
		file.clear();
		file << solverid << ";" << buildline << std::endl;
	}
	file.close();
	return solverid;
}

int Statistics::handle_benchmark( const std::string& filename, int solverid ){
	std::fstream file(filename.c_str() ,  std::ios_base::app | std::ios_base::in | std::ios_base::out);
	file.seekg( 0, std::ios_base::end );
	
	if( file.tellg() < 20 ){
		
		file << "name;clauses;variables;satisfiable;preprocessClauses;preprocessVariables;solver;" << std::endl;
		file.flush();
	}
	file.seekg(0, std::ios_base::beg );
	
	std::stringstream benchmarkline;
	benchmarkline << benchmark << ";" << cls << ";" << var1s << ";" << satisfiable << ";" << prep_cls << ";" << prep_var1s << ";";
	
	bool found = false;
	std::string line;
	while( getline (file, line) ){
		
		if( line.find( benchmarkline.str() ) != std::string::npos ) { found = true; break; }
	}
	
	if( ! found ){
		file.clear();
		file << benchmarkline.str() << solverid << ";" << std::endl;
	}
	file.close();
	return found;
}

int Statistics::handle_search( const std::string& filename, int solverid ){
	std::fstream file(filename.c_str() ,  std::ios_base::app | std::ios_base::in | std::ios_base::out);
	file.seekg( 0, std::ios_base::end );
	
	if( file.tellg() < 20 ){
		
		file << "solverid;hostname;time;decisions;conflicts;restarts;" << std::endl;
		file.flush();
	}
	file.seekg(0, std::ios_base::beg );
	
	file << solverid << ";" << get_hostname() << ";" << my_data.currentRuntime << ";"
			 << my_data.decisions << ";" << my_data.conflicts << ";" << my_data.currentRestarts << ";" << std::endl;

	return 0;
}

int Statistics::handle_timer( const std::string& filename, int solverid ){
	std::fstream file(filename.c_str() ,  std::ios_base::app | std::ios_base::in | std::ios_base::out);
	file.seekg( 0, std::ios_base::end );
	
	if( file.tellg() < 20 ){
		
		file << "solverid;hostname;number of timer;" << std::endl;
		file.flush();
	}
	file.seekg(0, std::ios_base::beg );
	
	file << solverid << ";" << get_hostname() << ";" << my_data.timer.size() << ";";
	for( uint32_t i = 0 ; i < my_data.timer.size(); ++i )
	{
		file << my_data.timer[i].event << ";" << my_data.timer[i].nanoseconds << ";";
	}
	file << std::endl;
	
	return 0;
}

int Statistics::handle_clause( const std::string& filename, int solverid ){
	std::fstream file(filename.c_str() ,  std::ios_base::app | std::ios_base::in | std::ios_base::out);
	file.seekg( 0, std::ios_base::end );
	
	if( file.tellg() < 20 ){
		
		file << "solverid;detaillevel;accesses;";
		
		for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << "R" << i << ";";
		file << "R>" << CLS_LEN << ";";
		for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << "W" << i << ";";
		file << "W>" << CLS_LEN << ";";
		for(uint32_t i = 0 ; i <= CLS_LEN; ++i ){
			for(uint32_t j = 0 ; j <= CLS_LEN; ++j ){
				file << "S" << i << "-" << j << ";";
			}
			file << "S" << i << "->" << CLS_LEN << ";";
		}
		file << "S>" << CLS_LEN << "->" << CLS_LEN << ";";
		file << std::endl;
		file.flush();
	}
	file.seekg(0, std::ios_base::beg );

	file << solverid << ";" << CLS_LEN << ";" << my_data.clause_accesses << ";";

	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << my_data.cls_read_access[i] << ";";
	file << my_data.cls_read_access[CLS_LEN+1] << ";";
	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ) file << my_data.cls_write_access[i] << ";";
	file << my_data.cls_write_access[CLS_LEN+1] << ";";

	for(uint32_t i = 0 ; i <= CLS_LEN; ++i ){
		for(uint32_t j = 0 ; j <= CLS_LEN; ++j ){
			file << my_data.cls_swap[ i*(CLS_LEN+2) + j]<< ";";
		}
		file << my_data.cls_swap[ i*(CLS_LEN+2) + CLS_LEN + 1] << ";";
	}
	file << my_data.cls_swap[ (CLS_LEN+1)*(CLS_LEN+2) + CLS_LEN + 1]<< ";";

	file << std::endl;
	return 0;
}
