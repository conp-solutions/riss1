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


#ifndef NETWORK_H_
#define NETWORK_H_

#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include <sstream>
#include <vector>
#include <string>

#include "defines.h"

#include "types.h"
#include "macros/malloc_macros.h"
#include "macros/clause_macros.h"

#include "utils/commandlineparser.h"
#include "utils/stringmap.h"
#include "utils/lock.hh"

#include "sat/searchdata.h"

using namespace std;



void* threadSolve(void *data);


class ComponentManager;



#define MAX_THREADS 8


class Network{
public:
	struct lEle {
		uint32_t elements;
		pthread_t author;
		uint32_t type;	
		uint32_t eleId;
		lEle* next;
		union data_t {
			CL_REF clauses[0];
			lit_t literals[0];
		} data;

	};

	
	struct threadInfo {
		pthread_t thread;
		lEle* lastProcessed;	
		uint32_t workingAt;	
		uint32_t sendCls;		
		uint32_t recvCls;		
		VEC_TYPE( assi_t )* solutions; 
		VEC_TYPE( CL_REF )* clause_set;
		uint32_t var_cnt;
		Network* nw;
		uint32_t finished;	
		StringMap cmd;	
		uint32_t canPrint;
		char fill [128 - sizeof(pthread_t) - sizeof(VEC_TYPE( assi_t )*)
		           -sizeof(assi_t) - sizeof( VEC_TYPE( CL_REF )* )
		           - sizeof( StringMap ) - sizeof(Network*)
		           - sizeof(lEle*) - 5*sizeof(uint32_t)];	

		threadInfo()
		: thread(0), lastProcessed(0), workingAt(0), sendCls(0), recvCls(0),solutions(0), finished(0), canPrint(0)
		{}
	};

private:
	
	threadInfo info [MAX_THREADS];
	lEle* first;	
	lEle* last;		
	uint32_t listElements;	
	uint32_t printedSummary;	

	uint64_t sendMask;	
	uint64_t recvMask;	

	
	CONST_PARAM uint32_t threads;	
	CONST_PARAM bool communication; 
	CONST_PARAM uint32_t debug;		
	
	CONST_PARAM uint32_t recvSizeThreshold;	
	CONST_PARAM bool recvRejectSat; 
	CONST_PARAM uint32_t dynRecvMode; 
	
	CONST_PARAM float actSendThresh; 
	CONST_PARAM uint32_t sendSize;	
	CONST_PARAM uint32_t dynSendMode; 
	
	
	vector< string > threadFiles;	

	
	Lock sleepLock;
	volatile bool runningAgain; 

	
	pthread_mutex_t commLock;

	
	void fillParameter(StringMap& cmd, uint32_t threadNumber);


	
	bool keepClause( const CL_REF clause, const searchData& search );

	
	bool doSendClause( const CL_REF clause, const searchData& search );

	
	void cleanClauses();

public:

	Network(const StringMap & commandline);
	~Network();

	
	VEC_TYPE( assi_t )* solve(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap& cmd, ComponentManager& cm);

	
	void addClauses( const VEC_TYPE(CL_REF)& cls, searchData& search);

	
	void addUnits( const VEC_TYPE(lit_t)& literals, searchData& search);

	
	void giveClauses( VEC_TYPE(CL_REF)& toFill, searchData& search);

	
	void wakeUp();

	
	bool doPrint();

	void set_parameter (const StringMap & cmd);
};

#include "sat/component.h"

inline Network::Network(const StringMap & commandline) :
first(0),last(0),
listElements(0),
printedSummary(0),
#ifdef PARALLEL
threads(4),
#else
threads(1),
#endif
communication(true),
debug(0),
recvSizeThreshold(0), 
recvRejectSat(false), 
dynRecvMode(2),	
actSendThresh(0.0),	
sendSize(0),	
dynSendMode(1),	
sleepLock(0), 
runningAgain(false)
{
	pthread_mutex_init(&commLock, 0);

	
	sendMask = ~(uint64_t)0;
	
	recvMask = ~(uint64_t)1;

	set_parameter(commandline);
}

inline Network::~Network(){
	pthread_mutex_destroy(&commLock);
}

