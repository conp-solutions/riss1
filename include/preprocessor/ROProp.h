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


#ifndef ROPROP_H_
#define ROPROP_H_

#include "defines.h"

#include "macros/vector_macros.h"
#include "macros/ringbuffer_macros.h"
#include "macros/clause_macros.h"
#include "utils/clause_container.h"
#include "structures/c_assignment.h"
#include "types.h"

#include <iostream>
using namespace std;


class ROProp {
	
	struct l2_t {
		lit_t a[2];
	};
	
	uint32_t varCnt;	
	VEC_TYPE( VEC_TYPE(lit_t)) w2;	
	VEC_TYPE( VEC_TYPE(l2_t) ) w3;	
	
	uint32_t maxUse;	
	RINGBUFFER_TYPE( lit_t ) unitQ; 
public:
	
	ROProp(uint32_t var_cnt, uint32_t maxUsedClSize = 2);
	~ROProp();

	
	solution_t init(VEC_TYPE (CL_REF) * clauses,  assi_t assi);
	solution_t init(VEC_TYPE (CL_REF) * clauses,  assi_t assi, VEC_TYPE(lit_t)& newUnits);

	
	void clear();

	
	bool createAssignment(assi_t orig, assi_t nAssi, const lit_t assume, VEC_TYPE(lit_t)& implications);

	
	bool conflictFree(assi_t orig, assi_t nAssi, VEC_TYPE(lit_t)& assumptions, VEC_TYPE(lit_t)& conflict);
	
	
	void setMax( uint32_t max );
	
	
	void extend( uint32_t newVar );
private:
	
	bool addClause(CL_REF cl, assi_t assi);

	
	bool enqueue(assi_t assi, lit_t l);

	
	lit_t dequeue();

	
	bool propagate(lit_t l, assi_t assi);

	
	solution_t fillAssi(assi_t assi);
};

#endif
