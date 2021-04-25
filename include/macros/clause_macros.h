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


#ifndef _CL_MACROS_H
#define _CL_MACROS_H



#include "defines.h"

#ifdef USE_RISS_CLAUSE

#include "structures/riss_clause.h"

#define CLAUSE RissClause
#define CL_REF uint32_t
#define CL_OBJSIZE sizeof(RissClause)

#define CL_CREATE_EMPTY() RissClause()
#define CL_CREATE_STD( stdvector ) RissClause( stdvector )
#define CL_CREATE( array, size ) RissClause( array, size ) 
#define CL_CREATE_NO_LEARNT( array, size ) RissClause( array, size, false) 

#define CL_EQUAL( clause, other ) clause.equals( other )
#define CL_COPY( clause ) (clause).copy()
#define CL_COPY_EXCEPT( clause, var ) (clause).copy_except(var)
#define CL_DESTROY( clause ) (clause).RissClause::~RissClause()
#define CL_CONTAINS_LIT( clause, literal ) (clause).contains_literal( literal) 
#define CL_ADD_LIT( clause, literal ) (clause).add( literal) 
#define CL_DELETE_LIT( clause, literal ) (clause).remove( literal) 
#define CL_DELETE_INDEX( clause, ind ) (clause).remove_index( ind)  
#define CL_SORT( clause ) (clause).sort()
#define CL_IS_LEARNT( clause ) (clause).isLearnt() 
#define CL_SET_NO_LEARNT( clause ) (clause).setLearnt(false)
#define CL_GET_ACTIVITY( clause ) (clause).get_activity() 
#define CL_SET_ACTIVITY( clause, pactivity ) (clause).set_activity( pactivity ) 
#define CL_INC_ACTIVITY( clause, pactivity ) (clause).inc_activity( pactivity )
#define CL_SIZE( clause ) (clause).get_size() 
#define CL_RESOLVE( clause1, clause2 ) (clause1).resolve( clause2 ) 
#define CL_CHECK_RESOLVE( one, other, variable ) (one).check_resolve( other, variable)
#define CL_RESOLVE_VAR( clause1, clause2, variable ) (clause1).resolve( clause2, variable ) 
#define CL_SUBSUMES_OR_SIMPLIFIES( one, other, lit1 ) (one).subsumes_or_simplifies_clause(other, lit1)
#define CL_GET_LIT( clause, ind ) (clause).get_literal( ind )
#define CL_SET_LIT( clause, ind, literal ) (clause).set_literal( ind, literal)
#define CL_SWAP_LITS( clause, ind1, ind2 ) (clause).swap_literals( ind1,  ind2)

#define CL_UNIGNORE( clause1 ) (clause1).setFlag1(false)
#define CL_IGNORE( clause1 ) (clause1).setFlag1(true)
#define CL_IS_IGNORED( clause1 ) ( (clause1).getFlag1() || (clause1).getFlag2() )
#define CL_IGNORE_AND_DELETE( clause1 ) (clause1).setFlag2(true)
#define CL_IS_IGNORED_AND_DELETED( clause1 ) (clause1).getFlag2()


#endif

#endif
