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



#ifndef _CPP_CACHE_CL_H
#define _CPP_CACHE_CL_H


#define _CL_ELEMENTS 4

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <vector>


#include <iostream>

#include <inttypes.h>

#include "utils/statistics.h"
#include "types.h"
#include "structures/literal_system.h"
#include "macros/malloc_macros.h"

class CppCacheClause
{

private:
	uint32_t		size;
	lit_t	own_lits[_CL_ELEMENTS];
	lit_t	*ext_lits;
	float activity;
	void init();
		
public:
	CppCacheClause( bool learnt=true);
	CppCacheClause(std::vector<lit_t>& lits, bool learnt=true);
	CppCacheClause(const lit_t *lits, uint32_t count, bool learnt=true);
	CppCacheClause& operator=(const CppCacheClause& other)
	{
		init();
		set_size( other.size );
		assert( size == other.size);
		if(size>_CL_ELEMENTS){
			memcpy (own_lits, other.own_lits, _CL_ELEMENTS*sizeof(lit_t));
			memcpy (ext_lits, other.ext_lits, (size- _CL_ELEMENTS)*sizeof(lit_t));
		} else {
			memcpy (own_lits, other.own_lits, _CL_ELEMENTS*sizeof(lit_t));
		}
		activity = other.activity;
		return *this;
	}
	~CppCacheClause();

	void reset(std::vector<lit_t>& lits);
	void reset(const lit_t *lits, uint32_t count);

	bool equals(const CppCacheClause& other) const;
	uint32_t	get_size() const ;
	void		set_size(uint32_t size);
	lit_t	get_literal(uint32_t pos) const;
	void		set_literal(uint32_t pos, lit_t lit1);
	CppCacheClause	copy();
	void	add(lit_t lit1);
	void	remove(lit_t lit1);
	void  remove_index(uint32_t ind);
	void	swap_literals(uint32_t pos1, uint32_t pos2);
	

	bool contains_literal( const lit_t literal);
	
	bool is_learnt() const { return get_activity() != -3.0f ; }
	float get_activity() const;
	void set_activity( const float activity);
	void inc_activity( const float activity);
	
	CppCacheClause* resolve(const CppCacheClause& other) const;
	CppCacheClause* resolve(const CppCacheClause& other, var_t variable) const;
	
	bool check_resolve( const CppCacheClause& other, var_t variable) const;
	bool subsumes_or_simplifies_clause(const CppCacheClause& other, lit_t *lit1) const;
	
	
	void ignore();
	
	bool is_ignored() const;
	
	void ignore_and_delete();
	
	bool is_ignored_and_deleted() const;
	
	
	void printInfo();
} CL_PACK ;

	
inline void CppCacheClause::printInfo(){
	std::cerr << "cl[" << std::hex << this << std::dec << "]: size(" << size << "), act(" << activity
			<< "), ext(" << size - _CL_ELEMENTS << ") at " << std::hex << ext_lits << std::dec << ", locals:";
	for( uint32_t i = 0 ; i < _CL_ELEMENTS; ++i ){
		std::cerr << " " << nr( own_lits[i] );
	}
	std::cerr << " and externals: ";
	for( uint32_t i = _CL_ELEMENTS; i < size ; ++i ){
		std::cerr << " " << nr( ext_lits[i-_CL_ELEMENTS] );
	}
	std::cerr << std::endl;
}

inline CppCacheClause::CppCacheClause(bool learnt)
{
	init();
	set_size(0);
	if( !learnt ) set_activity( -3.0f );
}

inline CppCacheClause::CppCacheClause(std::vector<lit_t>& lits, bool learnt)
{	
	init();
	reset(lits);
	if( !learnt ) set_activity( -3.0f );
}

inline CppCacheClause::CppCacheClause(const lit_t *lits, uint32_t count, bool learnt)
{
	init();
	reset(lits, count);
	if( !learnt ) set_activity( -3.0f );
}

inline CppCacheClause::~CppCacheClause()
{
	if(ext_lits != 0) FREE_SIZE(ext_lits,sizeof(lit_t) * (size - _CL_ELEMENTS));
	ext_lits = 0;
	size = 0;
	set_activity( 0 );
}

inline void CppCacheClause::reset(std::vector<lit_t>& lits)
{
	set_size(lits.size());
	

	for (uint32_t i=0; i<lits.size(); i++)
		set_literal(i, lits[i]);

}

inline void CppCacheClause::reset(const lit_t *lits, uint32_t count)
{
	set_size(count);

	for (uint32_t i=0; i<count; i++)
		set_literal(i, lits[i]);

}

