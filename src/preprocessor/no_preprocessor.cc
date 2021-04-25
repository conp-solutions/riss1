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


#include "preprocessor/no_preprocessor.h"

#ifdef COMPILE_LIBRARY

extern "C" NoPreprocessor* create_preprocessor(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline ){
	return new NoPreprocessor( clause_set, var_cnt, commandline );
}

extern "C" void destroy_preprocessor( NoPreprocessor* heuristic ){
	delete heuristic;
}
#endif