inline void Network::cleanClauses(){
	
	if( first  == 0 ) return;
	
	uint32_t maxMinId = info[0].workingAt;
	
	for( uint32_t t = 0 ; t < threads; t++ ){
		
		maxMinId = ( maxMinId > info[t].workingAt && ( recvMask & ((uint64_t)1 << t) ) != 0 )? info[t].workingAt : maxMinId;
	}
	
	
	if( maxMinId == 0 ) return;	

	uint32_t count = 0 ;	
	
	lEle* element = first;
	while( element->next != 0 && element->eleId  + 1 < maxMinId){
		lEle* tmp = element->next;
		
		if( element->type ==  0 ){
			for( uint32_t i = 0 ; i < element->elements; ++i ){
				CL_DESTROY( gsa.get(element->data.clauses[i]) );
				gsa.release( element->data.clauses[i] );
			}
		}
		free(element);
		element = tmp;
		count ++;
	}
	
	first = element;
	
}

inline VEC_TYPE( assi_t )* Network::solve(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap& cmd, ComponentManager& cm){
	
	if( debug > 1 ) cerr << "c start solving with " << threads << " threads" << endl;
	if( threads == 1 ){
		return cm.solve(clause_set,var_cnt,cmd, *this);
	}
	if( debug > 1 ){
		stringstream s;
		s << "c NW start network solving using " << threads << " threads" << endl;
		cerr << s.str();
	}
	const uint32_t cls_cnt = VEC_SIZE(CL_REF, (*clause_set) );
	for( uint32_t i = 0; i < threads; ++i ){
		if( debug > 1 ){
			stringstream s;
			s << "c NW enter create loop " << i << endl;
			cerr << s.str();
		}
		
		info[i].clause_set = new VEC_TYPE( CL_REF );
		VEC_RESIZE( CL_REF, (*info[i].clause_set), cls_cnt, 0 );
		for(uint32_t j = 0 ; j < cls_cnt; ++j ){
			(*info[i].clause_set)[j] = gsa.create( CL_COPY( gsa.get( (*clause_set)[j])) );
		}

		
		info[i].cmd = cmd;
		fillParameter(info[i].cmd, i);

		
		info[i].var_cnt = var_cnt;
		info[i].nw = this;

		
		if( debug > 1 ){
			stringstream s1;
			s1 << "c NW create thread" << endl;
			cerr << s1.str();
		}
		int rc = pthread_create(&(info[i].thread), 0, threadSolve, (void *)&(info[i]));
		if (rc){
			 printf("ERROR; return code from pthread_create() is %d\n", rc);
		} else {
			stringstream s;
			s << "c NW started thread " << i << " successfully (" << info[i].thread << ")" << endl;
			cerr << s.str();
		}
		
	}
	
	

	
	sleepLock.wait();
	
	runningAgain = true; 
	

	if( debug > 1 ){
		stringstream s2;
		s2 << "c NW parent wakes up again" << endl;
		cerr << s2.str();
	}

	
	for( uint32_t i = 0 ; i < threads; i++ ){
		int err = 1;
		do {
			err = pthread_cancel(info[i].thread);
			if( err == ESRCH ) break;
			if( err != 0 ){	
				cerr << "c cancel thread " << i << " failed with " << err << endl;
			}
		} while( err != 0 );
		
		if( err != 0 ){
			if( debug > 1 ){
				stringstream s;
				s << "c NW network was not able to kill thread " << info[i].thread << endl;
				cerr << s.str();
			}
		} else {
			if( debug > 1 ){
				stringstream s;
				s << "c NW network killed thread " << info[i].thread << endl;
				cerr << s.str();
			}
		}
	}

	cerr         << "c sended clauses  ";
	for( uint32_t j = 0 ; j < threads; ++j ){
		cerr << " | " << info[j].sendCls;
	}
	cerr << " |" << endl << "c received clauses";
	for( uint32_t j = 0 ; j < threads; ++j ){
		cerr << " | " << info[j].recvCls;
	}			
	cerr << " |" <<  endl;

	cerr << "c join all other threads" << endl;
	
	for( uint32_t i = 0 ; i < threads; i++ ){
		int* status = 0;
		int err = 0;

		err = pthread_join(info[i].thread, (void**)&status);
		if( err == EINVAL ) cerr << "c tried to cancel wrong thread" << endl;
		if( err == ESRCH ) cerr << "c specified thread does not exist" << endl;
		cerr << "c joined thread " << i << endl;
	}

	
	for( uint32_t i = 0; i < threads; ++i ){
		if( info[i].finished == 1 ){
			if( debug > 1 ){
				stringstream s;
				s << "c NW return thread " << i << " with address of solutions" << hex << info[i].solutions << dec << endl;
				cerr << s.str();
			}
			return info[i].solutions;
		}
	}

	
	return 0;
}

