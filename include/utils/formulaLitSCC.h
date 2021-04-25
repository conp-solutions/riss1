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


#ifndef _FORMULALITSCC_H
#define _FORMULALITSCC_H

#include <iostream>

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>

#include <inttypes.h>

#include "structures/c_assignment.h"

#include "macros/vector_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"

#include <set>
#include <vector>
#include <deque>

#define MINIM(x,y) (x) < (y) ? (x) : (y)


class FormulaLitSCC{

	int ind;
	std::vector< lit_t > stack;
	std::vector<unsigned char> inStack;

	std::vector< std::vector<  lit_t > > SCC;
	std::vector<unsigned char> inSCC;
	
	std::vector< int32_t > nodeindes;
	std::vector< int32_t > nodelowlinks;
	
	std::vector< std::set<lit_t> > graph;

	std::vector< lit_t > replaceWith;

public:
	FormulaLitSCC();
	~FormulaLitSCC();

	
	
	void analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment  );

private:	
	std::vector< lit_t> tarjan(lit_t v, lit_t list);
};






inline  FormulaLitSCC::~FormulaLitSCC()
{
}

#endif
