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


#ifndef CONFLICTDUMP
#define CONFLICTDUMP

#include "debug.h"
#include "types.h"

#include "macros/vector_macros.h"
#include "macros/stack_macros.h"
#include "macros/clause_macros.h" 
#include "utils/clause_container.h"

#include "utils/prefetch.h"
#include "sat/searchdata.h"

#include <fstream>
#include <string>
#include <sstream>
#include <set>


class ConflictDump{
	std::ofstream myfile; 
public:
	void dump(int conflictNr, CL_REF conflict_clause, searchData& search );
	void finalize_last();
	
	void addInfo(std::stringstream& stream, bool bigskip=true);
	
	void addVec(std::string name, VEC_TYPE(lit_t)& lits, bool bigskip=true );
	void addCls(std::string name, CL_REF cls, bool bigskip=true);
	
	~ConflictDump(){
		if( myfile.is_open() ) finalize_last();
	}
private:
	void TikZProlog(std::ofstream& file);
};

#endif