inline void Network::addClauses( const VEC_TYPE(CL_REF)& cls, searchData& search){
#ifdef PARALLEL
	if( !communication || threads == 1 || VEC_SIZE(CL_REF, cls) == 0) return;
	
	const pthread_t me = pthread_self ();
	
	
	uint32_t threadnr = 0;
	for( ; threadnr < threads; ++threadnr ){
		if( pthread_equal(me, info[threadnr].thread ) ){
			break;
		}
	}
	
	if( ( sendMask & ((uint64_t)1 << threadnr) ) == 0 ) return;
	
	uint32_t count = 0;
	CL_REF sendMe [VEC_SIZE(CL_REF, cls )];
	
	
	for( uint32_t i = 0 ; i < VEC_SIZE(CL_REF, cls ); ++i ){
		
		if( doSendClause( cls[i], search ) ){
			sendMe[count++] = gsa.create( CL_COPY( gsa.get(cls[i]) ) );
		}
	}	

	
	
	
	if( count == 0 ) return;
	
	lEle* next = (lEle*) malloc( sizeof(lEle) + sizeof(CL_REF) * count  );
	next->author = me;
	next->type = 0;	
	next->elements = count;	
	next->next = 0;
	
	memcpy( next->data.clauses, sendMe, sizeof(CL_REF) * count);

	info[threadnr].sendCls += VEC_SIZE(CL_REF, cls );

	
	pthread_mutex_lock(&commLock);
	
	next->eleId = listElements++;
	if( first == 0 ) first = next;
	if( last != 0 ) last->next = next;
	last = next;
	pthread_mutex_unlock(&commLock);
#endif
}

inline void Network::addUnits( const VEC_TYPE(lit_t)& literals, searchData& search){
#ifdef PARALLEL
	if( !communication || threads == 1 || VEC_SIZE(CL_REF, literals) == 0) return;

	
	const pthread_t me = pthread_self ();
	uint32_t threadnr = 0;
	for( ; threadnr < threads; ++threadnr ){
		if( pthread_equal(me, info[threadnr].thread ) ){
			break;
		}
	}	
	
	if( ( sendMask & ((uint64_t)1 << threadnr) ) == 0 ) return;

	info[threadnr].sendCls += VEC_SIZE(lit_t, literals );
	
	
	
	const uint32_t count = VEC_SIZE(lit_t, literals);
	lEle* next = (lEle*) malloc( sizeof(lEle) + sizeof(lit_t) * count  );
	next->author = me;
	next->type = 1;	
	next->elements = count;	
	next->next = 0;
	
	memcpy( next->data.literals, &(literals[0]), sizeof(lit_t) * count);

	
	pthread_mutex_lock(&commLock);
	
	next->eleId = listElements++;
	if( first == 0 ) first = next;
	if( last != 0 ) last->next = next;
	last = next;
	pthread_mutex_unlock(&commLock);
#endif
}



