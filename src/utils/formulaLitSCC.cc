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


#include "utils/formulaLitSCC.h"



FormulaLitSCC::FormulaLitSCC()
{
	ind = 0;
	
	
	
}

void FormulaLitSCC::analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment )
{
	std::cerr << "c ========================================" << std::endl;
	std::cerr << "c LitSCC analyzer:" << std::endl;
	std::cerr << "c build lit1 graph" << std::endl;
	
	
	graph = std::vector< std::set<lit_t> >( max_index( var_cnt ) );
	const VEC_TYPE( CL_REF )& clauses = *clause_set;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses); ++i ){
		const CLAUSE clause = clauses[i];
		
		if( CL_SIZE( clause ) > 2 ) continue;
		
		for( uint32_t j = 0 ; j < CL_SIZE( clause ); ++j ){
			const var_t var1J = var( CL_GET_LIT( clause, j ) );
			if( assi_variable_equal( assignment, var1J, pol(CL_GET_LIT( clause, j )) ) ) continue;
		}
		
		
		graph[  index( inv_lit(CL_GET_LIT( clause, 0 ))) ].insert( index(CL_GET_LIT( clause, 1 )) );
		graph[  index( inv_lit(CL_GET_LIT( clause, 1 ))) ].insert( index(CL_GET_LIT( clause, 0 )) );
		
	}
	
	
	
	
	nodeindes.resize( max_index( var_cnt ), -1 );
	nodelowlinks.resize( max_index( var_cnt ), -1 );
	inStack.resize( max_index( var_cnt ), 0 );
	inSCC.resize( max_index( var_cnt ), 0 );
	
	replaceWith.resize( max_index( var_cnt ), 0 );

	
	for( lit_t l = 0; l < max_index( var_cnt ); ++l )
	{
		if( graph[l].size() == 0 ) continue;	
		if( inSCC[l] != 0 ) continue; 

		
		std::vector< lit_t > component;
		component = tarjan(l,l);
		
		if( component.size() > 1 ) SCC.push_back(component);
	}
	
	
	
	std::cerr << "c found " << SCC.size() << " implication SCCs" << std::endl;



	
	for( uint32_t i = 0 ; i < SCC.size(); ++i )
	{
		for( uint32_t j = 0 ; j < SCC[i].size()-1; ++j ){
			replaceWith[ SCC[i][j] ] = SCC[i][ SCC[i].size() - 1 ];
		}
	}


	
	
	
	
	

}

std::vector< lit_t> FormulaLitSCC::tarjan(lit_t v, lit_t list)
{
	std::vector< lit_t > component;

    nodeindes[v] = ind;
    nodelowlinks[v] = ind;
    ind++;
    stack.push_back(v);
    inStack[v] = 1;
       
    std::set<lit_t>::iterator iter;

	for(iter = graph[list].begin(); iter != graph[list].end(); iter ++)
       {
           lit_t n = *iter;
           if(nodeindes[n] == -1){
               tarjan(n, n);
               nodelowlinks[v] = MINIM( nodelowlinks[v], nodelowlinks[n]);
           } else if(inStack[n] == 1){
               nodelowlinks[v] = MINIM(nodelowlinks[v], nodeindes[n]);
           }
       }
       
       if(nodelowlinks[v] == nodeindes[v]){
           lit_t n;
           do{
               n = *(stack.rbegin());
               stack.pop_back();
               inStack[n] = 0;
               inSCC[n] = 1;
               component.push_back(n);
           } while(n != v);
           
       }
       return component;
}
