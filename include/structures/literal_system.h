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


#ifndef _LIT_SYSTEM_H
#define _LIT_SYSTEM_H

#include <assert.h>
#include <inttypes.h>
#include "types.h"


#include <iostream>

#define NO_LIT 0
#define NO_VAR 0







inline lit_t lit( const int32_t number ){
 return (number < 0) ? (-2*number + 1) : 2*number;
}


inline lit_t inv_lit( const lit_t lit1 ){
 return lit1 ^ (lit_t)1;
}


inline int32_t nr( const lit_t literal ){
 return (literal%2==1) ? (((int32_t)literal)/2*-1) : (literal/2);
}


inline var_t var( const lit_t literal ){
 return literal / 2;
}


inline lit_t lit( const var_t number , pol_t pol){
 return (pol == NEG) ? (2*number + 1) : 2*number;
}


inline pol_t inv_pol( const pol_t polarity )
{
	return (pol_t)(3^(uint32_t )polarity);
}


inline pol_t pol( const lit_t literal ){
	
	
	return (pol_t)(1 + (literal & 0x1));
}


inline uint32_t index( const lit_t literal ){
	return literal;
}


inline uint32_t max_index( const uint32_t variable_count ){
	return 2 * ( variable_count + 1 );
}


#endif