inline void Network::giveClauses( VEC_TYPE(CL_REF)& toFill, searchData& search ){
#ifdef PARALLEL
	if( !communication || threads == 1 ) return;

	uint32_t oldSize = VEC_SIZE(CL_REF, toFill);
	
	uint32_t i = 0;
	const pthread_t me = pthread_self ();
	for(; i < threads; ++i ){
		if( pthread_equal(me, info[i].thread ) ) break;
	}

	
	if( i == 0 ){
		
		cleanClauses();
	}

	
	if( ( recvMask & ((uint64_t)1 << i) ) == 0 ) return;

	lEle*& ele = info[i].lastProcessed;
	if( last == 0 || first == 0 ){
		return;	
	}

	uint32_t workingAt = 0;

	while ( ele == 0 || ele->next != 0 ){
		
		ele = (ele == 0 ) ? first : ele->next;
		workingAt = ( ele == 0 ) ? 0 : ele->eleId;

		
		if( pthread_equal(me, ele->author) ) continue;
		
		if( ele->type == 0 ) VEC_PUSH_BACK_ANOTHER( CL_REF, toFill, (ele->data).clauses, ele->elements );
		
		if( ele->type == 1 ){
			for( uint32_t i = 0 ; i < ele->elements; i++ ){
				
				if( search.VAR_LEVEL( var( (ele->data).literals[i] ) ) != 0 ){
					VEC_PUSH_BACK( CL_REF, toFill, gsa.create( CL_CREATE(&((ele->data).literals[i]), 1 ) ) );
				}
			}
		}
	}
	
	
	if( VEC_SIZE(CL_REF, toFill) - oldSize != 0 ){
		if( debug > 1 ){
			stringstream s;
			s << "c NW [" << me << "] got " << VEC_SIZE(CL_REF, toFill) - oldSize << " clauses" << endl;
			cerr << s.str();
		}
		
		for( uint32_t j = oldSize ; j < VEC_SIZE(CL_REF, toFill); j++ ){
			if( !keepClause( toFill[j], search ) ){
				VEC_ERASE_NO_ORDER( CL_REF, toFill, j );
				j--;
			} else {
				
				toFill[j] = gsa.create( CL_COPY( gsa.get(toFill[j]) ) );
			}
		}
		info[i].recvCls += ( VEC_SIZE(CL_REF, toFill) - oldSize);
	}
	
	
	info[i].workingAt = workingAt;
#endif
}



inline bool Network::doSendClause( const CL_REF clause, const searchData& sd ){
#ifdef PARALLEL
	CLAUSE& c = gsa.get(clause);
	CLAUSE& cl = gsa.get( clause );
	const uint32_t size = CL_SIZE(cl);
	
	
	
	if( dynSendMode == 1 && size > sd.currentLearntMin ) return false;
	
	if( dynSendMode == 2 && sd.currentLearnts > 0 && size > sd.currentLearntAvg / sd.currentLearnts ) return false;
	
	const float act = CL_GET_ACTIVITY(cl);
	
	for( uint32_t i = 0 ; i < size; ++i ){
		if( var(CL_GET_LIT(cl,i) ) > sd.original_var_cnt ) return false;
	}

	
	
	if( actSendThresh > 0 && act < actSendThresh ) return false;
	
	if( sendSize > 0 && size > sendSize ) return false;
	return true;
#else
	return false;
#endif
}

inline bool Network::keepClause( const CL_REF clause, const searchData& sd ){
	
	uint32_t ud = 0,us=0;	
	
	
	const CLAUSE& cl = gsa.get( clause );
	
	
	if( CL_SIZE(cl) == 1 ){
		if( sd.VAR_LEVEL(var(CL_GET_LIT(cl,0))) == 0 ) return false;
		else return true;
	}

	const uint32_t size = CL_SIZE( gsa.get( clause ) );
	
	
	if( dynRecvMode == 1 && size > sd.currentLearntMin ) return false;
	
	if( dynRecvMode == 2 && sd.currentLearnts > 0 && size > sd.currentLearntAvg / sd.currentLearnts ) return false;
	
	
	for( uint32_t j = 0; j < size; ++j ){
		const lit_t literal = CL_GET_LIT( gsa.get( clause ) ,j);
		
		if( boolarray_get( sd.eliminated, var(literal) ) ) return false;
		if( var(CL_GET_LIT(cl,j) ) > sd.original_var_cnt ) return false;
		if( assi_is_undef( sd.assi, var(literal) ) ) ud ++; 
		else if( assi_is_unsat( sd.assi, literal ) ){
			us ++;
		}
	}
	
	
	
	if( us == size || (ud == 1 && us + 1 >= size)) return true;
	
	if( size > recvSizeThreshold ) return false;
	
	if( recvRejectSat && ud + us < size ) return false;
	return true;
}

inline void Network::wakeUp(){
	volatile bool readBool = runningAgain;
	while( !readBool ){
		readBool = runningAgain;
		sleepLock.unlock();
	}
}

