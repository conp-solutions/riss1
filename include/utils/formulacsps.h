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


#ifndef _FORMULACSPS_H
#define _FORMULACSPS_H

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
#include "debug.h"

class FormulaCsps{
	VEC_TYPE (VEC_TYPE (CL_REF)) binOcc;

	char* seen;

public:
	FormulaCsps();
	~FormulaCsps();

	
	
	void analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment );
	
private:
	
	bool oneOutOfN(CL_REF c);
	
	
	bool binXor(CL_REF c);
	
	
	void FindXor( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt );
	
	
	int litRatio(CL_REF c);
	
	
	bool dISaANDb(CL_REF c);
	
	
	void sort(VEC_TYPE( CL_REF )& clauses);
};




inline FormulaCsps::FormulaCsps() : seen( 0 )
{
}

inline  FormulaCsps::~FormulaCsps()
{
}

#endif
