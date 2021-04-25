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



#ifndef _CPP_CL_H
#define _CPP_CL_H

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <inttypes.h>

#include "types.h"
#include "structures/literal_system.h"

#include "macros/malloc_macros.h"

class CppClause
{
private:
	float activity;
	uint32_t size;
	lit_t	*literals;
	
	void init();
	void reset(std::vector<lit_t>& lits);
	void reset(const lit_t *lits, uint32_t count);
	void set_size(uint32_t new_size);
	
public:
	CppClause( bool learnt = true);
	CppClause(std::vector<lit_t>& lits, bool learnt = true);
	CppClause(const lit_t *lits, uint32_t count, bool learnt = true);
	CppClause& operator=(const CppClause& other)
	{
		init();
		set_size( other.size );
		assert( size == other.size);
		memcpy ( literals, other.literals, size*sizeof(lit_t) );
		activity = other.activity;
		return *this;
	}
		
	~CppClause();
	CppClause	copy() const;
	bool equals(const CppCacheClause& other) const;
	uint32_t   get_size() const;
	lit_t	get_literal(uint32_t pos) const;
	void set_literal(uint32_t pos, lit_t lit1);
	void add(lit_t lit1);
	void remove(lit_t lit1);
	void remove_index(uint32_t ind);
	void swap_literals(uint32_t pos1, uint32_t pos2);
	bool contains_literal( const lit_t literal);
	
	bool is_learnt() const { return get_activity() != -3.0f ; }
	float get_activity() const;
	void set_activity( const float activty);
	void inc_activity( const float activty);
	
	CppClause* resolve(const CppClause&other) const;
	CppClause* resolve(const CppClause&other, var_t variable) const;
	
	bool check_resolve( const CppClause& other, var_t variable) const;
	bool subsumes_or_simplifies_clause(const CppClause& other, lit_t *lit1) const;
	
	
	void ignore();
	
	bool is_ignored() const;
	
	void ignore_and_delete();
	
	bool is_ignored_and_deleted() const;
} CL_PACK ;

#include "utils/statistics.h"

inline bool CppClause::equals(const CppClause& other) const {
	const uint32_t myS = get_size();
	const uint32_t oS = other.get_size();
	for( uint32_t i = 0 ; i < myS ; ++i ){
		uint32_t j = 0;
		for( ; j < oS; ++j ){
			if( get_literal(i) == other.get_literal(j) ) break;
		}
		if( j != oS ) return false;
	}
	return true;
}

inline uint32_t CppClause::get_size() const
{
	return size;
}

inline lit_t CppClause::get_literal(uint32_t pos) const
{
	assert(pos < size);
	STAT_READLIT(pos );
	return literals[pos];
}

inline void CppClause::set_literal(uint32_t pos, lit_t lit1)
{
	assert(pos < size);
	STAT_WRITELIT( pos );
	literals[pos] = lit1;
}

inline void CppClause::swap_literals(uint32_t pos1, uint32_t pos2)
{
	STAT_SWAPLITS( pos1, pos2 );
	lit_t tmp = get_literal(pos1);
	set_literal(pos1, get_literal(pos2));
	set_literal(pos2, tmp);
}

inline CppClause CppClause::copy() const
{
	CppClause cl = new CppClause(literals, size);
	cl.set_activity( get_activity() );
	return cl;
}


inline void CppClause::init()
{
	activity = 0;
	size = 0;
	literals = 0;
}

inline void CppClause::set_size(uint32_t new_size){
	if( literals == 0 ){
		literals = (lit_t*)MALLOC_SIZE( sizeof(lit_t) * new_size );
	} else {
		literals = (lit_t*)REALLOC_SIZE( literals, sizeof(lit_t) * new_size, sizeof(lit_t) * size);
	}
	size = new_size;
}

inline CppClause::CppClause(bool learnt)
{
	init();
	set_size(0);
	if( !learnt ) set_activity( -3.0f );
}

inline CppClause::CppClause(std::vector<lit_t>& lits, bool learnt)
{	
	init();
	reset(lits);
	if( !learnt ) set_activity( -3.0f );
}

inline CppClause::CppClause(const lit_t *lits, uint32_t count, bool learnt)
{
	init();
	reset(lits, count);
	if( !learnt ) set_activity( -3.0f );
}

inline CppClause::~CppClause()
{
	if(literals != 0) FREE_SIZE(literals, sizeof(lit_t) * size);
	literals = 0;
	size = 0;
	set_activity(0);
}

inline void CppClause::add(lit_t lit1)
{
	set_size(get_size()+1);
	set_literal(get_size()-1, lit1);	
}

inline void CppClause::remove(lit_t lit1)
{
	uint32_t i, j;
	
	for (i = 0, j = 0; i < get_size(); i ++) {
		if (get_literal(i) != lit1)
			set_literal(j++, get_literal(i));
	}
	set_size(j);
}

inline void CppClause::remove_index(uint32_t ind)
{
	swap_literals( ind, get_size() - 1 );
	set_size( get_size() - 1);
}

