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


#ifndef _Coprocessor_H
#define _Coprocessor_H

#include "defines.h"
#include "types.h"

#include "structures/literal_system.h"
#include "structures/c_assignment.h"
#include "structures/cpp_heap.h"
#include "structures/c_boolarray.h"

#include "macros/vector_macros.h"
#include "macros/clause_macros.h"
#include "utils/clause_container.h"
#include "macros/heap_macros.h"

#include "utils/stringmap.h"
#include "utils/microtime.h"
#include "utils/sort.h"
#include "utils/varfileparser.h"
#include "utils/prefetch.h"

#include "sat/searchdata.h"

#include "preprocessor/SCCfinder.h"
#include "preprocessor/ROProp.h"

#include <vector>
#include <sstream>
#include <pthread.h>

#include<unistd.h>

using namespace std;


class Coprocessor
{
	
	CONST_PARAM uint32_t debug;	
	CONST_PARAM bool print_dimacs;	
	CONST_PARAM uint32_t simpDiff;	
	CONST_PARAM int32_t maxCls;	
	CONST_PARAM int32_t maxVEcls;	
	CONST_PARAM int32_t maxQsize;	
	CONST_PARAM float hteP;	
	CONST_PARAM int32_t blockMaxSize; 
	CONST_PARAM int32_t blockMaxOcc;	
	CONST_PARAM bool createBinaries;	
	CONST_PARAM bool vePropagate;	
	CONST_PARAM int32_t probVars;	
	CONST_PARAM int32_t probVarsP;	
	CONST_PARAM int32_t probClVars;	
	CONST_PARAM int32_t probClMaxS;	
	CONST_PARAM int32_t probClMaxN;	
	CONST_PARAM uint32_t erMaxV;	
	CONST_PARAM uint32_t erMinV;	
	CONST_PARAM uint32_t erPairs;	
	CONST_PARAM uint32_t erMinSize;	
	CONST_PARAM uint32_t erMinO; 
	CONST_PARAM uint32_t erTries;	
	CONST_PARAM uint32_t erIters;	
	CONST_PARAM int32_t aMinSize;	
	CONST_PARAM uint32_t aPercent;	
	CONST_PARAM uint32_t simpEvery;	
	CONST_PARAM bool dumpBiGraph;	
	uint32_t pure;		
	CONST_PARAM uint32_t prop_pure;	
	uint32_t blocked;	
	CONST_PARAM uint32_t ve;	
	CONST_PARAM uint32_t hteR;	
	CONST_PARAM uint32_t ee;	
	CONST_PARAM uint32_t er;	
	CONST_PARAM uint32_t probSize;	
	CONST_PARAM uint32_t probL;	
	CONST_PARAM uint32_t probC;	
	CONST_PARAM uint32_t asymm;	
	CONST_PARAM uint32_t ternary;	
	CONST_PARAM uint32_t rounds;	
	uint32_t simpPure;	
	uint32_t simpBlocked; 
	CONST_PARAM bool simpVe;      
	CONST_PARAM bool simpHte;     
	CONST_PARAM bool simpEe;      
	CONST_PARAM bool simpSubsSimp;	
	CONST_PARAM bool simpProbL;   
	CONST_PARAM bool simpProbC;   
	CONST_PARAM bool simpPAsymm;  
	CONST_PARAM uint32_t ppTimeout;	
	CONST_PARAM uint32_t simpTimeout;	
	CONST_PARAM bool usePrefetch;	
	std::string blackFile;	
	std::string whiteFile;	




	uint32_t cRound;	
	VEC_TYPE (uint32_t) pureLits;
	VEC_TYPE (uint32_t) removedBlocked;
	VEC_TYPE (uint32_t) removedVars;
	VEC_TYPE (uint32_t) removedVarCls;
	VEC_TYPE (uint32_t) removedSubsCls;
	VEC_TYPE (uint32_t) removedEquiCls;
	VEC_TYPE (uint32_t) foundProbUnits;
	VEC_TYPE (uint32_t) removedPropCls;
	VEC_TYPE (uint32_t) asymmNewCls;
	VEC_TYPE (uint32_t) hteRemoved;
	VEC_TYPE (uint32_t) hteUnits;
	uint32_t addedBinaries;	
	solution_t solution;
	uint32_t maxClLen;
	uint32_t var_cnt;
	bool simplifying;
	uint32_t simplifyRejections;
	uint64_t stopTime;	
	int32_t max_solutions;	



	
	struct elimination_t{
		var_t var1;
		uint32_t number;
		CL_REF *clauses;
	};

	
	struct blocking_t{
		lit_t lit1;
		CL_REF clause;
	};

	
	struct postEle{
		char kind;			
		union{
			blocking_t b;
			elimination_t e;
		};
	};

