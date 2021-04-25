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


#ifndef _STRINGMAP_H
#define _STRINGMAP_H

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <sstream>


#include <inttypes.h>


class StringMap{
public:
	struct stringmapentry{
		std::string key;
		std::string value;
	};

private:
	std::vector< stringmapentry > stringmap;
public:
	StringMap();
	~StringMap();

	stringmapentry operator[](uint32_t ind) const {
		return stringmap[ind];
	}

	void insert( char* key, char* value );
	bool contains( const char* key ) const;
	void clear();
	std::string get( const char* key ) const;
	std::string get_key( uint32_t ind ) const;
	std::string get_value( uint32_t ind ) const;
	
	bool remove( const char* key );
	
	uint32_t size() const;
	std::string toString() const ;
};




inline StringMap::StringMap()
{
}

inline  StringMap::~StringMap()
{
}


inline void StringMap::insert( char* key, char* value )
{
	const uint32_t mapsize = stringmap.size();
	for(uint32_t i = 0 ; i<mapsize; i++ )
	{
		if( stringmap[i].key.compare( key ) == 0 )	
		{
			if( value != 0 )	
			{
				stringmap[i].value = value;
			} else {
				stringmap[i].value = "";
			}
			return;
		}
	}
	
	stringmapentry entry;
	entry.key = std::string( key );
	if( value != 0 )	
	{
		entry.value = value;
	} else {
		entry.value = "";
	}
	
	stringmap.push_back( entry );
}


inline bool StringMap::contains( const char* key ) const
{
	const uint32_t mapsize = stringmap.size();
	for(uint32_t i = 0 ; i<mapsize; i++ ){
		if( stringmap[i].key.compare( key ) == 0 ) return true;
	}
	return false;
}


inline std::string StringMap::get( const char* key ) const
{
	const uint32_t size = stringmap.size();
	for(uint32_t i = 0 ; i<size; i++ )
	{
		if( stringmap[i].key.compare( key ) == 0 ){
			if( stringmap[i].value.size() == 0 ) return std::string();
			return stringmap[i].value;
		}
	}
	return std::string();	
}

inline std::string StringMap::get_key( uint32_t ind ) const
{
	assert( size() > ind );
	return stringmap[ind].key;
}

inline std::string StringMap::get_value( uint32_t ind ) const
{
	assert( size() > ind );
	return stringmap[ind].value;
}

inline void StringMap::clear()
{
	stringmap.clear();
}

inline bool StringMap::remove( const char* key )
{
	for( uint32_t i = 0 ; i < stringmap.size(); ++i ){
		if( stringmap[i].key.compare( key ) == 0 ){
			stringmap.erase( stringmap.begin() + i );
			return true;
		}
	}
	return false;
}

inline uint32_t StringMap::size() const
{
	return stringmap.size();
}

inline std::string StringMap::toString() const {
	std::stringstream stream;
	const uint32_t size = stringmap.size();
	for(uint32_t i = 0 ; i<size; i++ )
	{
		if( stringmap[i].key.compare("file") == 0 ) continue;
		stream << stringmap[i].key;
		if( stringmap[i].value.size() == 0 ){
			stream << "|";
		} else {
			stream << " " << stringmap[i].value << "|";
		}
	}
	
	if( contains( "file" ) ){
		stream << "file " << get("file") << "|";
	}
	return stream.str();
}

#endif
