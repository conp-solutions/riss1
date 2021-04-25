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


#ifndef _CPP_VEC_H
#define _CPP_VEC_H

#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <assert.h>

#include "macros/malloc_macros.h"



#define CPP_OWN_ELEMENTS (uint32_t) (( 64 - sizeof(T*) - sizeof(uint32_t) - sizeof(uint32_t) ) / sizeof(T))

template <typename T>
class CppVector {
	T privData[ CPP_OWN_ELEMENTS ];	
	T* data;	
	uint32_t _size;	
	uint32_t _cap;	

public:
	CppVector();
	
	CppVector( const uint32_t elements );
	
	CppVector( const CppVector& other );
	
	~CppVector();
	
	T& operator [] (const uint32_t index);
	
	const T& operator [] (const uint32_t index) const ;

	CppVector& operator=(const CppVector& other);
	
	void assign( const CppVector& src);

	void destroy();

	void resize( const uint32_t elements, const T& new_element );

	
	void reserve( const uint32_t elements );

	void reserve_another( const uint32_t more_elements );

	void push_back( const T& element );

	void push_back_another(const T* elements, const uint32_t number );
	
	void push_back_another(const CppVector<T>& elements, const uint32_t number );

	void pop_back();

	void erase(const uint32_t ind );

	void erase_no_order(const uint32_t ind );

	uint32_t size() const;

	uint32_t capacity() const;

	void clear();

	void minimize();

	void delete_first_elements(const uint32_t elements );

	void delete_last_elements(const uint32_t elements );
};



template <typename T>
inline 
CppVector<T>::CppVector()
 : data(0), _size(0), _cap(CPP_OWN_ELEMENTS)
{}

	
template <typename T>
inline 
CppVector<T>::CppVector( uint32_t elements )
 : data(0), _size(0), _cap(CPP_OWN_ELEMENTS)
{
	reserve( elements );
}
	
template <typename T>
inline 
CppVector<T>::CppVector( const CppVector& other )
: data(0), _size(0), _cap(CPP_OWN_ELEMENTS)
{
	reserve( other.size() + 1);
	memcpy( privData, other.privData, CPP_OWN_ELEMENTS * sizeof(T) );
	if( _cap > CPP_OWN_ELEMENTS ) memcpy( data, other.data,  (_size - CPP_OWN_ELEMENTS) * sizeof(T) );
}

template <typename T>
inline CppVector<T>::~CppVector(){
	if( data != 0 ) free(data);
}

template <typename T>
inline CppVector<T>& CppVector<T>::operator=(const CppVector& other){

	reserve( other.size() + 1);	
	memcpy( privData, other.privData, CPP_OWN_ELEMENTS * sizeof(T) );
	if( other.size() > CPP_OWN_ELEMENTS )
		memcpy( data, other.data,  (other.size() - CPP_OWN_ELEMENTS) * sizeof(T) );
	_size = other.size();
	return *this;
}
	
template <typename T>
inline T& CppVector<T>::operator [] (const uint32_t index){
	return (index < CPP_OWN_ELEMENTS) ? privData[index] : data[index - CPP_OWN_ELEMENTS];
}

template <typename T>
inline const T& CppVector<T>::operator [] (const uint32_t index) const {
	if (index < CPP_OWN_ELEMENTS) return privData[index];
	else return data[index - CPP_OWN_ELEMENTS];
}
	
template <typename T>
inline void CppVector<T>::assign( const CppVector& other){
	reserve( other.size() );	
	
	
	memcpy( privData, other.privData, CPP_OWN_ELEMENTS * sizeof(T) );
	if( _cap > CPP_OWN_ELEMENTS ) memcpy( data, other.data,  (_size - CPP_OWN_ELEMENTS) * sizeof(T) );
}

template <typename T>
inline void CppVector<T>::destroy(){
	if( data != 0 ) free( data );
	_size = 0; _cap = 0;
}

template <typename T>
inline void CppVector<T>::resize(const uint32_t elements, const T& new_element ){
	
	if( size() >= elements ){
		_size = elements;
		
	}
	
	
	reserve( elements );
	
	
	for( uint32_t i = _size; i < elements; ++i ) (*this)[i] = new_element;

}

template <typename T>
inline void CppVector<T>::reserve( const uint32_t elements ){
	assert( elements < 300000 );
	
	if( (elements < _cap || elements < CPP_OWN_ELEMENTS) ) return;

	
	data = data == 0 ? (T*) MALLOC( (elements-CPP_OWN_ELEMENTS) * sizeof(T) ) : (T*) realloc( data, (elements-CPP_OWN_ELEMENTS) * sizeof(T) );
	_cap = elements;
}

