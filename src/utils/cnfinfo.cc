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


#include "utils/cnfinfo.h"

CnfInfo::CnfInfo(const StringMap& cm){
	level = 0;
	noSearch = false;
	if( cm.contains( (const char*)"info_level" ) ) level = atoi( cm.get( (const char*)"info_level" ).c_str() );
	if( cm.contains( (const char*)"info_exit" ) ) noSearch = (0 != atoi( cm.get( (const char*)"info_exit" ).c_str() ));
}

CnfInfo::~CnfInfo(){

}

void CnfInfo::checkXOR( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const assi_t assignment ){

	


}

void CnfInfo::analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const assi_t assignment  )
{
	if( clause_set == 0 ){
		std::cerr << "c no clauses in the formula" << std::endl;
		if( noSearch ) exit(0);
		else return;
	}

	unsigned char varArray[var_cnt+1];
	memset(varArray,0, sizeof(char) * (var_cnt+1) );

	double clauses = VEC_SIZE( CL_REF , (*clause_set) );

	double sizeSum = 0;
	double maxSize = 0;
	double positive = 0, negative = 0;
	for( uint32_t i = 0; i < clauses; i++ ){
		const CLAUSE& cl = gsa.get ( (*clause_set)[i] );
		sizeSum += CL_SIZE(cl);
		maxSize = maxSize > CL_SIZE(cl) ? maxSize : CL_SIZE(cl);
		for( uint32_t j = 0; j < CL_SIZE(cl); j++ ){
			const lit_t l = CL_GET_LIT(cl,j);
			const var_t v = var( l );
			varArray[ v ] += ( varArray[v] == 255 ) ? 0 : 1;
			if( pol(l) == POS ) positive ++;
			else negative++;
		}
	
	}

	double presentV = 0;
	double max = 0;
	double min = 255;
	for( var_t v = 1; v <= var_cnt; v++ ) {
		if( varArray[v] > 0 ){
			presentV ++;
			max = max >= varArray[v] ? max : varArray[v];
			min = min <= varArray[v] ? min : varArray[v];
		}
	
	}

	cerr << "c ====== basic cnf info =======" << endl;
	cerr << "c info clauses " << clauses << endl;
	cerr << "c info vars " << var_cnt << endl;
	cerr << "c info presentVars " << presentV << endl;
	cerr << "c info avgClsSize " << sizeSum / clauses << endl;
	cerr << "c info maxClSize " << maxSize << endl;
	cerr << "c info positiveRatio " << positive / sizeSum << endl;
	cerr << "c info negativeRatio " << negative / sizeSum << endl;
	cerr << "c info maxOcc " << max << endl;
	cerr << "c info minOcc " << min << endl;
	cerr << "c info avgClsPerVar " << clauses / presentV << endl;
	cerr << "c =============================" << endl;
	
	if( level > 0 )
	{
		FormulaAnalyzer f;
		std::cerr << "c analyze preprocessed formula" << std::endl;
		f.analyze( clause_set, var_cnt, true, assignment );
	}
	if( level > 1 )
	{
		FormulaLitSCC f;
		std::cerr << "c find LitSCC in preprocessed formula" << std::endl;
		f.analyze( clause_set, var_cnt, true, assignment );
	}
	if( level > 2 )
	{
		FormulaSCCFinder f;
		std::cerr << "c find SCC in preprocessed formula" << std::endl;
		f.analyze( clause_set, var_cnt, true, assignment );
	}
	if( level > 3 )
	{
		FormulaCsps f;
		std::cerr << "c analyze formula csps" << std::endl;
		f.analyze( clause_set, var_cnt, true, assignment );
	}
	
	if( noSearch ) exit(0);
	else return;
}