inline bool CppCacheClause::equals(const CppCacheClause& other) const {
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

inline uint32_t CppCacheClause::get_size() const 
{
	return size;
}

inline lit_t CppCacheClause::get_literal(uint32_t pos) const
{
	STAT_READLIT(pos );
	return (pos < _CL_ELEMENTS) ? own_lits[pos] : ext_lits[pos-_CL_ELEMENTS];
}

inline void CppCacheClause::set_literal(uint32_t pos, lit_t lit1)
{
	STAT_WRITELIT( pos );
	(pos <_CL_ELEMENTS) ? own_lits[pos] = lit1 : ext_lits[pos-_CL_ELEMENTS] = lit1;
}

inline void CppCacheClause::swap_literals(uint32_t pos1, uint32_t pos2)
{
	assert (pos1 < size);
	assert (pos2 < size);
	STAT_SWAPLITS( pos1, pos2 );
	
	lit_t tmp = get_literal(pos1);	
	set_literal(pos1, get_literal(pos2));
	set_literal(pos2, tmp);
}

inline void CppCacheClause::add(lit_t lit1)
{
	set_size(get_size()+1);
	set_literal(get_size()-1, lit1);	
}

inline void	CppCacheClause::remove(lit_t lit1)
{
	uint32_t i, j;
	
	for (i = 0, j = 0; i < get_size(); i ++) {
		if (get_literal(i) != lit1)
			set_literal(j++, get_literal(i));
	}
	set_size(j);
}

inline void CppCacheClause::remove_index(uint32_t ind)
{
	swap_literals( ind, get_size() - 1 );
	set_size( get_size() - 1 );
}


inline CppCacheClause CppCacheClause::copy()
{
	std::vector<lit_t> tmp;
	tmp.reserve( size );

	
	if( size > _CL_ELEMENTS )
	{
		for(int i=0; i< (int)_CL_ELEMENTS; ++i)
			tmp.push_back(own_lits[i]);
		
		for (int i=0; i<(int)(size-_CL_ELEMENTS); ++i)
			tmp.push_back(ext_lits[i]);
	} else {
		for(int i=0; i< (int)size; ++i)
			tmp.push_back(own_lits[i]);
	}
	
	CppCacheClause cl(tmp);
	cl.set_activity( get_activity() );
	return cl;
}

inline void CppCacheClause::set_size(uint32_t new_lits)
{
	assert( sizeof(lit_t)==4 );
	
	if (size != new_lits) {
		
		
		
		if (new_lits > _CL_ELEMENTS) {
			if(size > _CL_ELEMENTS){
				
				#ifndef JAVALIB
				ext_lits = (lit_t *)REALLOC_SIZE(ext_lits, 
																	sizeof(lit_t) * (new_lits - _CL_ELEMENTS),
																	sizeof(lit_t) * (size - _CL_ELEMENTS) );
				#else
					lit_t* tmp = (lit_t*)MALLOC_SIZE( sizeof(lit_t) * (new_lits - _CL_ELEMENTS ) );
					if( tmp == 0 ) std::cerr << "c was not able to Mallocate memory for a new clause" << std::endl;
					memcpy( tmp, ext_lits, sizeof(lit_t) * (size - _CL_ELEMENTS) );
					FREE_SIZE( ext_lits, sizeof(lit_t) * (size - _CL_ELEMENTS) );
					ext_lits = tmp;
				#endif
			} else { 
				ext_lits = (lit_t *)MALLOC_SIZE(sizeof(lit_t) * (new_lits - _CL_ELEMENTS) );
				#ifdef JAVALIB
					if( ext_lits == 0 ) std::cerr << "c was not able to Mallocate memory for a new clause" << std::endl;
				#endif
				}
		} else {
			
			if (size > _CL_ELEMENTS){
				
				FREE_SIZE(ext_lits,sizeof(lit_t) * (size - _CL_ELEMENTS));
				ext_lits = 0;
			}
		}
		size = new_lits;
		
		
	}
}

inline void CppCacheClause::init()
{
	size = 0;
	ext_lits = 0;
	activity = 0;
	for (unsigned int i=0; i<_CL_ELEMENTS; ++i) own_lits[i]=NO_LIT;
}

inline float CppCacheClause::get_activity() const
{
	return activity;
}

inline void CppCacheClause::set_activity( const float activity)
{
	this->activity = activity;
}

inline void CppCacheClause::inc_activity( const float act)
{
	this->activity += act;
}

inline CppCacheClause* CppCacheClause::resolve(const CppCacheClause& other) const
{
	for( int32_t i = this->size - 1; i >=0; i--)
	{
		for( int32_t j = 
						other.get_size()
							 - 1; j >=0; j--)
		{
			if( get_literal( i ) == inv_lit( other.get_literal( j ) ) )
			return resolve( other, var( get_literal( i ) ) );
		}
	}
 	return 0;
}

inline CppCacheClause* CppCacheClause::resolve(const CppCacheClause& other, var_t variable) const
{
	var_t var11, var12;
	lit_t lit11, lit12;
	uint32_t j, sz;
	
	uint32_t bytes = (get_size() + other.get_size())  * sizeof(lit_t);
	
	lit_t new_lits[ bytes ];
	sz = 0;
	
	const CppCacheClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const CppCacheClause& cls2 = (get_size() < other.get_size()) ? other : (*this);
	
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
	
	
	return new CppCacheClause(new_lits, sz);
}

inline bool CppCacheClause::check_resolve( const CppCacheClause& other, var_t variable) const
{
	var_t var11, var12;
	lit_t lit11, lit12;
	
	const CppCacheClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const CppCacheClause& cls2 = (get_size() < other.get_size()) ? other : (*this);
	
	for (uint32_t i = 0; i < cls1.get_size(); i++)
	{
		lit11 = cls1.get_literal(i);
		var11 = var(lit11);
		
    	if (variable == var11)
    		continue;
		
		for (uint32_t j = 0; j < cls2.get_size(); j++)
		{
			lit12 = cls2.get_literal(j);
			var12 = var(lit12);
			
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

bool CppCacheClause::contains_literal( const lit_t literal)
{
	for (uint32_t i = 0; i < get_size(); i++)
	{
		if( get_literal(i) == literal ) return true;
	}
	return false;
}


inline bool CppCacheClause::subsumes_or_simplifies_clause(const CppCacheClause& other, lit_t *lit1) const
{
	uint32_t i, j, other_size;
	
	
	other_size = other.get_size();
	if (get_size() > other_size)
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

inline void CppCacheClause::ignore()
{
	if( get_activity() != -2 ) set_activity( -1 );
}


inline bool CppCacheClause::is_ignored() const
{
	return ( get_activity() == -1 || get_activity() == -2 );
}

inline void CppCacheClause::ignore_and_delete()
{
	set_activity( -2 );
}

inline bool CppCacheClause::is_ignored_and_deleted() const
{
	return ( get_activity() == -2 );
}

#endif 
