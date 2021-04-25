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


#ifndef _ACTIVITY_REMOVAL_HEURISTIC
#define _ACTIVITY_REMOVAL_HEURISTIC


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
class ActivityRemoval
{
	
	CAH clause_activity_data;	

	
	CONST_PARAM float keep_percent;	
	CONST_PARAM float keep_activity;	
	CONST_PARAM uint32_t keep_size;	
	CONST_PARAM bool keep_last_conflict;	
	CONST_PARAM int recalc_act; 
	
	
	CL_REF last_conflict;	
	VEC_TYPE( CL_REF )	added_clauses;	
	VEC_TYPE( CL_REF )	kept_clauses;	
	VEC_TYPE( CL_REF )	kept_restarts;	
	VEC_TYPE( CL_REF )	tmp_kept_restarts;	
	
public:
	
	ActivityRemoval( uint32_t var_cnt, const StringMap& commandline );

	
	~ActivityRemoval();

	
	void clauses_learnt(searchData& search, VEC_TYPE(CL_REF)& clauses );	

	
	void remove(searchData& search, VEC_TYPE( CL_REF )& clauses_to_remove );

	
	void restart_done();

	
	void get_all_clauses( VEC_TYPE( CL_REF )&	restart_learnts );

	
	void reinit(searchData& search, VEC_TYPE( CL_REF )& restart_learnts );

	
	void inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  );

private:
	
	void set_parameter( const StringMap& commandline );	
	
	
	void clause_learnt(searchData& search, const CL_REF clause );
	
	
	void clauses_readded(searchData& search, VEC_TYPE(CL_REF)& clauses );

	
	void sort_added_clauses();
};








template<class CAH>
inline ActivityRemoval<CAH>::ActivityRemoval( uint32_t var_cnt, const StringMap& commandline ):
	clause_activity_data( commandline ),
	keep_percent ( 50), 
	keep_activity ( 0.5),
	keep_size ( 2), 
	keep_last_conflict ( true),
	recalc_act(0)
{
	set_parameter( commandline );

	last_conflict = 0;

	VEC_CREATE(CL_REF, added_clauses ); 
	VEC_CREATE(CL_REF, kept_clauses ); 
	VEC_CREATE(CL_REF, kept_restarts );
	VEC_CREATE(CL_REF, tmp_kept_restarts );
}

template<class CAH>
inline ActivityRemoval<CAH>::~ActivityRemoval()
{
	
	for( uint32_t i = 0; i < VEC_SIZE(CL_REF, added_clauses); ++i ){
		CLAUSE& cl = gsa.get( added_clauses[i]);
		CL_DESTROY( cl );
		gsa.release( added_clauses[i] );
	}
	VEC_CLEAR( CL_REF, added_clauses );
	VEC_DESTROY( CL_REF, added_clauses );
	VEC_CLEAR( CL_REF, tmp_kept_restarts );
	VEC_DESTROY( CL_REF, tmp_kept_restarts );

	
	for( uint32_t i = 0; i < VEC_SIZE(CL_REF, kept_clauses); ++i ){
		CLAUSE& cl = gsa.get( kept_clauses[i]);
		CL_DESTROY( cl );
		gsa.release( kept_clauses[i] );
	}
	VEC_CLEAR( CL_REF, kept_clauses );
	VEC_DESTROY( CL_REF, kept_clauses );	

	for( uint32_t i = 0; i < VEC_SIZE(CL_REF, kept_restarts); ++i ){
		CLAUSE& cl = gsa.get( kept_restarts[i]);
		CL_DESTROY( cl );
		gsa.release( kept_restarts[i] );
	}
	VEC_CLEAR( CL_REF, kept_restarts );
	VEC_DESTROY( CL_REF, kept_restarts );

	
}

template<class CAH>
inline void ActivityRemoval<CAH>::clauses_learnt(searchData& search, VEC_TYPE(CL_REF)& clauses )
{
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i )
	{
		clause_learnt( search, clauses[i] );
	}
	
	clause_activity_data.set_learned_activity( search, clauses );
}

template<class CAH>
inline void ActivityRemoval<CAH>::clauses_readded(searchData& search, VEC_TYPE(CL_REF)& clauses )
{
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses ); ++i )
	{
		clause_learnt( search, clauses[i] );
	}
}


