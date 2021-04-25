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


#include "preprocessor/ROProp.h"



ROProp::ROProp(uint32_t var_cnt, uint32_t maxUsedClSize) : varCnt( var_cnt), maxUse(maxUsedClSize){
	unitQ = RINGBUFFER_CREATE( lit_t, varCnt );
	RINGBUFFER_CLEAR( lit_t, unitQ);
	VEC_CREATE_SPACE( VEC_TYPE(lit_t), w2, max_index(var_cnt));
	VEC_CREATE_SPACE( VEC_TYPE(l2_t),  w3, max_index(var_cnt));
	
	while( VEC_SIZE( lit_t,w2 ) < max_index(varCnt) )	VEC_PUSH_BACK( VEC_TYPE(lit_t),w2, VEC_TYPE(lit_t)() );
	while( VEC_SIZE( l2_t,w3 ) < max_index(varCnt) )	VEC_PUSH_BACK( VEC_TYPE(l2_t), w3, VEC_TYPE(l2_t)() );

}

ROProp::~ROProp() {

}

void ROProp::setMax( uint32_t max ){
	maxUse = max;
}

solution_t ROProp::init(VEC_TYPE (CL_REF) * clauses,  assi_t assi){
	RINGBUFFER_CLEAR( lit_t, unitQ );
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		if( !addClause( (*clauses)[i], assi ) ) return UNSAT;
	}
	
	
	lit_t l =  dequeue();
	while( l != NO_LIT ){
		if( !propagate(l,assi) ){
#ifdef _ROP_DEBUGME
			cerr << "c failed to propagate " << nr(l) << endl;
#endif
			return UNSAT;
		}
		l =  dequeue();
	}
	return UNKNOWN;
}

solution_t ROProp::init(VEC_TYPE (CL_REF) * clauses,  assi_t assi, VEC_TYPE(lit_t)& newUnits){
	RINGBUFFER_CLEAR( lit_t, unitQ );
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		if( !addClause( (*clauses)[i], assi ) ) return UNSAT;
	}
	
	
	lit_t l =  dequeue();
	while( l != NO_LIT ){
		
		VEC_PUSH_BACK( lit_t, newUnits, l);
		if( !propagate(l,assi) ) return UNSAT;
		l = dequeue();
	}
	return UNKNOWN;
}

void ROProp::clear(){
	for( var_t v = 1 ; v <=varCnt; ++v ){
		w2[ index(lit(v,POS))].clear();
		w2[ index(lit(v,NEG))].clear();
		w3[ index(lit(v,POS))].clear();
		w3[ index(lit(v,NEG))].clear();
	}
}

bool ROProp::createAssignment(assi_t orig, assi_t nAssi, const lit_t assume, VEC_TYPE(lit_t)& implications){
	RINGBUFFER_CLEAR( lit_t, unitQ );
	
	if( !enqueue( nAssi, assume) ) return false;
	
	for( var_t v = 1 ; v <=varCnt; ++v ){
		if( !assi_is_undef(orig, v) ){
			const pol_t p= assi_get_polarity( orig,v);
			if( !enqueue( nAssi, lit(v,p)) ) return false;
		}
	}
	
	
	lit_t l =  dequeue();
	while( l != NO_LIT ){
		if( assi_is_undef( orig, var(l) ) && var(l) != var(assume) ){
			VEC_PUSH_BACK( lit_t, implications, l);
		}
		if( !propagate(l,nAssi) ) return false;
		l =  dequeue();
	}
	return true;
}


bool ROProp::conflictFree(assi_t orig, assi_t nAssi, VEC_TYPE(lit_t)& assumptions, VEC_TYPE(lit_t)& newClause){
	RINGBUFFER_CLEAR( lit_t, unitQ );
	
	for( var_t v = 1 ; v <=varCnt; ++v ){
		if( !assi_is_undef(orig, v) ){
			const pol_t p= assi_get_polarity( orig,v);
			if( !enqueue( nAssi, lit(v,p)) ){
#ifdef _ROP_DEBUGME
				cerr << "c failed to init " << nr(lit(v,p)) << endl;
#endif
				return false;
			}
		}
	}
	
	
	for( uint32_t i = 0 ; i < VEC_SIZE( lit_t, assumptions); ++i ){
		
		
		for( uint32_t j = i; j < VEC_SIZE( lit_t, assumptions); ++j ){
			if( !assi_is_undef(nAssi, var(assumptions[j]) ) ){
				
				if( assi_is_unsat(nAssi, assumptions[j] ) ){
					VEC_PUSH_BACK( lit_t, newClause, assumptions[j]); 
					
					
					return false;
				} else {
					
					
					
					
					for( uint32_t ti = i; ti< VEC_SIZE( lit_t, assumptions); ++ti ){
						if( ti != j ){
							VEC_PUSH_BACK( lit_t, newClause, assumptions[ti]);
						}
					}
					
					return false;
				}
			}
		}
	
		VEC_PUSH_BACK( lit_t, newClause, assumptions[i]);	
		if( !enqueue( nAssi, assumptions[i]) ){
#ifdef _ROP_DEBUGME
			cerr << "c failed to add " << nr(assumptions[i])
					<< " pol of v=" << assi_get_polarity(orig,var(assumptions[i])) << endl;
#endif
			
			return false;
		}

		lit_t l =  dequeue();
		while( l != NO_LIT ){
			if( !propagate(l,nAssi) ){
#ifdef _ROP_DEBUGME
				cerr << "c failed to propagate " << nr(l) << endl;
#endif
				return false;
			}
			l =  dequeue();
		}
	}
	return true;
}

