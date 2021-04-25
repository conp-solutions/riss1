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


#ifndef _SCCFINDER_H
#define _SCCFINDER_H

#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/vector_macros.h"



#include "types.h"
#include "utils/stringmap.h"
#include "utils/sort.h"
#include "debug.h"
#include "structures/c_assignment.h"
#include "macros/vector_macros.h"

#include <set>
#include <vector>

#include "sat/searchdata.h"

#define MININ(x,y) (x) < (y) ? (x) : (y)


class SCCfinder
{
	
	uint32_t var_cnt;

	
	int ind;
	VEC_TYPE( lit_t ) stack;
	VEC_TYPE(unsigned char) inStack;
	VEC_TYPE( int32_t ) nodeindes;
	VEC_TYPE( int32_t ) nodelowlinks;

	
	VEC_TYPE( VEC_TYPE(  lit_t ) ) SCC;
	
	VEC_TYPE(unsigned char) inSCC;
	
	VEC_TYPE( std::set<lit_t> ) graph;

public:
	SCCfinder( uint32_t varCnt);
	~SCCfinder();

	
	void findSCCs(const VEC_TYPE( CL_REF )& clauses, assi_t assi);
	
	
	void resetGraph();
	
	
	uint32_t size() const;
	
	
	const VEC_TYPE(lit_t)& getEqs(uint32_t i);

	
	void storeGraph(const std::string& filename);

private:
	void create_graph( const VEC_TYPE( CL_REF )& orig_clauses, assi_t assi);
	void prepareSCCsearch();
	VEC_TYPE( lit_t) tarjan(lit_t v, lit_t list);
};

#endif
