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


#ifndef _SUFFIX_REMOVAL_HEURISTIC
#define _SUFFIX_REMOVAL_HEURISTIC


#include "types.h"
#include <stdlib.h>
#include "structures/c_assignment.h"
#include "utils/sort.h"
#include "macros/vector_macros.h"
#include "macros/clause_macros.h"
#include "utils/clause_container.h"
#include "utils/stringmap.h"
#include "sat/searchdata.h"
#include "debug.h"


#include "clauseactivity/lin_clause_activity_heuristic.h"

#include <assert.h>



template<class CAH>
class SuffixRemoval
{
	
	CAH clause_activity_data;	

	CONST_PARAM int32_t suffix_mode;	
	CONST_PARAM uint32_t suffix_size;	
	CONST_PARAM uint32_t keep_CL_SIZE;	
	CONST_PARAM float keep_important_percent;	
	CONST_PARAM int32_t change_suffix_strategy;		
	CONST_PARAM int32_t restart_level_strategy;	
	CONST_PARAM int32_t number_of_unassigned_literals;	
	CONST_PARAM bool keep_last_conflict;	

	int32_t last_conflict_level;	
	CL_REF last_conflict;	
	
	
	VEC_TYPE( VEC_TYPE( CL_REF ) ) level_clauses; 
	VEC_TYPE( CL_REF )	level_free_clauses;		
	VEC_TYPE( CL_REF )	kept_clauses;		
	VEC_TYPE( CL_REF )	kept_restarts;	
	
	
	VEC_TYPE(int32_t)	literals_sorthelp;		
	VEC_TYPE( CL_REF ) remove_these_in_propagation;	
public:
	SuffixRemoval( uint32_t var_cnt, const StringMap& commandline );

	~SuffixRemoval();

	void clauses_learnt(searchData& search, VEC_TYPE(CL_REF)& clauses );

	void remove(searchData& search, VEC_TYPE( CL_REF )& clauses_to_remove);

	void restart_done();

	void get_all_clauses( VEC_TYPE( CL_REF )& restart_learnts );
	
	void reinit(searchData& search, VEC_TYPE( CL_REF )& restart_learnts );

	void inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  );

private:
	void set_parameter( const StringMap& commandline );	
	
	void clause_learnt(searchData& search, const CL_REF clause );
	
	void clauses_readded(searchData& search, VEC_TYPE(CL_REF)& clauses );

	
	void reduce_to_level(searchData& search, int32_t current_level, assi_t assignment);

	
	void reduce_level_free_clauses(searchData& search, float keep_percent, assi_t assignment);	
};









template<class CAH>
inline SuffixRemoval<CAH>::SuffixRemoval( uint32_t var_cnt, const StringMap& commandline ):
	clause_activity_data( commandline ),
	suffix_mode ( 0),
	suffix_size ( 6),
	keep_CL_SIZE ( 2),
	keep_important_percent ( 45),
	change_suffix_strategy ( 0),
	restart_level_strategy ( 0),
	number_of_unassigned_literals ( 2),
	keep_last_conflict ( true)
{
	set_parameter( commandline );
	
	last_conflict_level = 0;
	
	last_conflict = 0;
	
	VEC_CREATE_SPACE( VEC_TYPE( CL_REF ),level_clauses, 250 ); 
	VEC_CREATE_SPACE(CL_REF,level_free_clauses, 2000 ); 
	VEC_CREATE_SPACE(CL_REF, kept_clauses, 2000 ); 
	
	VEC_CREATE(CL_REF, kept_restarts);
	
	
	VEC_CREATE_SPACE( int32_t, literals_sorthelp, 250 );
	VEC_CREATE_SPACE(CL_REF, remove_these_in_propagation, 1000 );
}