inline void Network::fillParameter(StringMap& cmd, uint32_t threadNumber){

	if( threadFiles[threadNumber] != "" ){
		CommandLineParser clp;
		clp.parseFile( threadFiles[threadNumber], cmd );
	
	} else {
		if( threadNumber == 0 ){
			
		} else if( threadNumber == 1 ){
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
		} else if( threadNumber == 2 ){
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
			cmd.insert(  (char*)"var_act_vmtf", (char*)"1" );
		} else if( threadNumber == 3 ){
			
			cmd.insert(  (char*)"CP_erMaxV", (char*)"50000" );
			cmd.insert(  (char*)"CP_erMinV", (char*)"30000" );
			cmd.insert(  (char*)"CP_erPairs", (char*)"400" );
			cmd.insert(  (char*)"CP_erIters", (char*)"5" );
			cmd.insert(  (char*)"CP_erMinO", (char*)"2" );
			cmd.insert(  (char*)"CP_erTries", (char*)"1" );
			
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
			cmd.insert(  (char*)"propagation", (char*)"Pwatch" );
			cmd.insert(  (char*)"cdcl_simp", (char*)"1" );
		} else if( threadNumber == 4 ){
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
			cmd.insert(  (char*)"restart_event", (char*)"dynamic" );
			cmd.insert(  (char*)"up_search_sat", (char*)"5" );
			cmd.insert(  (char*)"cdcl_simp", (char*)"1" );
			cmd.insert(  (char*)"CP_See", (char*)"1" );
		} else if( threadNumber == 5 ){
			cmd.insert(  (char*)"restart_event_geometric_nested", (char*)"1" );
			cmd.insert(  (char*)"restart_event_geometric_max", (char*)"100" );
			cmd.insert(  (char*)"restart_event_geometric_inc", (char*)"1.1" );
		} else if( threadNumber == 6 ){
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
			cmd.insert(  (char*)"dup_long_conflicts", (char*)"1" );
			cmd.insert(  (char*)"dup_lastBinCon", (char*)"1" );
			cmd.insert(  (char*)"cdcl_simp", (char*)"1" );
		} else if( threadNumber == 7 ){
			cmd.insert(  (char*)"cdcl_probing_mode", (char*)"1" );
			cmd.insert(  (char*)"up_search_sat", (char*)"5" );
			cmd.insert(  (char*)"cdcl_simp", (char*)"1" );
			cmd.insert(  (char*)"CP_See", (char*)"1" );
		} else if( threadNumber == 8 ){
			cmd.insert(  (char*)"rem_keep_last", (char*)"0");
			cmd.insert(  (char*)"dup_long_conflicts", (char*)"1" );
			cmd.insert(  (char*)"dup_lastBinCon", (char*)"1" );
		}
	
		
	}
}

inline bool Network::doPrint(){

	
	uint32_t i = 0;
	const pthread_t me = pthread_self ();
	for(; i < threads; ++i ){
		if( pthread_equal(me, info[i].thread ) ) break;
	}

	
	if( info[i].canPrint == 1 ) return true;

	bool doPrint = false;
	
	pthread_mutex_lock(&commLock);
		if( printedSummary == 0 ){
			doPrint = true;
			printedSummary = 1;
			
			info[i].canPrint = 1;
		}
	pthread_mutex_unlock(&commLock);
	return doPrint;
}

