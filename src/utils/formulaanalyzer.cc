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


#include "utils/formulaanalyzer.h"

void FormulaAnalyzer::analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment  )
{
	uint32_t* sizecount = new uint32_t[detaillevel+2];
	
	for( uint32_t i = 0; i < detaillevel + 2; ++i ) sizecount[i] = 0;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, (*clause_set) ); ++i )
	{
		
		if( CL_SIZE( gsa.get( (*clause_set)[i] ) ) <= detaillevel ) sizecount[ CL_SIZE(gsa.get( (*clause_set)[i] ) )]++;
		else sizecount[ detaillevel + 1 ]++;
		
		
	}
	
	std::string filename = "formula.csv";
	std::fstream file;
	
	
	
	if( add ) file.open(filename.c_str(), std::ios::app | std::ios_base::out);
	else  file.open(filename.c_str(), std::ios::trunc | std::ios_base::out);
	
	std::cerr << "c write clause sizes to formula.csv" << std::endl;
	
	for( uint32_t i = 0; i < detaillevel + 2; ++i ) file << sizecount[i] << ";";
	file << std::endl;
	
	file.close();
	
	delete sizecount;
}