template<class CAH>
inline SuffixRemoval<CAH>::~SuffixRemoval()
{
	
	for( uint32_t i = 0; i < VEC_SIZE( VEC_TYPE( CL_REF ),  level_clauses ); i++)
	{
		
		for(uint32_t j = 0; j< VEC_SIZE(CL_REF,  level_clauses[i] ); j++ ){
			CL_DESTROY( gsa.get(level_clauses[i][j]) );
			gsa.release( level_clauses[i][j] );
		}
		VEC_CLEAR(CL_REF,  level_clauses[i] );
		VEC_DESTROY(CL_REF,  level_clauses[i] );
	}
	
	VEC_DESTROY(CL_REF*,  level_clauses );
	
	
	for( uint32_t i = 0; i < VEC_SIZE(CL_REF,  level_free_clauses ); i++ ){
		CL_DESTROY( gsa.get(level_free_clauses[i]) );
		gsa.release( level_free_clauses[i] );
	}
	
	VEC_CLEAR(CL_REF,  level_free_clauses );
	VEC_DESTROY(CL_REF,  level_free_clauses );

	
	for( uint32_t i = 0; i < VEC_SIZE(CL_REF,  kept_clauses ); i++ ){
		CL_DESTROY( gsa.get(kept_clauses[i]) );
		gsa.release( kept_clauses[i] );
	}
	VEC_CLEAR(CL_REF,  kept_clauses );
	VEC_DESTROY(CL_REF,  kept_clauses );
	
	VEC_DESTROY(int,  literals_sorthelp );
	
	
	VEC_DESTROY(CL_REF,  remove_these_in_propagation );

	VEC_CLEAR(CL_REF, kept_restarts );
	VEC_DESTROY(CL_REF, kept_restarts );

	if( last_conflict != 0 && keep_last_conflict ){
		CL_DESTROY( gsa.get(last_conflict) );
		gsa.release( last_conflict );
	}
}

template<class CAH>
inline void SuffixRemoval<CAH>::clauses_learnt(searchData& search, VEC_TYPE(CL_REF)& clauses )
{
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i )
	{
		clause_learnt( search, clauses[i] );
	}
	
	
	clause_activity_data.set_learned_activity( search, clauses );
}

template<class CAH>
inline void SuffixRemoval<CAH>::clauses_readded(searchData& search, VEC_TYPE(CL_REF)& clauses )
{
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i ){
		clause_learnt( search, clauses[i] );
	}
}

template<class CAH>
inline void SuffixRemoval<CAH>::get_all_clauses( VEC_TYPE( CL_REF )&	restart_learnts )
{
	
	VEC_PUSH_BACK_ANOTHER(CL_REF, restart_learnts, level_free_clauses, VEC_SIZE(CL_REF, level_free_clauses));
	VEC_PUSH_BACK_ANOTHER(CL_REF, restart_learnts, kept_clauses, VEC_SIZE(CL_REF, kept_clauses));
	
	
	
	for ( uint32_t i = 0 ; i < VEC_SIZE( VEC_TYPE( CL_REF ), level_clauses ); ++i )
	{
		VEC_PUSH_BACK_ANOTHER(CL_REF, restart_learnts, level_clauses[i], VEC_SIZE(CL_REF, level_clauses[i]));
	}
	
	uint64_t sizesum = 0;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, restart_learnts ); ++i )
	{
		sizesum += CL_SIZE( gsa.get(restart_learnts[i]) );
	}
	
}

template<class CAH>
inline void SuffixRemoval<CAH>::reinit(searchData& search, VEC_TYPE( CL_REF )& restart_learnts )
{
	
	
	VEC_CLEAR(CL_REF, level_free_clauses);
	VEC_CLEAR(CL_REF, kept_clauses);
	
	for ( uint32_t i = 0 ; i < VEC_SIZE( VEC_TYPE( CL_REF ), level_clauses ); ++i )
	{
		VEC_CLEAR(CL_REF, level_clauses[i] );
	}
	
	clauses_readded(search, restart_learnts );
	
	
	uint32_t i = 0;
	for( ; i < VEC_SIZE( CL_REF, restart_learnts); ++i ) if( restart_learnts[i] == last_conflict ) break;
	if( i == VEC_SIZE( CL_REF, restart_learnts ) )	last_conflict = 0;
}


