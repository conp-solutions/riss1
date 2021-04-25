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


#include "utils/formulaSCCfinder.h"

void FormulaSCCFinder::analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment )
{
	std::cerr << "c ========================================" << std::endl;
	std::cerr << "c SCC analyzer:" << std::endl;
	std::cerr << "c build variable graph" << std::endl;
	
	
	graph = std::vector< std::set<var_t> > (var_cnt + 1);
	const VEC_TYPE( CL_REF )& clauses = *clause_set;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses); ++i ){
		const CLAUSE& clause = gsa.get(clauses[i]);
		
		for( uint32_t j = 0 ; j < CL_SIZE( clause ); ++j ){
			const var_t var1J = var( CL_GET_LIT( clause, j ) );
			if( assi_variable_equal( assignment, var1J, pol(CL_GET_LIT( clause, j )) ) ) continue;
		}
		
		
		for( uint32_t j = 0 ; j < CL_SIZE( clause ); ++j ){
			const var_t var1J = var( CL_GET_LIT( clause, j ) );
			if( assi_variable_equal( assignment, var1J, pol( inv_lit(CL_GET_LIT( clause, j )) )  ) ) continue;
			for( uint32_t k = j+1 ; k < CL_SIZE( clause ); ++k ){
				const var_t var1K = var( CL_GET_LIT( clause, k ) );
				if( assi_variable_equal( assignment, var1K, pol( inv_lit(CL_GET_LIT( clause, k )) )  ) ) continue;
				
				graph[var1J].insert(var1K);
				graph[var1K].insert(var1J);
			}
		}
	}
	
	int distributionSize = 10;
	int zeroCounter = 0;
	int counter[distributionSize];
	memset(counter, 0, distributionSize * sizeof(int) );
	int sumSize = 0;
	double average = 0;
	
	for( uint32_t i = 0 ; i < graph.size(); ++i )
	{
		if( graph[i].size() < (unsigned int)distributionSize )
		{
			counter[ graph[i].size() ] ++;
		}
		if( graph[i].size() == 0 ) {zeroCounter++; continue;}
		sumSize += graph[i].size();
	}
	
	average = (double)sumSize / (double)(var_cnt - zeroCounter);

	std::cerr << "c variables: " << var_cnt << std::endl;
	std::cerr << "c not appeared: " << zeroCounter << std::endl;
	std::cerr << "c variables to assign: " << var_cnt - zeroCounter << std::endl;
	std::cerr << "c min nr of edges : " << min_edge() << std::endl;
	std::cerr << "c avg. nr of edges: " << average << std::endl;
	std::cerr << "c max nr of edges : " << max_edge() << std::endl;
	std::cerr << "c distribution of sizes: " << std::endl;
	for( int i = 0 ; i < distributionSize; ++i )
	{
		std::cerr << "c edges " << i << "\t " << counter[i] << std::endl;
	}
	std::cerr << "c calculate nr of SCCs ... ";

	
	unsigned char* state = new unsigned char[var_cnt + 1];
	memset(state, 0, (var_cnt + 1)*sizeof(unsigned char));
	
	var_t currentV = 1;
	while( currentV <= var_cnt && graph[currentV].size() == 0) currentV ++;
	
	std::deque<var_t> workQ;

	int currentSCC = 2;	
	
	while( currentV <= var_cnt )
	{
		workQ.push_back(currentV);
		
		while( !workQ.empty() )
		{
			
			currentV = workQ.front();
			workQ.pop_front();
			
			
			if( graph[currentV].size() != 0 )
			{
				std::set<var_t>::iterator iter;
				
				for( iter = graph[currentV].begin(); iter != graph[currentV].end(); iter++ ){
					
					if( state[ *iter ] != 0 ) continue;
					
					state[ *iter ] = 1;
					workQ.push_back(*iter);
				}
				state[currentV] = currentSCC;
			}
		}
		
		currentSCC ++;
		while( currentV <= var_cnt && (graph[currentV].size() == 0 || state[currentV] != 0 ) ) currentV ++;	
		
	}
	
	std::cerr << currentSCC - 2 << std::endl;
	std::cerr << "c ========================================" << std::endl;	

	delete [] state;	
	
	
	if(currentSCC-2 > 1) return;
	
	std::vector<var_t> minV = min_edges_variables();
	std::cerr << "c nr of minEdgeVariables: " << minV.size() << std::endl;
	
	if(minV.size() == 0 ) return;
	
	
	state = new unsigned char[var_cnt + 1];
	memset(state, 0, (var_cnt + 1)*sizeof(unsigned char));

	std::vector<var_t> innerNodes; 
	std::set<var_t> betweenNodes;
	
	double minScore = (double)var_cnt;
	uint32_t minBoundNodes = var_cnt;
	uint32_t minInnerNodes = 0;
	
	
	var_t split1V = minV[0];
	state[split1V]= 1;
	betweenNodes.insert(split1V);

	bool criteria = true;	
	while( criteria )
	{
		
		int minEdges = 0;
		var_t minV = *betweenNodes.begin();
		
		std::set<var_t>::iterator innerIter;
		
		for( innerIter = graph[minV].begin(); innerIter != graph[minV].end(); innerIter++ ){
			
			if( state[ *innerIter ] == 0 ) minEdges ++;
		}
		
		std::set<var_t>::iterator iter;
		
		for( iter = betweenNodes.begin(); iter != betweenNodes.end(); iter++ )
		{
			int edgeCount = 0;			
			std::set<var_t>::iterator innerIter;
			
			for( innerIter = graph[*iter].begin(); innerIter != graph[*iter].end(); innerIter++ ){
				
				if( state[ *innerIter ] == 0 ) edgeCount ++;
			}
			
			if( edgeCount < minEdges ){
				minEdges = edgeCount;
				minV = *iter;
			}
		}
		
		betweenNodes.erase( minV );

		
		
		state[minV] = 2;
		
		int count = 0;
		for( iter = graph[minV].begin(); iter != graph[minV].end(); iter++ ){
			
			if( state[ *iter ] != 0 ) continue;
			count ++;
			
			state[*iter] = 1;
			
			betweenNodes.insert(*iter);
		}

		
		
		state[minV] = 2;
		innerNodes.push_back(minV);
		
		double score = ( (double)betweenNodes.size()/((double)var_cnt - (double)innerNodes.size() ) )
										+ ( (double)betweenNodes.size()/((double)(innerNodes.size() + betweenNodes.size())) );
		
		if( score < minScore )
		{
			if( true && (minInnerNodes < innerNodes.size() - 1 || minInnerNodes < 10 ))
			{
				std::cerr << "found new score - old score:" << std::endl;
				std::cerr << "c minScore: " << minScore << std::endl;
				std::cerr << "c min boundnodes: " << minBoundNodes << std::endl;
				std::cerr << "c min innerNodes: " << minInnerNodes << std::endl;
			}
			minScore = score;
			minBoundNodes = betweenNodes.size();
			minInnerNodes = innerNodes.size();
		}
		
		criteria = ( betweenNodes.size() + innerNodes.size() <= var_cnt - zeroCounter ) && (betweenNodes.size() < 10) ;
	}

	std::cerr << "c final score" << std::endl;
	std::cerr << "c try to split1 clauses:" << std::endl;
	std::cerr << "c minScore: " << minScore << std::endl;
	std::cerr << "c min boundnodes: " << minBoundNodes << std::endl;
	std::cerr << "c min innerNodes: " << minInnerNodes << std::endl;
	std::cerr << "c ========================================" << std::endl;	
	delete [] state;	
}


