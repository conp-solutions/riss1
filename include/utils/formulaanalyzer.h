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


#ifndef _FORMULAANALYZER_H
#define _FORMULAANALYZER_H

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


class FormulaAnalyzer{
	uint32_t detaillevel;

public:
	FormulaAnalyzer();
	~FormulaAnalyzer();

	
	
	void analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment );
};




inline FormulaAnalyzer::FormulaAnalyzer() : detaillevel( 8 )
{
}

inline  FormulaAnalyzer::~FormulaAnalyzer()
{
}

#endif