inline void Network::set_parameter (const StringMap & cmd){
#ifdef USE_COMMANDLINEPARAMETER

#ifdef PARALLEL	
	if( cmd.contains( (const char*)"NW_threads" ) ) threads = atoi( cmd.get( (const char*)"NW_threads" ).c_str() );
	if( cmd.contains( (const char*)"NW_comm" ) )  communication = 0 != atoi( cmd.get( (const char*)"NW_comm" ).c_str() );
	if( cmd.contains( (const char*)"NW_debug" ) ) debug = atoi( cmd.get( (const char*)"NW_debug" ).c_str() );
	
	if( cmd.contains( (const char*)"NW_rMaxSize" ) ) recvSizeThreshold = atoi( cmd.get( (const char*)"NW_rMaxSize" ).c_str() );
	if( cmd.contains( (const char*)"NW_rNoSat" ) ) recvRejectSat = 0 != atoi( cmd.get( (const char*)"NW_rNoSat" ).c_str() );
	if( cmd.contains( (const char*)"NW_rMode" ) ) dynRecvMode = atoi( cmd.get( (const char*)"NW_rMode" ).c_str() );

	if( cmd.contains( (const char*)"NW_sMinAct" ) ) actSendThresh = atoi( cmd.get( (const char*)"NW_sMinAct" ).c_str() );
	if( cmd.contains( (const char*)"NW_sMaxSize" ) ) sendSize = atof( cmd.get( (const char*)"NW_rMode" ).c_str() );
	if( cmd.contains( (const char*)"NW_sMode" ) ) dynSendMode = atoi( cmd.get( (const char*)"NW_sMode" ).c_str() );
#endif

	if( threads > MAX_THREADS) threads = MAX_THREADS;

	threadFiles.resize(threads);
#ifdef PARALLEL
	
	for( uint32_t i = 0 ; i < threads; ++i ){
		stringstream s;
		s << "NW_config" << i;		
		if( cmd.contains( (const char*)s.str().c_str() ) ) threadFiles[i] = cmd.get( (const char*)s.str().c_str() );
	}
	for( uint32_t i = 0 ; i < threads; ++i ){
		stringstream s;
		s << "NW_send" << i;		
		if( cmd.contains( (const char*)s.str().c_str() ) ){
			uint64_t bit = atoi( cmd.get( (const char*)s.str().c_str() ).c_str() );
			if( bit == 0 ){
				sendMask = sendMask & ~((uint64_t)1 << i);
			} else {
				sendMask = sendMask | ((uint64_t)1 << i);
			}
		}
	}
	for( uint32_t i = 0 ; i < threads; ++i ){
		stringstream s;
		s << "NW_recv" << i;		
		if( cmd.contains( (const char*)s.str().c_str() ) ){
			uint64_t bit = atoi( cmd.get( (const char*)s.str().c_str() ).c_str() );
			if( bit == 0 ){
				recvMask = recvMask & ~((uint64_t)1 << i);
			} else {
				recvMask = recvMask | ((uint64_t)1 << i);
			}
		}
	}		
#endif

#endif
#ifndef TESTBINARY
#ifndef COMPETITION
	if( cmd.contains( (const char*)"-h" ) || cmd.contains( (const char*)"--help" ) )
	{
		cerr << "=== Network information ===" << endl;
		cerr << " parameter  values  info" << endl;
		cerr << " NW_threads 1-" << MAX_THREADS << "    number of threads that can be used" << endl;
		cerr << " NW_comm    0,1     do communication" << endl;
		cerr << " NW_debug   0-n     level of debug output" << endl;
		cerr << " NW_sendX   0,1     allow/disallow config X to send clauses" << endl;
		cerr << " NW_recvX   0,1     allow/disallow config X to receive clauses" << endl;
		cerr << " NW_configX file    X ... number of thread, file ... name of configuration file for this thread" << endl;
		cerr << " ... sending" << endl;
		cerr << " NW_sMinAct  0-n    minimum activity to send clause (0=all)" << endl;
		cerr << " NW_sMaxSize 0-n    max. size to still send (0=all)" << endl;
		cerr << " NW_sMode    0,1,2  send only length below own 1: learned minimum 2: learned average    since the last restart" << endl;
		cerr << " ... receiving" << endl;
		cerr << " NW_rMaxSize 0-n    maximum size, that is received" << endl;
		cerr << " NW_rNoSat   0,1    reject clauses, that are satisfied" << endl;
		cerr << " NW_rMode    0,1,2  receive only length below own 1: learned minimum 2: learned average    since the last restart" << endl;
	}
#endif
#endif
}

inline void* threadSolve(void *data)
{
	Network::threadInfo* info = (Network::threadInfo*) data;
	ComponentManager cm;
	if( false ){
		stringstream s;
		s << "c NW thread " << info->thread << " started with finished=" << info->finished << endl;
		cerr << s.str();
	}
	
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);
	
	
	
	
	info->solutions = cm.solve( info->clause_set, info->var_cnt,info->cmd, *(info->nw));
	if( false ){
		stringstream s1;
		s1 << "c NW thread " << info->thread << " finished and copied solutions to " << hex << info->solutions << dec << endl;
		cerr << s1.str();
	}
	volatile int end = 1;
	info->finished = end;

	
	if( false ){
		stringstream s3;
		s3 << "c NW wake parent up" << endl;
		cerr << s3.str();
	}
	info->nw->wakeUp();
	return 0;
}


#endif
