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


#include "utils/formulacsps.h"

void FormulaCsps::analyze( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, bool add, const assi_t assignment  )
{
	cerr << "c start analysis" << endl;

	
	seen = new char[max_index(var_cnt)];
	memset( seen, 0, max_index(var_cnt) );

		
	VEC_CREATE_SPACE (VEC_TYPE (CL_REF), binOcc, max_index(var_cnt ) );
	VEC_CLEAR (VEC_TYPE (CL_REF), binOcc);
	for (uint32_t i = 0; i < max_index(var_cnt ) + 1; ++i) {
		VEC_TYPE (CL_REF) tmp;
		VEC_CREATE (CL_REF, tmp);
		VEC_PUSH_BACK (VEC_TYPE (CL_REF), binOcc, tmp);
	}
	
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clause_set)); ++i) {
		const CL_REF c = (*clause_set)[i];
		CLAUSE& cl = gsa.get(c);
		if( CL_SIZE(cl) == 2 ){
			VEC_PUSH_BACK( CL_REF, binOcc[ index( CL_GET_LIT(cl,0) ) ], c );
			VEC_PUSH_BACK( CL_REF, binOcc[ index( CL_GET_LIT(cl,1) ) ], c );
		}
	}
	
	
	
	
	uint32_t count1ooN = 0;
	uint32_t count1ooNcandidates = 0;
	uint32_t count1ooNclauses = 0;
	
	uint32_t countAnd = 0;
	uint32_t countAndCand = 0;
	uint32_t countAndClauses = 0;
	
	uint32_t countHornP = 0;
	uint32_t countHornN = 0;
	uint32_t countPos = 0;
	uint32_t countNeg = 0;
		
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clause_set)); ++i) {
		const CL_REF c = (*clause_set)[i];
		CLAUSE& cl = gsa.get(c);
		if( CL_SIZE(cl) == 2 ) continue;
		count1ooNcandidates ++;
		if( oneOutOfN(c) ){
			count1ooN ++;
			count1ooNclauses ++;
			uint32_t tmp = CL_SIZE(cl) * (CL_SIZE(cl) - 1);
			count1ooNclauses += (tmp / 2);
		}
		
		if( CL_SIZE(cl) == 3 ) countAndCand ++;
		if( dISaANDb(c) ){
			countAnd++;
			countAndClauses += 3;
		}
		
		int h = litRatio(c);
		if( h == 1 ) countHornP++;
		else if( h == -1 ) countHornN++;
		else if( h == 2 ) countPos ++;
		else if( h == -2 ) countNeg ++; 
		
	}
	
	
	uint32_t countBXor = 0;
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clause_set)); ++i) {
		const CL_REF c = (*clause_set)[i];
		if( binXor(c) ){
			countBXor ++;
		}
	}
	countBXor = countBXor >> 1;	
	
	std::cerr << "c ==================" << std::endl;
	std::cerr << "c  1ooN: " << count1ooN << "(/" << count1ooNcandidates << ") in " << count1ooNclauses << " clauses" << std::endl;
	std::cerr << "c  horn: pos:" << countHornP << " neg: " << countHornN << " of " << count1ooNcandidates << std::endl;
	std::cerr << "c  pure: pos:" << countPos << " neg: " << countHornN << " of " << count1ooNcandidates << std::endl;
	std::cerr << "c  bAnd: " << countAnd << "(/" << countNeg << ") in " << countAndClauses << " clauses" << std::endl;
	std::cerr << "c  bXor: " << countBXor << std::endl;
	std::cerr << "c ==================" << std::endl;
	
	FindXor( clause_set, var_cnt );
}

