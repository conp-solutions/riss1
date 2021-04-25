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


#include "defines.h"
#ifdef SATSOLVER

#ifndef _COMPONENT_H
#define _COMPONENT_H

#include "utils/stringmap.h"
#include "structures/c_assignment.h"

#include "preprocessor/coprocessor.h"
#include "preprocessor/satellike.h"
#include "preprocessor/no_preprocessor.h"

#include "sat/searchdata.h"

#include "decisionheuristic/var_activity_heuristic.h"

#include "unitpropagation/dual_propagation.h"
#include "unitpropagation/csp_propagation.h"
#include "unitpropagation/Pwatchedpropagation.h"

#include "fileparser/iterative_parser.h"

#include "removalheuristic/activity_removal_heuristic.h"
#include "removalheuristic/suffix_removal_heuristic.h"

#include "eventheuristic/dynamic_event_heuristic.h"
#include "eventheuristic/geometric_event_heuristic.h"
#include "eventheuristic/luby_event_heuristic.h"
#include "eventheuristic/cls_ratio_event_heuristic.h"

#include "clauseactivity/lin_clause_activity_heuristic.h"

#include "analyze/cdclminanalyze.h"

class Network;


class ComponentManager
{
public:
	
	ComponentManager();
	
	
	ComponentManager( StringMap& commandline );

	
	VEC_TYPE( assi_t )* solve(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline, Network& nw);

	
	solution_t parse_file(char* filename, uint32_t & var_cnt, VEC_TYPE( CL_REF )* clause_set, const StringMap& commandline);
	
private:
	
	
	
	template<class UPR>
	VEC_TYPE( assi_t )* solve_UP(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);
	
	
	template<class UPR, class DEH>
	VEC_TYPE( assi_t )* solve_UP_DEH(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE, class RME>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE_RME(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE, class RME, class ANA>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE_RME_ANA(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE_RME_ANA_PRP(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE_RME_ANA_PRP_CAH(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

	
	template<class UPR, class DEH, class RSE, class RME, class ANA, class PRP, class CAH, template< class > class RMH>
	VEC_TYPE( assi_t )* solve_UP_DEH_RSE_RME_ANA_PRP_CAH_RMH(  VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, StringMap commandline, Network& nw);

};

#include "utils/network.h"

#endif

#endif 
