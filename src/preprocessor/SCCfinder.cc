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


#include "preprocessor/SCCfinder.h"

SCCfinder::SCCfinder( uint32_t varCnt) : var_cnt(varCnt),
graph ( VEC_TYPE( std::set<lit_t> )( max_index( varCnt ) ) )
{}

SCCfinder::~SCCfinder()
{}

void SCCfinder::create_graph( const VEC_TYPE( CL_REF )& clauses, assi_t assi )
{
	
	for( uint32_t i = 0 ; i < graph.size(); i++) graph[i].clear();
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses); ++i ){
		const CLAUSE& clause = gsa.get( clauses[i] );
		if( CL_IS_IGNORED(clause)) continue;
		uint32_t undef = 0;
		lit_t undefs[2];
		uint32_t j = 0;
		
		for( ;j < CL_SIZE(clause); ++j ){
			const lit_t l = CL_GET_LIT( clause,j);
			if( assi_is_undef( assi, var(l) ) ) {
				++undef;
				if( undef > 2) break;	
				undefs[undef-1] = l;
				continue;
			}
			if( assi_is_sat( assi, l)) break;
		}
		if( j < CL_SIZE(clause) || undef != 2 ) continue;
		
		graph[ index( inv_lit(undefs[0])) ].insert( undefs[1]);
		graph[ index( inv_lit(undefs[1])) ].insert( undefs[0]);
	}
}

void SCCfinder::storeGraph(const std::string& filename){
	std::fstream file;
	file.open( filename.c_str() , std::ios_base::out);

	if( !file) std::cerr << "error during opening file " << filename << std::endl;
	else std::cerr << "opened file " << filename << std::endl;
	file << "digraph G {" << std::endl;
	for( lit_t l = 0; l < max_index( var_cnt ); ++l )
	{
		std::set<lit_t>::iterator iter;
		for(iter = graph[index(l)].begin(); iter != graph[index(l)].end(); iter ++)
		{
			file << " " << nr(l) << " -> " << nr(*iter) << std::endl;
		}
	}
	file << "}" << std::endl;
	file.close();
}

void SCCfinder::prepareSCCsearch()
{
	
	ind = 0;
	nodeindes.clear();
	nodeindes.resize( max_index( var_cnt ), -1 );
	nodelowlinks.clear();
	nodelowlinks.resize( max_index( var_cnt ), -1 );
	inStack.clear();
	inStack.resize( max_index( var_cnt ), 0 );
	
	inSCC.clear();
	inSCC.resize( max_index( var_cnt ), 0 );
	SCC.clear();
}

void  SCCfinder::resetGraph(){
	for( uint32_t i = 0; i <  max_index( var_cnt ) ; ++i ) graph[i].clear();
}

uint32_t SCCfinder::size() const { return SCC.size(); }

const VEC_TYPE(lit_t)& SCCfinder::getEqs(uint32_t i){
	return SCC[i];
}

void SCCfinder::findSCCs(const VEC_TYPE( CL_REF )& clauses, assi_t assi){

	create_graph(clauses,assi);
	prepareSCCsearch();

	VEC_TYPE( lit_t ) component;
	
	for( lit_t l = 0; l < max_index( var_cnt ); ++l )
	{
		if( graph[l].size() == 0 ) continue; 
		if( inSCC[l] != 0 ) continue; 
		
		component = tarjan(l,l);
		
		if( component.size() > 1 ) SCC.push_back(component);
	}
}

VEC_TYPE( lit_t) SCCfinder::tarjan(lit_t v, lit_t list)
{
	VEC_TYPE( lit_t ) component;
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
               nodelowlinks[v] = MININ( nodelowlinks[v], nodelowlinks[n]);
           } else if(inStack[n] == 1){
               nodelowlinks[v] = MININ(nodelowlinks[v], nodeindes[n]);
           }
       }
       
       if(nodelowlinks[v] == nodeindes[v]){
           lit_t n;
           do{
               n = stack[stack.size() - 1]; 
               stack.pop_back();
               inStack[n] = 0;
               inSCC[n] = 1;
               component.push_back(n);
           } while(n != v);
           
       }
       return component;
}