template<class CAH>
inline void ActivityRemoval<CAH>::get_all_clauses( VEC_TYPE( CL_REF )&	restart_learnts )
{

	VEC_PUSH_BACK_ANOTHER( CL_REF, restart_learnts, added_clauses, VEC_SIZE( CL_REF, added_clauses));
	VEC_PUSH_BACK_ANOTHER( CL_REF, restart_learnts, kept_clauses, VEC_SIZE( CL_REF, kept_clauses));
	VEC_PUSH_BACK_ANOTHER( CL_REF, restart_learnts, kept_restarts, VEC_SIZE( CL_REF, kept_restarts));
	
	if ( last_conflict != 0 && keep_last_conflict) VEC_PUSH_BACK( CL_REF, restart_learnts, last_conflict);
}

template<class CAH>
inline void ActivityRemoval<CAH>::reinit(searchData& search, VEC_TYPE( CL_REF )& restart_learnts )
{
	
	VEC_CLEAR( CL_REF, added_clauses );
	VEC_CLEAR( CL_REF, kept_clauses );

	
	VEC_CLEAR( CL_REF, tmp_kept_restarts );
	CL_REF tmpLastCon = last_conflict;
	last_conflict = 0;
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, restart_learnts ); ++i ){
		if( keep_last_conflict && tmpLastCon == restart_learnts[i]){
			last_conflict = tmpLastCon;
			VEC_ERASE_NO_ORDER( CL_REF, restart_learnts, i );
			--i;
			continue;
		}
		for( uint32_t j=0; j < VEC_SIZE( CL_REF, kept_restarts); ++j ){
			if( restart_learnts[i] == kept_restarts[j] ){
				VEC_PUSH_BACK( CL_REF, tmp_kept_restarts, restart_learnts[i]);
				VEC_ERASE_NO_ORDER( CL_REF, restart_learnts, i );
				VEC_ERASE_NO_ORDER( CL_REF, kept_restarts, j );
				i--;
				break;
			}
		}
	}
	VEC_CLEAR( CL_REF, kept_restarts );
	VEC_PUSH_BACK_ANOTHER( CL_REF, kept_restarts, tmp_kept_restarts, VEC_SIZE(CL_REF, tmp_kept_restarts) );

	
	clauses_readded(search, restart_learnts );
}


template<class CAH>
inline void ActivityRemoval<CAH>::clause_learnt(searchData& search, const CL_REF clause )
{
	
	CLAUSE& cl = gsa.get(clause);
	
	
	if( !CL_IS_LEARNT(cl) ) return;

	if( !keep_last_conflict )
	{
		if( CL_SIZE( cl ) <= keep_size )
		{
			VEC_PUSH_BACK( CL_REF, kept_clauses, (CL_REF)clause );
		} else {
			VEC_PUSH_BACK( CL_REF, added_clauses, (CL_REF)clause );
		}
	} else {
		if( last_conflict != 0 )
		{
			if( CL_SIZE( gsa.get(last_conflict) ) <= keep_size )
			{
				VEC_PUSH_BACK( CL_REF, kept_clauses, last_conflict );
			} else {
				VEC_PUSH_BACK( CL_REF, added_clauses, last_conflict );
			}
		}
		last_conflict = (CL_REF)clause;
	}
}


template<class CAH>
inline void ActivityRemoval<CAH>::restart_done()
{
	if( keep_last_conflict )
	{
		if( last_conflict != 0 ){
			VEC_PUSH_BACK( CL_REF, kept_restarts, last_conflict );
			CL_SET_ACTIVITY( gsa.get(last_conflict), clause_activity_data.overflow_divisor() - 1e10 );
			last_conflict = 0;
		}
	}
	clause_activity_data.restart_done();
}

template<class CAH>
inline void ActivityRemoval<CAH>::inc_analysis_activity(searchData& search,VEC_TYPE( CL_REF )& clauses  )
{
	
	if( clause_activity_data.inc_analysis_activity(search, clauses ) )
	{
		
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, added_clauses); ++i )
		{
			CL_INC_ACTIVITY( gsa.get(added_clauses[i]), 1.0f/clause_activity_data.overflow_divisor() );
		}
		for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, kept_clauses); ++i )
		{
			CL_INC_ACTIVITY( gsa.get(kept_clauses[i]), 1.0f/clause_activity_data.overflow_divisor() );
		}
	}
}



