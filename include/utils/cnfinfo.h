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


#ifndef _CNFINFO_H
#define _CNFINFO_H

#include <iostream>

#include "defines.h"
#include "types.h"
#include "macros/clause_macros.h" 
#include "structures/c_assignment.h"

#include "utils/formulaanalyzer.h"
#include "utils/formulaLitSCC.h"
#include "utils/formulaSCCfinder.h"
#include "utils/formulacsps.h"

#include "utils/stringmap.h"

using namespace std;

class CnfInfo{

	uint32_t level;
	bool noSearch;

public:
	CnfInfo(const StringMap& commandline);
	~CnfInfo();

	
	
	void analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const assi_t assignment );
	
private:

	void checkXOR( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const assi_t assignment );

};


#endif
