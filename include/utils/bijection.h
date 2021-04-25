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


#ifndef _BIJECTION_H
#define _BIJECTION_H


#include <inttypes.h>
#include <vector>


template< typename KEY, typename VALUE >
class bijection{
	bool keyIndex;	
	std::vector<KEY> k;	
	std::vector<VALUE> v;	

public:
	bijection(): keyIndex(true) {};
	
	bool isKeyOrdered(){ return keyIndex; }	

	uint32_t size(){ return k.size(); }
	void clear(){ k.clear(); v.clear(); }

	
	bool insert(const KEY& key, const VALUE& value){
		if( containsKey(key) ) return false;
		
		if( k.size() == 0 || key > k.back() ) {
			k.push_back(key);
			v.push_back(value);
			return true;	
		}

		uint32_t m = findPos<KEY>(key, k);

		k.insert( k.begin() + m, key );
		v.insert( v.begin() + m, value );
		return true;
	}
	
	bool containsKey( const KEY& key )
	{
		if( k.size() == 0 ) return false;
		if( key > k.back() ) return false;
		if( key < *k.begin() ) return false;
		uint32_t m = findPos<KEY>(key, k);
		return k[m] == key;
	}
	
	bool containsValue( const VALUE& value )
	{
		if( v.size() == 0 ) return false;
		if( value > v.bacv() ) return false;
		if( value < *v.begin() ) return false;
		uint32_t m = findPos<VALUE>(value, v);
		return v[m] == value;
	}
	
	const VALUE& valueAt(uint32_t i){ return v[i]; }
	const VALUE& getValue(const KEY& key )
	{
		if( key == k.back() ) return v.back();
		if( key == *k.begin() ) return *v.begin();
		uint32_t m = findPos<KEY>(key, k);
		return v[m];
	}

	const KEY& keyAt(uint32_t i){ return k[i]; }
	const KEY& getKey(const VALUE& value)
	{
		if( value == v.back() ) return k.back();
		if( value == *v.begin()) return *k.begin();
	
		uint32_t m = findPos<VALUE>(value, v);
		return k[m];
	}

	void swapIndex(){
		keyIndex = !keyIndex;
		if( keyIndex )
			doubesort<KEY,VALUE>( &(k[0]), &(v[0]), k.size() );
		else
			doubesort<VALUE,KEY>( &(v[0]), &(k[0]), k.size() );
	}

private:
	template< typename A, typename B >
	void doubesort( A* a, B* b, uint32_t size )
	{
		
		A cA;	B cB;
		for( uint32_t i = 0; i < size; ++i ){
			uint32_t p = i;
			
			for( uint32_t j = i+1; j < size; ++j ){
				if( a[j] < a[p] ){
					p = j;
				}
			}
			cA = a[i];
			a[i] = a[p];
			a[p] = cA;
			
			cB = b[i];
			b[i] = b[p];
			b[p] = cB;
			
		}
	
	}
	
	template<typename C>
	uint32_t findPos(const C& key, std::vector<C>& c)
	{	
		if( key == c[0] ) return 0;
		uint32_t l = 0;
		uint32_t u = c.size() - 1;
		uint32_t m = (u + l + 1)/2;
		
		do{
			if( key > c[m] ){
				l = m + 1;
				m = (u + l)/2;
			} else {
				u = m;
				m = (u + l + 1)/2;
			}
		}while( m<u );
		return m;
	}

};


#endif