template<class CAH>
inline void ActivityRemoval<CAH>::remove(searchData& search, VEC_TYPE( CL_REF )& clauses_to_remove )
{
	
	assert( VEC_SIZE( CL_REF, clauses_to_remove ) == 0 );
	
	uint32_t startind = (uint32_t)	((float)VEC_SIZE( CL_REF, added_clauses ) * keep_percent / 100.0);

	
	if( recalc_act == 1 ){
		clause_activity_data.set_learned_activity( search, added_clauses );
	}

	
	sort_added_clauses();
	
	
	if( false )
	{
		
		
		
	} else {
		while(VEC_SIZE( CL_REF, added_clauses) > startind )
		{
			
			if( keep_activity <= CL_GET_ACTIVITY( gsa.get(added_clauses[ VEC_SIZE( CL_REF, added_clauses)  - 1 ]) ) ) break;
			
			
			VEC_PUSH_BACK( CL_REF, clauses_to_remove, added_clauses[ VEC_SIZE( CL_REF, added_clauses)  - 1 ] );
			VEC_POP_BACK( CL_REF, added_clauses );
		}
	}
	
	
	if( recalc_act == 2 ){
		clause_activity_data.set_learned_activity( search, added_clauses );
	}

	
	VEC_TYPE( CL_REF ) wrongly_removed_clauses;
	VEC_CREATE( CL_REF, wrongly_removed_clauses );
	for( uint32_t i = 0 ; i < VEC_SIZE( CL_REF, clauses_to_remove ); ++i ){
		
		CLAUSE& cl = gsa.get( clauses_to_remove[i] );
		for(uint32_t j = 0 ; j < CL_SIZE( cl ); ++j ){
			if( search.VAR_REASON( var( CL_GET_LIT( cl, j ) ) ).isLit() ) continue;	
			if( search.VAR_REASON( var( CL_GET_LIT( cl, j ) ) ).getCL() == clauses_to_remove[i] ){
				VEC_PUSH_BACK( CL_REF, wrongly_removed_clauses, clauses_to_remove[i] );
				VEC_ERASE_NO_ORDER( CL_REF, clauses_to_remove, i );
				--i;
				break;
			}
		}
	}
	
	
	clauses_learnt( search, wrongly_removed_clauses );
}

template<class CAH>
inline void ActivityRemoval<CAH>::sort_added_clauses()
{
	
	int32_t m, s;
	int32_t n = VEC_SIZE( CL_REF, added_clauses );
	
	CL_REF* b = new CL_REF[ (n+1)/2 ];
	
	for (s=1; s<n; s+=s)
	{
		for (m=n-1-s; m>=0; m-=s+s)
		{
			int32_t lo = ((m-s+1) > 0) ? (m-s+1) : 0; 
			
			int32_t i, j, k;
			i=0; j=lo;
				
				while (j<=m)
					b[i++]=added_clauses[j++];
				i=0; k=lo;
				
				while (k<j && j<= m + s )
					if (  CL_GET_ACTIVITY( gsa.get(b[i]) )  > CL_GET_ACTIVITY( gsa.get(added_clauses[j]) ) )
						added_clauses[k++]=b[i++];
					else
						added_clauses[k++]=added_clauses[j++];
				
				while (k<j)
					added_clauses[k++]=b[i++];
		}
	}
	
	delete [] b;
}

template<class CAH>
inline void ActivityRemoval<CAH>::set_parameter(const StringMap& commandline)
{

#ifdef USE_COMMANDLINEPARAMETER
	if( commandline.contains( (const char*)"rem_keep_percent" ) ) keep_percent = atof( commandline.get( (const char*)"rem_keep_percent" ).c_str() );
	if( commandline.contains( (const char*)"rem_keep_size" ) ) keep_size = atoi( commandline.get( (const char*)"rem_keep_size" ).c_str() );
	if( commandline.contains( (const char*)"rem_keep_last" ) ) keep_last_conflict = 0 != atoi( commandline.get( (const char*)"rem_keep_last" ).c_str() );
	if( commandline.contains( (const char*)"act_rem_keep_activity" ) ) keep_activity = atof( commandline.get( (const char*)"act_rem_keep_activity" ).c_str() );
	if( commandline.contains( (const char*)"act_rem_recalc_act" ) ) recalc_act = atoi( commandline.get( (const char*)"act_rem_recalc_act" ).c_str() );
#endif

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( commandline.contains( (const char*)"-h" ) || commandline.contains( (const char*)"--help" ) )
	{
		std::cerr << "=== activity removal heuristic information ===" << std::endl;
		std::cerr << " parameter             value info" << std::endl;
		std::cerr << " rem_keep_precent      0-100 size of clauses, which where not suffixed" << std::endl;
		std::cerr << " rem_keep_size         1-n   size of clauses, which where not suffixed" << std::endl;
		std::cerr << " rem_keep_last         0,1   keep last conflict before restart" << std::endl;
		std::cerr << " act_rem_keep_activity 0-x   size of clauses, which where not suffixed" << std::endl;
		std::cerr << " act_rem_recalc_act    0,1,2 recalculate clause activities (0=no,1=before removing,2=after)" << std::endl;
		std::cerr << std::endl;
	}
#endif
#endif
}



#endif
