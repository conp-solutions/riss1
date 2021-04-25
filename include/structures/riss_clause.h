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



#ifndef _RISS_CLAUSE_H
#define _RISS_CLAUSE_H


#define _CL_ELEMENTS ((32 - sizeof(uint32_t) - sizeof(lit_t*) - sizeof(uint32_t)) / sizeof(lit_t))



#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <iostream>

#include <inttypes.h>

#include "defines.h"
#include "types.h"
#include "structures/literal_system.h"
#include "macros/malloc_macros.h"
#include "utils/statistics.h"




#ifdef TRACK_CLAUSE
	#define TRACK_CHECK(x) if( is1ooN() ) std::cerr << "c tracked clause: " << x << std::endl

#else
	#define TRACK_CHECK(x)
#endif


class RissClause
{

private:
	uint32_t _sizedata;	
	lit_t	own_lits[_CL_ELEMENTS];	
	lit_t	*ext_lits;	
	union {	
		float activity;
		uint32_t hash;
	} data;	
	void init();

public:
	
	RissClause( bool learnt=true);	
	RissClause(std::vector<lit_t>& lits, bool learnt=true);	
	RissClause(const lit_t *lits, uint32_t count, bool learnt=true);	
	RissClause& operator=(const RissClause& other);	
	~RissClause();

	
	bool operator >=(RissClause& other) const ;
	
	bool operator <=(RissClause& other) const;
	
	bool operator < (const RissClause& other) const;

	
	void reset(std::vector<lit_t>& lits);	
	void reset(const lit_t *lits, uint32_t count);

	
	bool equals(const RissClause& other) const;	
	uint32_t	get_size() const ;	
	void set_size(uint32_t s );	
	void	setStorage(uint32_t s);	
	lit_t	get_literal(uint32_t pos) const;	
	void set_literal(uint32_t pos, lit_t lit1);	
	RissClause	copy();	
	RissClause	copy_except(const var_t v);	
	void	add(lit_t lit1);	
	void	remove(lit_t lit1);	
	void  remove_index(uint32_t ind);	
	void	swap_literals(uint32_t pos1, uint32_t pos2);	
	bool contains_literal( const lit_t literal);

	
	float get_activity() const;	
	void set_activity( const float act);	
	void inc_activity( const float act);

	
	
	RissClause* resolve(const RissClause& other) const;	
	RissClause* resolve(const RissClause& other, var_t variable) const;

	
	bool check_resolve( const RissClause& other, var_t variable) const;	
	bool subsumes_or_simplifies_clause(const RissClause& other, lit_t *lit1) const;

	
	RissClause* resolveLin(const RissClause& other, var_t variable) const;	
	bool check_resolveLin( const RissClause& other, var_t variable) const;

	
	void sort();

	
	bool getFlag1() const ;
	void setFlag1(bool flag);

	bool getFlag2() const ;
	void setFlag2(bool flag);

	bool isXor() const ;
	void setXor(bool flag);

	bool is1ooN() const;
	void set1ooN(bool flag);

	bool isLearnt() const ;
	void setLearnt( bool l );

	
	void printInfo();} CL_PACK ;

	
inline void RissClause::printInfo(){
	std::cerr << "cl[" << std::hex << this << std::dec << "]: size()(" << get_size() << "), act(" << data.activity
			<< "), ext(" << get_size() - _CL_ELEMENTS << ") at " << std::hex << ext_lits << std::dec << ", locals:";
	for( uint32_t i = 0 ; i < _CL_ELEMENTS; ++i ){
		std::cerr << " " << nr( own_lits[i] );
	}
	std::cerr << " and externals: ";
	for( uint32_t i = _CL_ELEMENTS; i < get_size() ; ++i ){
		std::cerr << " " << nr( ext_lits[i-_CL_ELEMENTS] );
	}
	std::cerr << std::endl;
}

inline RissClause::RissClause(bool learnt)
{
	init();
	setStorage(0);
	setLearnt(learnt);
}

inline RissClause::RissClause(std::vector<lit_t>& lits, bool learnt)
{
	init();
	reset(lits);
	setLearnt(learnt);
}

inline RissClause::RissClause(const lit_t *lits, uint32_t count, bool learnt)
{
	init();
	reset(lits, count);
	setLearnt(learnt);
}

