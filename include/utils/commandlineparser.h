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


#ifndef _COMMANDLINEPARSER_H
#define _COMMANDLINEPARSER_H


#include <iostream>
#include <string>
#include <fstream>
#include "utils/stringmap.h"

using namespace std;


class CommandLineParser
{
	StringMap map;

	void print_sat_help();
	void print_csp_help();
public:
	StringMap parse(int32_t argc, char* argv[], bool binary=true);
	
	bool parseFile( string filename, StringMap& thisMap );

};



#endif