solution_t ROProp::fillAssi(assi_t assi){
	lit_t l =  dequeue();
	while( l != NO_LIT ){
		if( !propagate(l,assi) ) return UNSAT;
		l =  dequeue();
	}
	return UNKNOWN;
}

bool ROProp::addClause(CL_REF clause, assi_t assi){
	CLAUSE & cl = gsa.get (clause);
	if( CL_IS_IGNORED(cl) ) return true;
	
	if (CL_SIZE (cl) < 2){
		if( CL_SIZE (cl) == 0 )return false;
		const lit_t literal = CL_GET_LIT (cl, 0);
		if( !enqueue( assi, literal) ) return false;
		else return true;
	}
	lit_t lits[CL_SIZE (cl)];
	uint32_t undef = 0;
	uint32_t j = 0;
	for (; j < CL_SIZE (cl); ++j) {
		const lit_t literal = CL_GET_LIT (cl, j);
		if( assi_is_undef( assi, var(literal) )) lits[undef++] = literal;
		if( assi_is_sat( assi, literal)) break;
	}
	if( j != CL_SIZE(cl) ) return true;
	
	if( undef == 1 ){
		if( !enqueue( assi, lits[0]) ) return false;
		else return true;
	}
	if( undef == 0 ) return false;

	
	if( undef == 2 ){
#ifdef _ROP_DEBUGME
		cerr << "c ROP add clause " << nr(lits[0]) << "," << nr(lits[1]) << endl;
#endif
		w2[ index( inv_lit(lits[0]))].push_back(lits[1]);
		w2[ index( inv_lit(lits[1]))].push_back(lits[0]);
	}
	
	if( undef == 3 ){
#ifdef _ROP_DEBUGME
		cerr << "c ROP add clause " << nr(lits[0]) << "," << nr(lits[1]) << endl;
#endif
		l2_t l;
		l.a[0] = lits[0];
		l.a[1] = lits[1];	
		w3[ index( inv_lit(lits[2]))].push_back(l);
		l.a[1] = lits[2];	
		w3[ index( inv_lit(lits[1]))].push_back(l);
		l.a[0] = lits[1];	
		w3[ index( inv_lit(lits[0]))].push_back(l);
		
		
	}
	
	return true;
}


bool ROProp::enqueue(assi_t assi, lit_t l){
	if( assi_is_unsat( assi, l) ) return false;
	if( assi_is_sat( assi, l ) ) return true;
#ifdef _ROP_DEBUGME
	cerr << "c ROP enqueue " << nr(l) << endl;
#endif
	assi_set_polarity( assi, var(l), pol(l));
	RINGBUFFER_PUSH_BACK( lit_t, unitQ, l);
	return true;
}


lit_t ROProp::dequeue(){
	if( RINGBUFFER_IS_EMPTY( lit_t, unitQ )) return NO_LIT;
	lit_t l = NO_LIT;
	RINGBUFFER_POP_FRONT( lit_t, unitQ, l );
	return l;
}

bool ROProp::propagate(lit_t l, assi_t assi){
	VEC_TYPE(lit_t)& list2 = w2[index(l)];
#ifdef _ROP_DEBUGME
	cerr << "c ROP prop " << nr(l) << " with list" << list2.size() << endl;
#endif
	for( uint32_t i = 0 ; i < list2.size(); ++i ){
		if( !enqueue(assi, list2[i] ) ){
#ifdef _ROP_DEBUGME
			cerr << " failed to enqueue " << nr(list2[i]) << endl;
#endif
			return false;
		}
#ifdef _ROP_DEBUGME
		else cerr << " added " << nr(list2[i]) << endl;
#endif
	}

	
	if( maxUse > 2 ){
		VEC_TYPE(l2_t)& list3 = w3[index(l)];

		for( uint32_t i = 0 ; i < list3.size(); ++i ){
		
			if( assi_is_unsat( assi, list3[i].a[0]) ){
				if( !enqueue(assi, list3[i].a[1] ) ){
					return false;
				}
			} else if( assi_is_unsat( assi, list3[i].a[1]) ){
				if( !enqueue(assi, list3[i].a[0] ) ){
					return false;
				}
			}
		}
	}
	
	return true;
}

void ROProp::extend( uint32_t newVar ){
	
	VEC_RESIZE( VEC_TYPE(lit_t), w2, max_index(newVar), VEC_TYPE(lit_t)() );
	VEC_RESIZE( VEC_TYPE(l2_t),  w3, max_index(newVar), VEC_TYPE(l2_t)() );
	
	while( VEC_SIZE( lit_t,w2 ) < max_index(varCnt) )	VEC_PUSH_BACK( VEC_TYPE(lit_t),w2, VEC_TYPE(lit_t)() );
	while( VEC_SIZE( l2_t,w3 ) < max_index(varCnt) )	VEC_PUSH_BACK( VEC_TYPE(l2_t), w3, VEC_TYPE(l2_t)() );
	
	varCnt = newVar;
	
}