inline RissClause& RissClause::operator=(const RissClause& other)
{
	
	init();
	
	_sizedata = other._sizedata;
	
	set_size(0);
	
	setStorage( other.get_size() );
	assert( get_size() == other.get_size());
	if( get_size()>_CL_ELEMENTS){
		memcpy (own_lits, other.own_lits, _CL_ELEMENTS*sizeof(lit_t));
		memcpy (ext_lits, other.ext_lits, ( get_size()- _CL_ELEMENTS)*sizeof(lit_t));
	} else {
		memcpy (own_lits, other.own_lits, _CL_ELEMENTS*sizeof(lit_t));
	}
	data.activity = other.data.activity;
	TRACK_CHECK("copied to other clause");
	return *this;
}

inline bool RissClause::operator < (const RissClause& other) const {
	const uint32_t size = get_size();
	if( size > other.get_size() ) return false;
	if( size < other.get_size() ) return true;
	for( uint32_t i = 0 ; i < size; i++ ){
		if( get_literal(i) > other.get_literal(i) ) return false;
		if( get_literal(i) < other.get_literal(i) ) return true;
	}
	return false;
}

inline bool RissClause::operator >=(RissClause& other) const {
	const uint32_t size = get_size();
	if( size > other.get_size() ) return true;
	if( size < other.get_size() ) return false;
	for( uint32_t i = 0 ; i < size; i++ ){
		if( get_literal(i) > other.get_literal(i) ) return true;
		if( get_literal(i) < other.get_literal(i) ) return false;
	}
	return true;
}
	
inline bool RissClause::operator <=(RissClause& other) const {
	const uint32_t size = get_size();
	if( size > other.get_size() ) return false;
	if( size < other.get_size() ) return true;
	for( uint32_t i = 0 ; i < size; i++ ){
		if( get_literal(i) > other.get_literal(i) ) return false;
		if( get_literal(i) < other.get_literal(i) ) return true;
	}
	return true;
}

inline RissClause::~RissClause()
{
	
	
	TRACK_CHECK("destroyed");
	if(ext_lits != 0) FREE_SIZE(ext_lits,sizeof(lit_t) * (size() - _CL_ELEMENTS));
	ext_lits = 0;
	set_size( 0 );
	_sizedata = 0;
	set_activity( 0 );
}

inline void RissClause::reset(std::vector<lit_t>& lits)
{
	setStorage( lits.size() );
	

	for (uint32_t i=0; i<lits.size(); i++)
		set_literal(i, lits[i]);
	TRACK_CHECK("reset");
}

inline void RissClause::reset(const lit_t *lits, uint32_t count)
{
	setStorage(count);

	for (uint32_t i=0; i<count; i++)
		set_literal(i, lits[i]);
	TRACK_CHECK("reset");
}