void FormulaSCCFinder::SCC( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment  )
{
	
	
	graph = std::vector< std::set<var_t> > (var_cnt + 1);
	const VEC_TYPE( CL_REF )& clauses = *clause_set;
	
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses); ++i ){
		const CLAUSE& clause = clauses[i];
		
		for( uint32_t j = 0 ; j < CL_SIZE( clause ); ++j ){
			const var_t var1J = var( CL_GET_LIT( clause, j ) );
			if( assi_variable_equal( assignment, var1J, pol(CL_GET_LIT( clause, j )) ) ) continue;
		}
		
		
		for( uint32_t j = 0 ; j < CL_SIZE( clause ); ++j ){
			const var_t var1J = var( CL_GET_LIT( clause, j ) );
			if( assi_variable_equal( assignment, var1J, pol( inv_lit(CL_GET_LIT( clause, j )) )  ) ) continue;
			for( uint32_t k = j+1 ; k < CL_SIZE( clause ); ++k ){
				const var_t var1K = var( CL_GET_LIT( clause, k ) );
				if( assi_variable_equal( assignment, var1K, pol( inv_lit(CL_GET_LIT( clause, k )) )  ) ) continue;
				
				graph[var1J].insert(var1K);
				graph[var1K].insert(var1J);
			}
		}
	}
	
	std::cerr << "c calculate nr of SCCs ... ";

	
	unsigned char* state = new unsigned char[var_cnt + 1];
	memset(state, 0, (var_cnt + 1)*sizeof(unsigned char));
	
	var_t currentV = 1;
	while( currentV <= var_cnt && graph[currentV].size() == 0) currentV ++;
	
	std::deque<var_t> workQ;

	int currentSCC = 2;	
	
	while( currentV <= var_cnt )
	{
		workQ.push_back(currentV);
		
		while( !workQ.empty() )
		{
			
			currentV = workQ.front();
			workQ.pop_front();
			
			
			if( graph[currentV].size() != 0 )
			{
				std::set<var_t>::iterator iter;
				
				for( iter = graph[currentV].begin(); iter != graph[currentV].end(); iter++ ){
					
					if( state[ *iter ] != 0 ) continue;
					
					state[ *iter ] = 1;
					workQ.push_back(*iter);
				}
				state[currentV] = currentSCC;
			}
		}
		
		currentSCC ++;
		while( currentV <= var_cnt && (graph[currentV].size() == 0 || state[currentV] != 0 ) ) currentV ++;	
		
	}
	
	std::cerr << currentSCC - 2 << std::endl;

	delete [] state;
}


uint32_t FormulaSCCFinder::max_edge()
{
	uint32_t max = 0;
	for( var_t i = 1 ; i < graph.size(); ++i )
	{
		max = ( graph[i].size() > max ) ? graph[i].size() : max;
	}
	return max;
}


uint32_t FormulaSCCFinder::min_edge()
{
	uint32_t min = (uint32_t)-1; 
	for( var_t i = 1 ; i < graph.size(); ++i )
	{
		if( graph[i].size() == 0 ) continue;
		min = ( graph[i].size() < min ) ? graph[i].size() : min;
	}
	return min;
}


std::vector<var_t> FormulaSCCFinder::min_edges_variables()
{
	uint32_t min = min_edge();
	std::cerr << "min in min_edge_var1s: " << min << std::endl;
	std::vector<var_t> ret;	
	for( var_t i = 1 ; i < graph.size(); ++i )
	{
		if( graph[i].size() == min ) ret.push_back(i);
	}
	return ret;
}


std::vector<var_t> FormulaSCCFinder::max_edges_variables()
{
	uint32_t max = max_edge();
	std::vector<var_t> ret;	
	for( var_t i = 1 ; i < graph.size(); ++i )
	{
		if( graph[i].size() == max ) ret.push_back(i);
	}
	return ret;
}