inline void CppClause::reset(std::vector<lit_t>& lits)
{
	set_size(lits.size());
	for (uint32_t i=0; i<lits.size(); i++)
		set_literal(i, lits[i]);
}

inline void CppClause::reset(const lit_t *lits, uint32_t count)
{
	set_size(count);
	for (uint32_t i=0; i<count; i++)
		set_literal(i, lits[i]);
}

inline bool CppClause::contains_literal( const lit_t literal)
{
	for (uint32_t i = 0; i < size; i++)
	{
		if( literals[i] == literal ) return true;
	}
	return false;
}

inline float CppClause::get_activity() const
{
	return activity;
}

inline void CppClause::set_activity( const float activity)
{
	this->activity = activity;
}

inline void CppClause::inc_activity( const float act)
{
	this->activity += act;
}

inline CppClause* CppClause::resolve(const CppClause& other) const
{
	for( int32_t i = this->size - 1; i >=0; i--)
	{
		for( int32_t j = other.get_size() - 1; j >=0; j--)
		{
			if( get_literal( i ) == inv_lit( other.get_literal( j ) ) )
			return resolve( other, var( get_literal( i ) ) );
		}
	}
	return 0;
}

inline CppClause* CppClause::resolve(const CppClause& other, var_t variable) const
{
	var_t var11, var12;
	lit_t lit11, lit12;
	uint32_t j, sz;
	
	uint32_t bytes = (get_size() + other.get_size()) * sizeof(lit_t);
	
	lit_t new_lits[ bytes ];
	sz = 0;
	
	const CppClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const CppClause& cls2 = (get_size() < other.get_size()) ? other : (*this);
	
	for (uint32_t i = 0; i < cls1.  get_size(); i++)
	{
		lit11 = cls1.get_literal(i);
		var11 = var(lit11);
		
		if (variable == var11)
			continue;
		
		for (j = 0; j < cls2.  get_size(); j++)
		{
			lit12 = cls2.get_literal(j);
			var12 = var(lit12);
			
			if (var12 == var11) {
				if (lit11 == inv_lit(lit12)) {
					
					return 0;
				}
				else
					break;
			}
		}
		
		if (j == cls2.get_size())
			new_lits[sz++] = lit11;
	}
	
	for (uint32_t j = 0; j < cls2.  get_size(); j++)
	{
		lit12 = cls2.get_literal(j);
		var12 = var(lit12);
		
		if (var12 != variable)
			new_lits[sz++] = lit12;
	}
	
	
	return new CppClause(new_lits, sz);
}

inline bool CppClause::check_resolve( const CppClause& other, var_t variable) const
{
	const CppClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const CppClause& cls2 = (get_size() < other.get_size()) ? other : (*this);
	
	for (uint32_t i = 0; i < cls1.get_size(); i++)
	{
		const lit_t lit11 = cls1.get_literal(i);
		const var_t var11 = var(lit11);
		
    	if (variable == var11)
    		continue;
		
		for (uint32_t j = 0; j < cls2.get_size(); j++)
		{
			const lit_t lit12 = cls2.get_literal(j);
			const var_t var12 = var(lit12);
			
			if (var12 == var11) {
				if (lit11 == inv_lit(lit12))
					return false;
				else
					break;
			}
		}
	}
	
	
	return true;
}

inline bool CppClause::subsumes_or_simplifies_clause(const CppClause& other, lit_t *lit1) const
{
	uint32_t i, j, other_size;
	
	
	other_size = other.get_size();
	if (get_size() > other_size)
		return false;
	
	

	uint64_t hash = 0;
	
	for (i=0; i<get_size(); i++) {
		hash |= ( 1 << ( var( get_literal(i) ) & 63 ) );
	}
	uint64_t other_hash = 0;
	for (i=0; i<other.get_size(); i++) {
		other_hash |= ( 1 << ( var( other.get_literal(i) ) & 63 ) );
	}

	if ((hash & ~other_hash ) != 0)
		return false;
		
	*lit1 = NO_LIT;
	
	
	for (i=0; i<get_size(); i++) {
		for (j=0; j<other_size; j++) {
			lit_t other_lit1;
			
			other_lit1 = other.get_literal(j);
			
			if (get_literal(i) == other_lit1)
				break;
			if ((*lit1 == NO_LIT )&& (get_literal(i) == inv_lit(other_lit1))) {
			  *lit1 = other_lit1;
				break;
			}
			if ((*lit1 != NO_LIT )&& (get_literal(i) == inv_lit(other_lit1)))
			  return false;
			
		}
		
		
		if (j==other_size)
			return false;
	}
	
	
	return true;
}

inline void CppClause::ignore()
{
	if( get_activity() != -2 ) set_activity( -1 );
}


inline bool CppClause::is_ignored() const
{
	return ( get_activity() == -1 || get_activity() == -2 );
}

inline void CppClause::ignore_and_delete()
{
	set_activity( -2 );
}

inline bool CppClause::is_ignored_and_deleted() const
{
	return ( get_activity() == -2 );
}

#endif
