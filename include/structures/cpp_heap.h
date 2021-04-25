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


#ifndef _WEIGHTED_HEAP_H
#define _WEIGHTED_HEAP_H

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <vector>

template<class Key, class Compare> 
class cpp_heap
{
	private:
		Compare				comp;
	
		std::vector<unsigned>	sorting;
		std::vector<unsigned>	indices;
		Key					**values;
		unsigned			max_value;

		void update(unsigned item, bool ins, bool rem);
		void swap_items(unsigned item1, unsigned item2);

	public:
		cpp_heap(Key **values_ref, unsigned size);
		cpp_heap() : values(0), max_value(0) {}

		bool is_empty();
		void clear();

		void insert_item(unsigned item);
		void update_item(unsigned item);
		void remove_item(unsigned item);

		unsigned size();
		unsigned item_at(unsigned ind) const ;
		bool contains(unsigned item);
		
		
		void resize(unsigned int newElements,Key** newKeys);

		std::vector<unsigned>::const_iterator begin();
		std::vector<unsigned>::const_iterator end();
};









template<class Key, class Compare> 
inline unsigned cpp_heap<Key, Compare>::size()
{
	return sorting.size();
}

template<class Key, class Compare> 
inline void cpp_heap<Key, Compare>::resize(unsigned int newElements,Key** newKeys){


	values = newKeys;
	sorting.reserve( newElements );
	const uint32_t oldSize = indices.size();
	
	
	max_value = newElements;

	sorting.reserve(max_value);
	
	
	if( newElements > oldSize ){
		indices.insert( indices.end(), newElements - oldSize + 1, UINT_MAX );
	}
}

template<class Key, class Compare> 
inline bool cpp_heap<Key, Compare>::is_empty()
{
	return sorting.empty();
}

template<class Key, class Compare> 
inline unsigned cpp_heap<Key, Compare>::item_at(unsigned ind) const 
{
	
	assert(ind < sorting.size());

	return sorting[ind];
}

template<class Key, class Compare> 
inline bool cpp_heap<Key, Compare>::contains(unsigned item)
{
	return (indices[item] < sorting.size());
}

#define CPPPARENT(___ind)	(((___ind) - 1) / 2)
#define CPPCHILD_LEFT(___ind)		(2 * (___ind) + 1)
#define CPPCHILD_RIGHT(___ind)		(2 * (___ind) + 2)

#define CPPCOMPARE(___ind1, ___ind2) \
							(comp((*values)[sorting[___ind1]], (*values)[sorting[___ind2]]))

template<class Key, class Compare> 
cpp_heap<Key, Compare>::cpp_heap(Key **values_ref, unsigned size)
{
	values = values_ref;
	max_value = size;

	sorting.reserve(max_value);
	indices.assign(max_value, UINT_MAX);

}

template<class Key, class Compare> 
void cpp_heap<Key, Compare>::update(unsigned item, bool ins, bool rem)
{

	unsigned ind, size, max, old_item;

	
	if (ins && !rem) {
		
		indices[item] = sorting.size();
		sorting.push_back(item);
	}
	
	if (rem && !ins) {
		old_item = item;
		ind = indices[item];
		
		item = sorting[sorting.size() - 1];
		swap_items(ind, sorting.size() - 1);
		
		sorting.pop_back();
		indices[old_item] = UINT_MAX;

		
		if( item == old_item ) return;
	}

	
	ind = indices[item];
	
	size = sorting.size();
	
	
	if (size == 0)
		return;

	assert(CPPCHILD_LEFT(CPPPARENT(ind)) == ind || CPPCHILD_RIGHT(CPPPARENT(ind)) == ind);
	
	while (ind > 0 && 
		comp((*values)[sorting[ ind ]], (*values)[sorting[ CPPPARENT(ind) ]]) )
	
	
	{
		swap_items(ind, CPPPARENT(ind));
		ind = CPPPARENT(ind);
	}

	assert (ind==0 || !CPPCOMPARE(ind, CPPPARENT(ind)));

	
	while (1) {
		if (CPPCHILD_LEFT(ind) < size)
			max = CPPCHILD_LEFT(ind);
		else
			break;

		if (CPPCHILD_RIGHT(ind) < size && CPPCOMPARE(CPPCHILD_RIGHT(ind), CPPCHILD_LEFT(ind)))
			max = CPPCHILD_RIGHT(ind);

		if (!CPPCOMPARE(max, ind))
			break;

		swap_items(ind, max);
		ind = max;
	}
	
	assert (ind==0 || !CPPCOMPARE(ind, CPPPARENT(ind)));
	assert (CPPCHILD_LEFT(ind) >= size || !CPPCOMPARE(CPPCHILD_LEFT(ind), ind));
	assert (CPPCHILD_RIGHT(ind) >= size || !CPPCOMPARE(CPPCHILD_RIGHT(ind), ind));
}

template<class Key, class Compare> 
inline void cpp_heap<Key, Compare>::swap_items(unsigned ind1, unsigned ind2)
{
	unsigned tmp;

	
	tmp = sorting[ind1];
	sorting[ind1] = sorting[ind2];
	sorting[ind2] = tmp;

	
	tmp = indices[sorting[ind1]];
	indices[sorting[ind1]] = indices[sorting[ind2]];
	indices[sorting[ind2]] = tmp;
}

template<class Key, class Compare> 
void cpp_heap<Key, Compare>::clear()
{
	sorting.clear();
	indices.assign(max_value, UINT_MAX);
}

template<class Key, class Compare> 
void cpp_heap<Key, Compare>::insert_item(unsigned item)
{
	update(item, true, false);
}

template<class Key, class Compare> 
void cpp_heap<Key, Compare>::update_item(unsigned item)
{
	update(item, true, true);
}

template<class Key, class Compare> 
void cpp_heap<Key, Compare>::remove_item(unsigned item)
{
	update(item, false, true);
}

template<class Key, class Compare> 
std::vector<unsigned>::const_iterator cpp_heap<Key, Compare>::begin()
{
	return sorting.begin();
}

template<class Key, class Compare> 
std::vector<unsigned>::const_iterator cpp_heap<Key, Compare>::end()
{
	return sorting.end();
}

#endif