template<class CAH>
inline void SuffixRemoval<CAH>::clause_learnt(searchData& search, const CL_REF clause )
{
	
	int32_t last_level = last_conflict_level;
	CL_REF last_clause = last_conflict;
	int32_t level = 0;	
	
	const CLAUSE& cl = gsa.get(clause);
	const uint32_t size = CL_SIZE( cl );

	
	if( !CL_IS_LEARNT(cl) ) return;

	if( keep_CL_SIZE < size )
	{
		
		VEC_CLEAR(int32_t,  literals_sorthelp );
		
		for(uint32_t i = 0; i < size; ++i )
		{
			VEC_PUSH_BACK(int32_t, literals_sorthelp, search.VAR_LEVEL( var( CL_GET_LIT( cl, i) ) ) );
		}
		
		insertionsort<int32_t>( &(literals_sorthelp[0]), VEC_SIZE( int32_t, literals_sorthelp ) );

		
		switch( suffix_mode )
		{
			case 0:	
				
				if( size <= suffix_size ) break;
				
				level = literals_sorthelp[ suffix_size ];
				break;
			case 1:
				
				if( size <= suffix_size ) level = literals_sorthelp[ size - 1 ];	
				
				level = literals_sorthelp[ size - suffix_size ];
				break;
			case 2:
				
				level = literals_sorthelp[ (size * suffix_size ) / 100 ];
				break;
			default:
				assert( 1 == 0 );	
		}
	
		
		VEC_CLEAR(int32_t,  literals_sorthelp );
	}
	
	
	
	
	last_conflict_level = level;
	
	last_conflict = (CL_REF)clause;
	
	
	if( !keep_last_conflict ){
		last_level = level;
		last_clause = (CL_REF)clause;
	} else {
		
		if( last_clause == 0 ) return;
	}	
	
	
	if( last_level > 0 )
	{
		
		while( last_level >= (int)VEC_SIZE( VEC_TYPE(CL_REF) ,  level_clauses ) )
		{
			VEC_TYPE( CL_REF ) tmp;
			VEC_CREATE_SPACE(CL_REF, tmp, 6 );
			VEC_PUSH_BACK( VEC_TYPE(CL_REF),  level_clauses, tmp );	
		}
		
		VEC_PUSH_BACK(CL_REF,  level_clauses[ last_level ], last_clause );
	} else {
		
		if( last_clause != 0 ) VEC_PUSH_BACK(CL_REF,  level_free_clauses, last_clause );
	}

}


template<class CAH>
inline void SuffixRemoval<CAH>::restart_done()
{
	
	
	if( keep_last_conflict )
	{
		if( last_conflict != 0 ){
			VEC_PUSH_BACK(CL_REF,  kept_restarts, last_conflict );
			last_conflict = 0;
			last_conflict_level = 0;
		}
	}
	clause_activity_data.restart_done();
}



template<class CAH>
inline void SuffixRemoval<CAH>::remove(searchData& search, VEC_TYPE( CL_REF )& clauses_to_remove )
{
	
	
	
	assert( VEC_SIZE(CL_REF,  clauses_to_remove ) == 0 );

	int32_t remove_level;
	switch(restart_level_strategy)	
	{
		case 0:
			remove_level = search.current_level + 1;
		break;
		default:
			assert( 1 == 0 );	
			remove_level = search.current_level + 1;			
	}
	
	
	VEC_CLEAR(CL_REF,  remove_these_in_propagation );
	
	
	reduce_to_level( search, remove_level, search.assi);
	
	
	VEC_TYPE( CL_REF ) wrongly_removed_clauses;
	VEC_CREATE( CL_REF, wrongly_removed_clauses );
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, remove_these_in_propagation ); ++i ){
		
		const CLAUSE& cl = gsa.get(remove_these_in_propagation[i]);
		for(uint32_t j = 0 ; j < CL_SIZE( cl ); ++j ){
			if( search.VAR_REASON( var( CL_GET_LIT( cl, j ) ) ).getCL() == remove_these_in_propagation[i] ){
				VEC_PUSH_BACK( CL_REF, wrongly_removed_clauses, remove_these_in_propagation[i] );
				VEC_ERASE_NO_ORDER( CL_REF, remove_these_in_propagation, i );
				--i;
				break;
			}
		}	
	}
	
	clauses_learnt( search, wrongly_removed_clauses );

	
	for(uint32_t i = 0; i< VEC_SIZE(CL_REF, remove_these_in_propagation); i++)
	{
		
		VEC_PUSH_BACK(CL_REF,  clauses_to_remove, remove_these_in_propagation[i] );
	}
	
}