template <typename T>
inline void CppVector<T>::reserve_another( const uint32_t more_elements ){
	reserve( size() + more_elements );
}

template <typename T>
inline void CppVector<T>::push_back( const T& element ){
	if( size() + 1 >= capacity() ){
		
		reserve( (1+size()) << 1 );
	}
	
	(*this)[size()] = element;
	_size ++;
}

template <typename T>
inline void CppVector<T>::push_back_another(const T* elements, const uint32_t number ){
	if( capacity() < size() + number ){
		
		reserve_another( number );
	}
	
	if( number + size() >= CPP_OWN_ELEMENTS ){
		
		const uint32_t diff = CPP_OWN_ELEMENTS - size();
		
		memcpy( &data[ size() ], &elements[diff], (number-diff) * sizeof(T) );
		
		memcpy( &privData[size()], elements, diff*sizeof(T) );
	} else {
		
		memcpy( &privData[size()], elements, number*sizeof(T) );
	}
}

template <typename T>
inline void CppVector<T>::push_back_another(const CppVector<T>& elements, const uint32_t number ){
	if( number > CPP_OWN_ELEMENTS ){
		T ele[number];
		
		memcpy( ele, elements.privData, CPP_OWN_ELEMENTS*sizeof(T) );
		
		memcpy( &ele[CPP_OWN_ELEMENTS], elements.data, (number-CPP_OWN_ELEMENTS)*sizeof(T) );
		
		push_back_another( ele, number );
	} else {
		
		push_back_another( elements.privData, number );
	}
}

template <typename T>
inline void CppVector<T>::pop_back(){
	assert ( size() == 0 );
	_size = _size - 1;
}

template <typename T>
inline void CppVector<T>::erase(const uint32_t ind ){
	assert ( size() == 0 );
	
	if ( ind < CPP_OWN_ELEMENTS ){
		for(uint32_t i = ind; i < CPP_OWN_ELEMENTS - 1; i++ ){
			privData[i] = privData[i+1];
		}
		if( data != 0 ) privData[CPP_OWN_ELEMENTS - 1] = data[0];
	}
	
	const uint32_t max = size() - 1 - CPP_OWN_ELEMENTS;
	for(uint32_t i = ind - CPP_OWN_ELEMENTS; i < max; i++ ){
		data[i] = data[ i + 1 ];
	}
	
	_size = (_size > 0 ) ? _size - 1 : 0;
}

template <typename T>
inline void CppVector<T>::erase_no_order(const uint32_t ind ){
	assert ( size() == 0 );
	
	(*this)[ind] = (*this)[size()-1];
	
	_size = (_size > 0 ) ? _size - 1 : 0;
}

template <typename T>
inline uint32_t CppVector<T>::size() const{
	return _size;
}

template <typename T>
inline uint32_t CppVector<T>::capacity() const{
	return _cap;
}

template <typename T>
inline void CppVector<T>::clear(){
	_size = 0;
}

template <typename T>
inline void CppVector<T>::minimize(){
	const uint32_t elements = size();
	
	if( elements < CPP_OWN_ELEMENTS || elements == _cap ) return;

	
	data = data == 0 ? (T*) MALLOC( (elements-CPP_OWN_ELEMENTS) * sizeof(T) ) : (T*) realloc( data, (elements-CPP_OWN_ELEMENTS) * sizeof(T) );
	_cap = elements < CPP_OWN_ELEMENTS ? CPP_OWN_ELEMENTS : elements;
}

template <typename T>
inline void CppVector<T>::delete_first_elements(const uint32_t elements ){
	assert( size() >= elements );
	const uint32_t s = size();

	uint32_t i = 0; 

	while( i < CPP_OWN_ELEMENTS && i + elements < size() ){
		(*this)[i] = (*this)[i+elements];
		++i;
	}

	const uint32_t reducedSize = size() - CPP_OWN_ELEMENTS;
	if( i + elements < reducedSize ){
		i = 0;	
		const uint32_t gap = elements ;
		while( i + (gap << 1) < reducedSize ){	
			memcpy( data + i, data + i + gap, sizeof(T) * gap );
			i+= gap;
		}
		
		std::cerr << " i=" << i <<
		" size=" << size() <<
		" gap=" << gap <<
		" own=" << CPP_OWN_ELEMENTS <<
		" reducedSize=" << reducedSize <<
		 std::endl;
		
		memcpy( data + i, data + i + gap, sizeof(T) * (reducedSize - i + gap) );
	}
	_size -= elements;
}

template <typename T>
inline void CppVector<T>::delete_last_elements(const uint32_t elements ){
	_size = _size - elements;
}

#endif