bool FormulaCsps::oneOutOfN(CL_REF c){
	
	const CLAUSE& cl = gsa.get(c);
	const uint32_t size = CL_SIZE( cl );
	
	for( uint32_t i = 0; i < size; ++i ){
		
		const lit_t current = inv_lit(CL_GET_LIT(cl, i));
		
		
		const uint32_t s = VEC_SIZE( CL_REF, binOcc[ index(current)] );
		for( uint32_t j = 0 ; j < s; ++j ){
			CLAUSE& jcl = gsa.get( binOcc[ index(current)][j] );
			const lit_t fl = CL_GET_LIT(jcl,0);
			const lit_t sl = CL_GET_LIT(jcl,1);
			seen[ fl != current ? index(fl) : index(sl)] = 1;
		}
		
		uint32_t count = 0;
		for( uint32_t j = 0; j < size; ++j ){
			if( j == i ) continue;
			const lit_t jl = CL_GET_LIT(cl, j);
			if( seen[ index(inv_lit(jl)) ] == 1 ) count ++;
		}

		if( count < size -1 ) return false;	
		
		
		for( uint32_t j = 0 ; j < s; ++j ){
			CLAUSE& jcl = gsa.get( binOcc[ index(current)][j] );
			const lit_t fl = CL_GET_LIT(jcl,0);
			const lit_t sl = CL_GET_LIT(jcl,1);
			seen[ fl != current ? index(fl) : index(sl)] = 0;
		}
	}
	return true;
}

bool FormulaCsps::binXor(CL_REF c){
	const CLAUSE& cl = gsa.get(c);
	const uint32_t size = CL_SIZE( cl );
	if( size != 2 ) return false;
	
	const lit_t invfl = inv_lit( CL_GET_LIT(cl,0) );
	const lit_t invsl = inv_lit( CL_GET_LIT(cl,1) );
	
	const uint32_t s = VEC_SIZE( CL_REF, binOcc[ index(invfl)] );
	for( uint32_t j = 0 ; j < s; ++j ){
		CLAUSE& jcl = gsa.get( binOcc[ index(invfl)][j] );
		const lit_t jfl = CL_GET_LIT(jcl,0);
		const lit_t jsl = CL_GET_LIT(jcl,1);
		if( jfl == invfl && jsl == invsl ) return true;
		if( jfl == invsl && jsl == invfl ) return true;
	}
	return false;
}

void FormulaCsps::FindXor( VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt ){
	VEC_TYPE( CL_REF )& cls1 = *clause_set;
	cerr << "c start find xor" << endl;
	
	VEC_TYPE (CL_REF) table;

	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF,cls1); ++i) {
		const CL_REF c = cls1[i];
		CLAUSE& cl = gsa.get(c);
		if( CL_SIZE(cl) < 3 ) continue;
		
		VEC_PUSH_BACK( CL_REF, table, c );
		const uint32_t size = CL_SIZE(cl);
		
		for( uint32_t j = 1; j<size; ++j ){
			if( CL_GET_LIT(cl,j-1) > CL_GET_LIT(cl,j) ){
				CL_SORT( cl );
				break;
			}
		}
	}
	
	cerr << "c relevant clauses: " << VEC_SIZE(CL_REF,table) << endl;
	uint32_t xors = 0;
	
	uint32_t cL = 3;	
	uint32_t cP = 0;	
	uint32_t xorClauses=0;	
	
	sort(table);
	
	
	while( cP < VEC_SIZE (CL_REF, table) && cL < 63){
		uint32_t start = cP;
		cL = CL_SIZE( gsa.get( table[cP] ) );
		
		while( cP < VEC_SIZE(CL_REF,table) && CL_SIZE( gsa.get( table[cP] ) ) == cL ) cP ++;
	
		
		for (uint32_t i = start; i < cP-1; ++i) {
			const CL_REF c = table[i];
			const CLAUSE& cl = gsa.get(c);
			uint32_t stop = i + 1;
			
			for ( ; stop < cP; ++stop) {
				const CL_REF c2 = table[stop];
				const CLAUSE& cl2 = gsa.get(c2);
				uint32_t k = 0;
				for( ; k < CL_SIZE(cl2); k ++ ){
					if( var(CL_GET_LIT(cl,k)) != var(CL_GET_LIT(cl2,k)) ) break;
				}
				if( k != CL_SIZE(cl2) ) break;
			}
			
			
			uint64_t shift = 1;
			shift = shift << ( CL_SIZE(cl) -1 );

			
			if( stop - start >= shift ){
				
				
				uint32_t odd = 0;
				uint32_t count = 0;
				
				for( uint32_t j = 0 ; j < CL_SIZE(cl); ++j){
					if( pol( CL_GET_LIT(cl,j) ) == NEG ) odd = odd ^ 1;
				}
				
				for( uint32_t j = i ; j < stop; ++j){
					const CL_REF c2 = table[j];
					const CLAUSE& cl2 = gsa.get(c2);
					uint32_t o = 0;
					for( uint32_t k = 0; k < CL_SIZE(cl2); k ++ )
						if( pol( CL_GET_LIT(cl2,k) ) == NEG ) o = o ^ 1;
					if( o == odd ) count ++;
				}
				if( count == shift ){
	
					xors ++;
					xorClauses += count;
				}
				
				start = stop;
			}
		}
		
		
		cL ++;
	}

	cerr << "c found " << xors << " non-binary xors encoded with " << xorClauses << " clauses" << endl;
}

