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


#ifndef _VARFILEPARSER_H
#define _VARFILEPARSER_H

#include <iostream>

#include <fstream>
#include <sstream>
#include <string>

#include <inttypes.h>
#include <cstdlib>
#include <cstring>

#include "types.h"
#include "macros/vector_macros.h"

using namespace std;


class VarFileParser{
	std::fstream file;
public:
	
	VarFileParser( const string& filename);

	
	void extract ( VEC_TYPE(var_t)& vars );
};



;

inline void VarFileParser::extract ( VEC_TYPE(var_t)& vars ){
	file.seekg( 0 );
	std::string line;
	uint32_t lines = 0;
	uint32_t var_cnt = 0;
	while(getline (file, line))
	{
		if( line.size() == 0 ) continue;
		
		if( line.at(0) == 'c' ) continue;
		if( line.find('.') != string::npos ){
			
			uint32_t dotPos = line.find('.');
			std::string first = line.substr(0, line.find('.'));
			var_t firstVar = atoi(first.c_str());

			line = line.substr(dotPos + 2);
			var_t secondVar = atoi(line.c_str());
			for( var_t v = firstVar; v <= secondVar; v++) VEC_PUSH_BACK(var_t, vars, v );
		} else {
			
			VEC_PUSH_BACK(var_t, vars, atoi( line.c_str() ) );
		}
	}
}


inline VarFileParser::VarFileParser( const string& filename)
{
	file.open(filename.c_str(), std::ios::app | std::ios_base::in );
}

#endif