template<class CAH>
inline void SuffixRemoval<CAH>::reduce_to_level(searchData& search, int32_t current_level, assi_t assignment)
{
	
	switch(change_suffix_strategy)	
	{
		case 0:
		break;
		default:
			assert( 1 == 0 );	
	}
	
	
	for(uint32_t i = current_level + 1; i < VEC_SIZE(CL_REF*,  level_clauses ); i++ )
	{
		
		if( search.current_level != 0 )
		{
			
			for( uint32_t j = 0; j < VEC_SIZE(CL_REF, level_clauses[i]); j++ )
			{
				int32_t max_level = -1;
				const CL_REF clause = level_clauses[i][j];
				const CLAUSE& cl = gsa.get(clause);
				const uint32_t size =CL_SIZE( cl );
				uint32_t k = 0;
				for( ; k < size; k++ )
				{
					if( search.VAR_LEVEL( var( CL_GET_LIT( cl, k) ) ) == -1 )	
					{
						VEC_PUSH_BACK( CL_REF, remove_these_in_propagation, (CL_REF)clause );
						break;
					} else {
						max_level = ( max_level > search.VAR_LEVEL( var( CL_GET_LIT( cl, k) ) ) ) ? max_level : search.VAR_LEVEL( var( CL_GET_LIT( cl, k) ) );
					}
				}
				
				if(k == size) {
					assert(max_level > -1 );	
					VEC_PUSH_BACK(CL_REF,  level_clauses[max_level], clause );
				}
			}
		}			

		
		VEC_CLEAR(CL_REF,  level_clauses[i] );
	}
	
	
	reduce_level_free_clauses( search, keep_important_percent, assignment);
}


template<class CAH>
inline void SuffixRemoval<CAH>::reduce_level_free_clauses(searchData& search, float keep_percent, assi_t assignment)
{
	
	
	
	int32_t erase_limit = (int32_t)( (float)(VEC_SIZE(CL_REF,  level_free_clauses ) * ( 100.0 - keep_percent ) ) / 100.0);

	if( erase_limit >= (int32_t)VEC_SIZE(CL_REF,  level_free_clauses ) )
		erase_limit = VEC_SIZE(CL_REF,  level_free_clauses ) - 1;
		
	for( int32_t i = 0 ; i<erase_limit; ++i )
	{
		const CLAUSE& cl = gsa.get(level_free_clauses[i]);
		const uint32_t size  = CL_SIZE(cl);
		
		uint32_t unassigned_lits = size;	
		
		if( search.current_level != 0 )
		{
			unassigned_lits = 0;
			for( uint32_t j = 0 ; j < size; ++j )
			{
				if( assi_variable_equal( assignment, var( CL_GET_LIT( cl, j ) ), UNDEF ) ) unassigned_lits ++;
			}
		}
		
		if( unassigned_lits <= (uint32_t)number_of_unassigned_literals || keep_CL_SIZE > size )
		{
			erase_limit --;
			CL_REF tmp = level_free_clauses[i];
			level_free_clauses[ i ] = level_free_clauses[ erase_limit ];
			level_free_clauses[ erase_limit ] = tmp;
			
			--i;
		}
	}
	
	
	

	
	

		
		

	if( erase_limit > 0 )
	{
	
		VEC_PUSH_BACK_ANOTHER(CL_REF,  remove_these_in_propagation, level_free_clauses, (uint32_t)erase_limit);
	
		VEC_DELETE_FIRST_ELEMENTS(CL_REF,  level_free_clauses, erase_limit);


	}
}