	VEC_TYPE (CL_REF) *clauses;
	VEC_TYPE (CL_REF) clausesR;
	VEC_TYPE (VEC_TYPE (CL_REF)) occ;
	uint32_t *vOcc;
	uint32_t* lOcc;	
	uint32_t lastTrailSize;	

	VEC_TYPE (lit_t) unitQ;	
	VEC_TYPE (CL_REF) subsQ;	

	boolarray_t touchedVars;	
	boolarray_t elimWhite; 
	boolarray_t eliminated;	
	boolarray_t hla_lits;	

	assi_t ta;	

	cpp_heap < uint32_t, std::less < uint32_t > >resQ;	
	cpp_heap<lit_t, std::less<uint32_t> > litHeap; 

	
	std::vector<lit_t> replacedBy;
	boolarray_t varRemoved;	

	ROProp propagation;	
	assi_t* lAssi;	
	assi_t tmpAssi; 
	VEC_TYPE (lit_t) tmpLits;	
	VEC_TYPE (lit_t) tmpLits2;	

	std::vector < postEle > postProcessStack;	

	searchData* data;	

	const StringMap & cmd;	
	
#ifdef TRACK_CLAUSE
	lit_t trackme;
#endif
public:
	Coprocessor (VEC_TYPE (CL_REF) * clause_set, uint32_t var_cnt, const StringMap & commandline);
	
	~Coprocessor ();

	solution_t preprocess ( searchData& sd, VEC_TYPE( CL_REF )* formula);
	
	bool doSimplify(searchData& sd);
	
	solution_t simplifysearch (searchData& sd, VEC_TYPE (CL_REF)& clauses, VEC_TYPE (CL_REF)& learnt);
	
	void postprocess (VEC_TYPE (assi_t) * assignments);
	
	void postprocessSingle (assi_t assignment);
	
	
	void addEquis( const VEC_TYPE(lit_t)& eqs );

	
	void extend( uint32_t newVar );

private:
	
	
	void detectPure (assi_t assignment);
	
	solution_t remove_blocked(assi_t assignment);
	
	solution_t eliminate_variables (assi_t assignment);
	
	bool searchEqui(assi_t assignment);
	solution_t applyEqui(assi_t assignment);

	
	solution_t extendedResolution(searchData& search);

	
	solution_t hyberBinProb(assi_t assignment);
	solution_t failedLitProbing(assi_t assignment);
	solution_t clauseProbing(assi_t assignment);
	solution_t asymmBranch(assi_t assignment);
	
	
	solution_t ternaryResolution(assi_t assignment);

	
	solution_t hte(assi_t assignment, uint32_t first = 0);	

	
	void fill_hla(lit_t l);	

	
	solution_t addSingleClause (assi_t assignment, const CL_REF newCls); 
	solution_t fillOcc (assi_t assignment);	
	void resetStructures(); 
	void resetAssi(); 
	void remClOcc (CL_REF clause);	
	void remIgnClOcc ();	
	
	
	bool enqueue(assi_t assignment, const lit_t l);
	solution_t propagate (assi_t assignment);	

	void reduceClauses ();	
	solution_t subsSimp (assi_t assignment);	

	
	
	
	
	solution_t resolveVar (assi_t assignment, var_t var1, int force=0);

	void createBinaryClauses( assi_t assignment, lit_t first, VEC_TYPE(lit_t)& others);	
	void addActiveCls();	
	void freeStructures (); 

	
	void scanClauses (assi_t assignment);
	
	
	void prefetchOcc( const lit_t l ) const;

	void set_parameter (const StringMap & commandline);
};

#endif