void FormulaCsps::sort(VEC_TYPE( CL_REF )& clauses){

	uint32_t n = VEC_SIZE(CL_REF,clauses);
	int32_t m, s;
	
	CL_REF* a=new CL_REF[ n ];
	for( uint32_t i = 0 ; i < n; i++ ){
		a[i] = clauses[i];
	}
	CL_REF* original = a;	
	CL_REF* b=new CL_REF[ n ];

	
	for (s=1; s<n; s+=s)
	{
		m = n;
		do {
			m = m - 2*s;	
			int32_t hi = (m+s > 0) ? m + s : 0;	
			
			int32_t i = (m > 0) ? m : 0;	
			int32_t j = hi;
			
			int32_t stopb = m + 2*s;	
			int32_t currentb = i;			
			
			
			while( i < hi && j < stopb)
			{
				if( ( gsa.get(a[i]) ) < ( gsa.get(a[j]))  )
					b[currentb++] = a[i++];
				else
					b[currentb++] = a[j++];
			}
			
			for( ; i < hi; )
				b[currentb++] = a[i++];
				
			for( ; j< stopb; 	)
				b[currentb++] = a[j++];
				
		} while( m > 0 );
		
		
		CL_REF* tmp = a;
		a = b;
		b = tmp;
	}
	
	delete [] b;
	
	for( uint32_t i = 0 ; i < n; i++ ){
		clauses[i] = a[i];
	}
	return;
}


int FormulaCsps::litRatio(CL_REF c){
	const CLAUSE& cl = gsa.get(c);
	const uint32_t size = CL_SIZE( cl );

	uint32_t n = 0;	
	for( uint32_t i = 0 ; i < size; i++ ){
		if( pol( CL_GET_LIT(cl,i) ) == NEG ) n ++;
	}
	
	if( n == 0 ) return 2;
	if( n == 1 ) return -1;
	if( n == size - 1 ) return 1;
	if( n == size ) return -2;
	return 0;
}

bool FormulaCsps::dISaANDb(CL_REF c){
	const CLAUSE& cl = gsa.get(c);
	const uint32_t size = CL_SIZE( cl );
	if( size != 3 ) return false;
	
	
	
	lit_t oinv[2];
	for( uint32_t i = 0 ; i < 3; ++i ){
		lit_t current = CL_GET_LIT(cl,i);
		
		uint32_t t = 0;
		for( uint32_t j = 0 ; j < 3; ++j ){
			if( current != CL_GET_LIT(cl,j) ) oinv[t++] = inv_lit(CL_GET_LIT(cl,j));
		}
		
		current = inv_lit(current);
		
		const uint32_t s = VEC_SIZE( CL_REF, binOcc[ index(current) ] );
		for( uint32_t j = 0 ; j < s; ++j ){
			CLAUSE& jcl = gsa.get( binOcc[ index(current) ][j] );
			const lit_t olit = current ^ CL_GET_LIT( jcl,0 ) ^ CL_GET_LIT( jcl,1 );	
			
			for( uint32_t ti = 0; ti < 2; ti++){
				if( olit == oinv[ti] ) oinv[ti] = NO_LIT;
			}
		}
		
		if( oinv[0] == NO_LIT && oinv[1] == NO_LIT ) return true;
	}
	
	return false;
}