inline bool RissClause::equals(const RissClause& other) const {
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

inline uint32_t RissClause::get_size() const
{
	return _sizedata & 0x7FFFFFF;	
}

inline void RissClause::set_size(uint32_t s ){
	_sizedata = (_sizedata & ~0x7FFFFFF) | (s & 0x7FFFFFF);	

	TRACK_CHECK("set size to " << s);
}

inline lit_t RissClause::get_literal(uint32_t pos) const
{

	STAT_READLIT(pos );

	
	lit_t l = (pos < _CL_ELEMENTS) ? own_lits[pos] : ext_lits[pos-_CL_ELEMENTS];
	return l;
}

inline void RissClause::set_literal(uint32_t pos, lit_t lit1)
{
	TRACK_CHECK("set literal at " << pos << " to " << nr(lit1) );
	STAT_WRITELIT( pos );
	(pos <_CL_ELEMENTS) ? own_lits[pos] = lit1 : ext_lits[pos-_CL_ELEMENTS] = lit1;

}

inline void RissClause::swap_literals(uint32_t pos1, uint32_t pos2)
{
	assert (pos1 < get_size());
	assert (pos2 < get_size());
	STAT_SWAPLITS( pos1, pos2 );
	
	lit_t tmp = get_literal(pos1);
	set_literal(pos1, get_literal(pos2));
	set_literal(pos2, tmp);
	TRACK_CHECK("swap literals " << pos1 << " and " << pos2);
}

inline void RissClause::add(lit_t lit1)
{
	setStorage(get_size()+1);
	set_literal(get_size()-1, lit1);

}

inline void	RissClause::remove(lit_t lit1)
{
	TRACK_CHECK("remove literal " << nr(lit1) );
	uint32_t i, j;

	for (i = 0, j = 0; i < get_size(); i ++) {
		if (get_literal(i) != lit1)
			set_literal(j++, get_literal(i));
	}
	setStorage(j);

}

inline void RissClause::remove_index(uint32_t ind)
{
	TRACK_CHECK("remove literal at index " << ind);
	swap_literals( ind, get_size() - 1 );
	setStorage( get_size() - 1 );

}

inline RissClause RissClause::copy_except(const var_t v){
	TRACK_CHECK("copy except variable " << v);
	const uint32_t sz = get_size()-1;
	lit_t tmp[sz];
	uint32_t j = 0;
	
	if( sz + 1 > _CL_ELEMENTS )
	{
		for(int i=0; i< (int)_CL_ELEMENTS; ++i){
			if( v != var(own_lits[i])) tmp[j++] = own_lits[i];
		}

		
		for (int i=0; i<(int)(get_size()-_CL_ELEMENTS); ++i)
			if( v != var(ext_lits[i])) tmp[j++] = ext_lits[i];
	} else {
		for(int i=0; i< (int)get_size(); ++i)
			if( v != var(own_lits[i])) tmp[j++] = own_lits[i];
	}

	
	RissClause cl(tmp, sz, get_activity());
	cl.setLearnt( isLearnt() );

	return cl;
}
inline RissClause RissClause::copy()
{
	const uint32_t sz = get_size();
	lit_t tmp[sz];
	uint32_t j = 0;
	
	if( sz > _CL_ELEMENTS )
	{
		for(int i=0; i< (int)_CL_ELEMENTS; ++i){
			tmp[j++] = own_lits[i];
		}
		
		for (int i=0; i<(int)(get_size()-_CL_ELEMENTS); ++i)
			tmp[j++] = ext_lits[i];
	} else {
		for(int i=0; i< (int)get_size(); ++i)
			tmp[j++] = own_lits[i];
	}

	
	RissClause cl(tmp, sz, get_activity());
	cl.setLearnt( isLearnt() );
	return cl;
}

inline void RissClause::setStorage(uint32_t new_lits)
{
	assert( sizeof(lit_t)==4 );

	if (get_size() != new_lits) {
		

		
		if (new_lits > _CL_ELEMENTS) {
			if(get_size() > _CL_ELEMENTS){
				
				#ifndef JAVALIB
				ext_lits = (lit_t *)REALLOC_SIZE(ext_lits,
																	sizeof(lit_t) * (new_lits - _CL_ELEMENTS),
																	sizeof(lit_t) * (get_size() - _CL_ELEMENTS) );
				#else
					lit_t* tmp = (lit_t*)MALLOC_SIZE( sizeof(lit_t) * (new_lits - _CL_ELEMENTS ) );
					if( tmp == 0 ) std::cerr << "c was not able to Mallocate memory for a new clause" << std::endl;
					memcpy( tmp, ext_lits, sizeof(lit_t) * (get_size() - _CL_ELEMENTS) );
					FREE_SIZE( ext_lits, sizeof(lit_t) * (get_size() - _CL_ELEMENTS) );
					ext_lits = tmp;
				#endif
			} else { 
				ext_lits = (lit_t *)MALLOC_SIZE(sizeof(lit_t) * (new_lits - _CL_ELEMENTS) );
				#ifdef JAVALIB
					if( ext_lits == 0 ) std::cerr << "c was not able to Mallocate memory for a new clause" << std::endl;
				#endif
				}
		} else {
			
			if (get_size() > _CL_ELEMENTS){
				
				FREE_SIZE(ext_lits,sizeof(lit_t) * (get_size() - _CL_ELEMENTS));
				ext_lits = 0;
			}
		}
		set_size( new_lits );

		
	}
	TRACK_CHECK("set storage to " << new_lits);

}

inline void RissClause::init()
{
	_sizedata = 0;
	ext_lits = 0;
	data.activity = 0;
	for (unsigned int i=0; i<_CL_ELEMENTS; ++i) own_lits[i]=NO_LIT;
}

inline float RissClause::get_activity() const
{
	return data.activity;
}

inline void RissClause::set_activity( const float activity)
{
	data.activity = activity;
}

inline void RissClause::inc_activity( const float act)
{
	data.activity += act;
}

inline RissClause* RissClause::resolve(const RissClause& other) const
{
	TRACK_CHECK("resolve");
	const uint32_t s = get_size();
	for( int32_t i = s - 1; i >=0; i--)
	{
		const uint32_t os = other.get_size();
		for( int32_t j = os - 1; j >=0; j--)
		{
			if( get_literal( i ) == inv_lit( other.get_literal( j ) ) )
			return resolve( other, var( get_literal( i ) ) );
		}
	}
 	return 0;
}

inline RissClause* RissClause::resolveLin(const RissClause& other, var_t variable) const
{
	const uint32_t bytes = (get_size() + other.get_size()) * sizeof(lit_t);

	lit_t lits[ bytes ];

	const RissClause& a = (*this);
	const RissClause& b = other;

	uint32_t i=0,j=0, sz=0;
	const uint32_t sa = a.get_size();
	const uint32_t sb = b.get_size();
	do {
		if( var(a.get_literal(i)) == variable ) {i++; continue; }
		if( var(b.get_literal(j)) == variable ) {j++; continue; }
		if( a.get_literal(i) == inv_lit(b.get_literal(j)) ) return 0;	
		lits[sz++] = (a.get_literal(i) < b.get_literal(j)) ? a.get_literal(i++) : ( (a.get_literal(i) > b.get_literal(j++)) ? b.get_literal(j) : a.get_literal(i++) );
	} while( i < sa && j < sb );

	for( ; i < sa; ++i ) lits[sz++] = a.get_literal(i);
	for( ; j < sb; ++j ) lits[sz++] = b.get_literal(j);

#ifdef RISSCLAUSE_DEBUG
	cerr << "c new resolvent from ";
	for( uint32_t k = 0 ; k < sa; ++k ) cerr << " " << nr( a.get_literal(k) );
	cerr << " and";
	for( uint32_t k = 0 ; k < sb; ++k ) cerr << " " << nr( b.get_literal(k) );
	cerr << " to";
	for( uint32_t k = 0 ; k < sz; ++k ) cerr << " " << nr( lits[k] );
	cerr << endl;
#endif

	
	return new RissClause(lits, sz, a.isLearnt() || b.isLearnt());
}

inline bool RissClause::check_resolveLin( const RissClause& other, var_t variable) const
{
	const RissClause& a = (*this);
	const RissClause& b = other;

	uint32_t i=0,j=0;
	const uint32_t sa = a.get_size();
	const uint32_t sb = b.get_size();

#ifdef RISSCLAUSE_DEBUG
	cerr << "c check for resolving ";
	for( uint32_t k = 0 ; k < sa; ++k ) cerr << " " << nr( a.get_literal(k) );
	cerr << " and";
	for( uint32_t k = 0 ; k < sb; ++k ) cerr << " " << nr( b.get_literal(k) );
	cerr << endl;
#endif

	do {
#ifdef RISSCLAUSE_DEBUG
	cerr << "c step: a[" << i << "]=" << a.get_literal(i) << " and b[" <<j << "]=" << b.get_literal(j) << endl;
#endif
		if( var(a.get_literal(i)) == variable ) {i++; break; }
		if( var(b.get_literal(j)) == variable ) {j++; continue; }
		if( a.get_literal(i) == inv_lit(b.get_literal(j)) ) return false;	
		lit_t c = (a.get_literal(i) < b.get_literal(j)) ? a.get_literal(i++) : ( (a.get_literal(i) > b.get_literal(j++)) ? b.get_literal(j) : a.get_literal(i++) );
	} while( i < sa && j < sb );
	if( i < sa && j < sb ){
		do {
		#ifdef RISSCLAUSE_DEBUG
			cerr << "c step: a[" << i << "]=" << a.get_literal(i) << " and b[" <<j << "]=" << b.get_literal(j) << endl;
		#endif
	
			if( var(b.get_literal(j)) == variable ) {j++; continue; }
			if( a.get_literal(i) == inv_lit(b.get_literal(j)) ) return false;	
			lit_t c = (a.get_literal(i) < b.get_literal(j)) ? a.get_literal(i++) : ( (a.get_literal(i) > b.get_literal(j++)) ? b.get_literal(j) : a.get_literal(i++) );
		} while( i < sa && j < sb );
	}
	return true;
}


inline RissClause* RissClause::resolve(const RissClause& other, var_t variable) const
{
	var_t var11, var12;
	lit_t lit11, lit12;
	uint32_t j, sz;

	uint32_t bytes = (get_size() + other.get_size())  * sizeof(lit_t);

	lit_t new_lits[ bytes ];
	sz = 0;

	const RissClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const RissClause& cls2 = (get_size() < other.get_size()) ? other : (*this);

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

	
	return new RissClause(new_lits, sz, cls1.isLearnt() || cls2.isLearnt() );
}

inline bool RissClause::check_resolve( const RissClause& other, var_t variable) const
{
	TRACK_CHECK("check resolve");
	var_t var11, var12;
	lit_t lit11, lit12;

	const RissClause& cls1 = (get_size() < other.get_size()) ? (*this) : other;
	const RissClause& cls2 = (get_size() < other.get_size()) ? other : (*this);

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

inline bool RissClause::contains_literal( const lit_t literal)
{
	TRACK_CHECK("contain literal " << nr(literal));
	for (uint32_t i = 0; i < get_size(); i++)
	{
		if( get_literal(i) == literal ) return true;
	}
	return false;
}


inline bool RissClause::subsumes_or_simplifies_clause(const RissClause& other, lit_t *lit1) const
{
	TRACK_CHECK("check subsume simplify");
	
	const uint32_t other_size = other.get_size();
	const uint32_t size = get_size();
	if ( size > other_size) return false;

	*lit1 = NO_LIT;

	
	for (uint32_t i=0; i<size; i++) {
		uint32_t j=0;
		for (; j<other_size; j++) {
			const lit_t otherJ = other.get_literal(j);
			
			if (get_literal(i) == otherJ) break;
			
			if ((*lit1 == NO_LIT )&& (get_literal(i) == inv_lit(otherJ))) {
				*lit1 = otherJ;
				break;
			}
			
			if ((*lit1 != NO_LIT )&& (get_literal(i) == inv_lit(otherJ)))
			  return false;
		}
		
		if (j==other_size) return false;
	}

	
	return true;
}


inline void RissClause::sort(){
	const uint32_t size = get_size()-1;
	for( uint32_t i = 0 ; i < size; ++i ){
		uint32_t p = i;
		lit_t l = get_literal(p);
		for( uint32_t j = i + 1; j<=size; ++j ){
			const lit_t l2 = get_literal(j);
			if( l > l2 ){ p = j; l = l2; }
		}
		swap_literals(i, p);
	}
}

inline bool RissClause::getFlag1() const { 
	TRACK_CHECK("get flag 1");
	return _sizedata & 0x80000000;
}

inline void RissClause::setFlag1(bool flag){ 
	TRACK_CHECK("set flag 1");
	 _sizedata = (_sizedata & ~0x80000000) | ((uint32_t)flag << 31); 
}


inline bool RissClause::getFlag2() const { 
	TRACK_CHECK("get flag 2");
	 return _sizedata & 0x40000000; 
}

inline void RissClause::setFlag2(bool flag){ 
	TRACK_CHECK("set flag 2");
	 _sizedata = (_sizedata & ~0x40000000) | ((uint32_t)flag << 30); 
}

inline bool RissClause::isXor() const { 
	 return _sizedata & 0x20000000; 
}

inline void RissClause::setXor(bool flag){ 
	 _sizedata = (_sizedata & ~0x20000000) | ((uint32_t)flag << 29); 
}

inline bool RissClause::is1ooN() const { 
	return _sizedata & 0x10000000; 
}

inline void RissClause::set1ooN(bool flag){ 
	 _sizedata = (_sizedata & ~0x10000000) | ((uint32_t)flag << 28); 
}

inline bool RissClause::isLearnt() const { 
	TRACK_CHECK("get flag learnt");
	 return _sizedata & 0x8000000; 
}

inline void RissClause::setLearnt( bool l ){ 
	TRACK_CHECK("set flag learnt to " << l);
	 _sizedata = (_sizedata & ~0x8000000) | ((uint32_t)l << 27); 
}


#endif 