template<class CAH>
inline void SuffixRemoval<CAH>::inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  )
{
	
	if( clause_activity_data.inc_analysis_activity(search, clauses ) )
	{
		
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, level_free_clauses); ++i ){
			CL_INC_ACTIVITY( gsa.get(level_free_clauses[i]), 1.0f/clause_activity_data.overflow_divisor() );
		}
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, kept_clauses); ++i ){
			CL_INC_ACTIVITY( gsa.get(kept_clauses[i]), 1.0f/clause_activity_data.overflow_divisor() );
		}
		
		for ( uint32_t i = 0 ; i < VEC_SIZE( VEC_TYPE( CL_REF ), level_clauses ); ++i ){
			for( uint32_t j = 0 ; j < VEC_SIZE( CL_REF, level_clauses[i]); ++j ){
				CL_INC_ACTIVITY( gsa.get(level_clauses[i][j]), 1.0f/clause_activity_data.overflow_divisor() );
			}
		}
	}
}


template<class CAH>
inline void SuffixRemoval<CAH>::set_parameter(const StringMap& commandline)
{

#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"suffix_mode" ) ) suffix_mode = atoi( commandline.get( (const char*)"suffix_mode" ).c_str() );
	if( commandline.contains( (const char*)"suffix_size" ) ) suffix_size = atoi( commandline.get( (const char*)"suffix_mode" ).c_str() );
	if( commandline.contains( (const char*)"suffix_kept_cls_size" ) ) keep_CL_SIZE = atoi( commandline.get( (const char*)"suffix_kept_cls_size" ).c_str() );
	if( commandline.contains( (const char*)"suffix_kept_vip_percent" ) ) keep_important_percent = atoi( commandline.get( (const char*)"suffix_kept_vip_percent" ).c_str() );
	if( commandline.contains( (const char*)"suffix_suffix_strategy" ) ) change_suffix_strategy = atoi( commandline.get( (const char*)"suffix_suffix_strategy" ).c_str() );
	if( commandline.contains( (const char*)"suffix_level_strategy" ) ) restart_level_strategy = atoi( commandline.get( (const char*)"suffix_level_strategy" ).c_str() );
	if( commandline.contains( (const char*)"suffix_undef_lits" ) ) number_of_unassigned_literals = atoi( commandline.get( (const char*)"suffix_undef_lits" ).c_str() );

	
	if( number_of_unassigned_literals < 1 ) number_of_unassigned_literals = 1;
	if( commandline.contains( (const char*)"rem_keep_last" ) ) keep_last_conflict = 0 != atoi( commandline.get( (const char*)"rem_keep_last" ).c_str() );
#endif
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== suffix removal heuristic information ===" << std::endl;
		std::cerr << " parameter        info" << std::endl;
		std::cerr << " suffix_mode              how to determin the suffix, delete if" << std::endl;
		std::cerr << "                   0=X assigned literals left" << std::endl;
		std::cerr << "                   1=X literals unassigned" << std::endl;
		std::cerr << "                   2= X% assigned literals left" << std::endl;
		std::cerr << " suffix_size              X from above" << std::endl;
		std::cerr << " suffix_kept_vip_percent  keep X percent of important clauses" << std::endl;
		std::cerr << " suffix_suffix_strategy   how to change suffix on remove event" << std::endl;
		std::cerr << " suffix_level_strategy    how to change erase level on event" << std::endl;
		std::cerr << " suffix_undef_lits        undefined literals to erase important clause" << std::endl;
		std::cerr << "                          clauses with less size stay forever" << std::endl;
		std::cerr << " suffix_kept_cls_size     size of clauses, which where not suffixed" << std::endl;
		std::cerr << " rem_keep_last  0,1    keep last conflict before restart" << std::endl;
		std::cerr << std::endl;
	}
#endif
}



#endif
