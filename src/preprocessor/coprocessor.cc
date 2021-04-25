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


#include "preprocessor/coprocessor.h"

Coprocessor::Coprocessor (VEC_TYPE (CL_REF) * clause_set, uint32_t varCnt, const StringMap & commandline):
debug(0),
print_dimacs (false),
simpDiff(300),
maxCls( 6000000 ),
maxVEcls( 5000000 ),	
maxQsize( 1000000 ),		
hteP( 10 ),
blockMaxSize( -1 ),
blockMaxOcc( 20 ),
createBinaries(false),
vePropagate( true ),
probVars(5000),
probVarsP(10),
probClVars(10),
probClMaxS(3),
probClMaxN(100),
erMaxV(15000),
erMinV(10000),
erPairs(50),
erMinSize(0),
erMinO(3),
erTries(10),
erIters(6),
aMinSize(8),
aPercent(60),
simpEvery( 20000 ),
dumpBiGraph(false),
pure(1),
prop_pure(0),
blocked(2),
ve(1),
hteR(2),
ee(2),
er(1),
probSize(3),
probL(1),
probC(0),
asymm(1),
ternary(0),
rounds(2),
simpPure(false),
simpBlocked(true),
simpVe(false),
simpHte(true),
simpEe(true),
simpSubsSimp(true),
simpProbL(false),
simpProbC(false),
simpPAsymm(false),
ppTimeout(-1),	
simpTimeout(-1),	
usePrefetch(false),
addedBinaries(0),

solution( UNKNOWN ),
maxClLen(0),
var_cnt( varCnt),
simplifyRejections(0),
max_solutions(1),
lastTrailSize( 0 ),
propagation( varCnt ),
cmd( commandline)
{
	
	vOcc = (uint32_t*) malloc( sizeof(uint32_t) * (var_cnt+1) );
	memset(vOcc, 0 , sizeof(uint32_t) * (var_cnt+1) );
	
	lOcc = (uint32_t*) malloc(  sizeof(uint32_t) * max_index( var_cnt)  );
	memset( lOcc, 0, sizeof(uint32_t) * max_index( var_cnt) );
	
	lAssi = (void**) malloc(  sizeof(assi_t) * max_index( var_cnt) );
	memset( lAssi, 0, sizeof(assi_t) * max_index( var_cnt) );

	tmpAssi = assi_create(var_cnt);

	
	resQ = cpp_heap < uint32_t, less < uint32_t > >(&vOcc, var_cnt + 1);
	litHeap = cpp_heap<lit_t, less<uint32_t> > ( &lOcc, max_index(var_cnt ) );
	
	VEC_CREATE (lit_t, unitQ);
	VEC_CREATE (CL_REF, clausesR);
	
	VEC_CREATE (CL_REF, subsQ);
	VEC_RESERVE (CL_REF, subsQ, var_cnt + 1);
	VEC_CREATE (lit_t, tmpLits);
	VEC_CREATE (lit_t, tmpLits2);

	
	VEC_CREATE_SPACE (VEC_TYPE (CL_REF), occ, max_index(var_cnt ) );
	VEC_CLEAR (VEC_TYPE (CL_REF), occ);
	for (uint32_t i = 0; i < max_index(var_cnt ) + 1; ++i) {
		VEC_TYPE (CL_REF) tmp;
		VEC_CREATE (CL_REF, tmp);
		VEC_PUSH_BACK (VEC_TYPE (CL_REF), occ, tmp);
	}

	
	replacedBy.resize(var_cnt + 1, NO_LIT );
	for( var_t var1 = 0 ; var1 < var_cnt+1; var1 ++ ){
		replacedBy[ var1 ] = lit( var1, POS );
	}
	varRemoved = boolarray_create (var_cnt + 1);

	
	touchedVars = boolarray_create (var_cnt + 1);
	eliminated = boolarray_create (var_cnt + 1);
	elimWhite = boolarray_create (var_cnt + 1);
	
	

	set_parameter (commandline);
	
	propagation.setMax(probSize);
	
	simplifying = false;

	
	VEC_CREATE_SPACE (uint32_t, pureLits, rounds+1);
	VEC_RESIZE (uint32_t, pureLits, rounds+1, 0);
	VEC_CREATE_SPACE (uint32_t, removedBlocked, rounds+1);
	VEC_RESIZE (uint32_t, removedBlocked, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, removedVars, rounds+1);
	VEC_RESIZE (uint32_t, removedVars, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, removedVarCls, rounds+1);
	VEC_RESIZE (uint32_t, removedVarCls, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, removedSubsCls, rounds+1);
	VEC_RESIZE (uint32_t, removedSubsCls, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, removedEquiCls, rounds+1);
	VEC_RESIZE (uint32_t, removedEquiCls, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, foundProbUnits, rounds+1);
	VEC_RESIZE (uint32_t, foundProbUnits, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, asymmNewCls, rounds+1);
	VEC_RESIZE (uint32_t, asymmNewCls, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, hteRemoved, rounds+1);
	VEC_RESIZE (uint32_t, hteRemoved, rounds+1,0);
	VEC_CREATE_SPACE (uint32_t, hteUnits, rounds+1);
	VEC_RESIZE (uint32_t, hteUnits, rounds+1,0);
	
	VEC_CREATE_SPACE (uint32_t, removedPropCls, rounds+2);
	VEC_RESIZE (uint32_t, removedPropCls, rounds+2,0);
	
	
#ifdef TRACK_CLAUSE
	cerr << "literal to track: " << endl;
	trackme = NO_LIT;
	int32_t number;
	cin >> number;
	trackme = lit(number);
#endif
}

Coprocessor::~Coprocessor(){
#ifndef SILENT
	cerr << "c ==== simplify results ====" << endl;
		cerr << "c cls pure"; for(uint32_t i = VEC_SIZE( uint32_t, pureLits)-1; i < VEC_SIZE( uint32_t, pureLits); ++i ) cerr << " " << pureLits[i]; cerr << endl;
		cerr << "c BCE -cls"; for(uint32_t i = VEC_SIZE( uint32_t, removedBlocked)-1; i < VEC_SIZE( uint32_t, removedBlocked); ++i ) cerr << " " << removedBlocked[i]; cerr << endl;
		cerr << "c HTE -cls "; for(uint32_t i= VEC_SIZE( uint32_t, hteRemoved)-1; i < VEC_SIZE( uint32_t, hteRemoved); ++i ) cerr << " " << hteRemoved[i]; cerr << endl;
		cerr << "c HTE -unit"; for(uint32_t i= VEC_SIZE( uint32_t, hteUnits)-1; i < VEC_SIZE( uint32_t, hteUnits); ++i ) cerr << " " << hteUnits[i]; cerr << endl;
		cerr << "c VE -var "; for(uint32_t i = VEC_SIZE( uint32_t, removedVars)-1; i < VEC_SIZE( uint32_t, removedVars); ++i ) cerr << " " << removedVars[i]; cerr << endl;
		cerr << "c VE -cls "; for(uint32_t i = VEC_SIZE( uint32_t, removedVarCls)-1; i < VEC_SIZE( uint32_t, removedVarCls); ++i ) cerr << " " << removedVarCls[i]; cerr << endl;
		cerr << "c SUB -cls"; for(uint32_t i = VEC_SIZE( uint32_t, removedSubsCls)-1; i < VEC_SIZE( uint32_t, removedSubsCls); ++i ) cerr << " " << removedSubsCls[i]; cerr << endl;
		cerr << "c EQU -cls"; for(uint32_t i = VEC_SIZE( uint32_t, removedEquiCls)-1; i < VEC_SIZE( uint32_t, removedEquiCls); ++i ) cerr << " " << removedEquiCls[i]; cerr << endl;
		cerr << "c PRB -var"; for(uint32_t i = VEC_SIZE( uint32_t, foundProbUnits)-1; i < VEC_SIZE( uint32_t, foundProbUnits); ++i ) cerr << " " << foundProbUnits[i]; cerr << endl;
		cerr << "c PRB +cls"; for(uint32_t i = VEC_SIZE( uint32_t, asymmNewCls)-1; i < VEC_SIZE( uint32_t, asymmNewCls); ++i ) cerr << " " << asymmNewCls[i]; cerr << endl;
#endif
	
	
		for (int32_t j = VEC_SIZE (postEle, postProcessStack) - 1; j >= 0; --j) {
			const postEle pe = postProcessStack[j];
			if (pe.kind == 'b') {
				CLAUSE & cl = gsa.get (pe.b.clause);
				CL_DESTROY( cl );
				gsa.release (pe.b.clause);
			} else {
				if (pe.kind == 'v') {
					const elimination_t & elim = pe.e;
					for (uint32_t k=0; k < elim.number; ++k) {
						uint32_t ind;
						CLAUSE & cl = gsa.get (elim.clauses[k]);
						CL_DESTROY( cl );
						gsa.release (elim.clauses[k]);
					}
					free( elim.clauses );
				}
			}
		}				
		
	
	freeStructures ();	
}

void Coprocessor::extend( uint32_t newVar ){

	if( debug > 1 )	cerr << "c extend all coprocessor structures to " << newVar << endl;
	
	
	vOcc = (uint32_t*) realloc( vOcc, sizeof(uint32_t) * (newVar+1) );
	for( var_t v = var_cnt; v <= newVar; ++v ) vOcc[v] = 0;
	
	lOcc = (uint32_t*) realloc( lOcc, sizeof(uint32_t) * max_index( newVar) );
	for( lit_t l = max_index(var_cnt); l < max_index(newVar); ++l ){
		lOcc[ index(l) ] = 0;
	}

	lAssi = (void**) realloc( lAssi, max_index( newVar) * sizeof(assi_t*));
	for( lit_t l = max_index(var_cnt); l < max_index(newVar); ++l ){
		lAssi[ index(l) ] = 0;
	}

	assi_extend(tmpAssi, var_cnt, newVar);

	propagation.extend(newVar);

	
	resQ.resize(newVar + 1,&vOcc);
	litHeap.resize( max_index(newVar) ,&lOcc);

	
	VEC_RESERVE (CL_REF, subsQ, newVar);

	
	VEC_RESERVE( VEC_TYPE (CL_REF), occ, max_index(newVar) );
	while( VEC_SIZE(VEC_TYPE (CL_REF), occ) < max_index(newVar ) + 1) {
		VEC_TYPE (CL_REF) tmp;
		VEC_CREATE (CL_REF, tmp);
		VEC_PUSH_BACK (VEC_TYPE (CL_REF), occ, tmp);
	}

	
	replacedBy.resize(newVar + 1, NO_LIT );
	for( var_t var1 = var_cnt+1 ; var1 < newVar+1; var1 ++ ){
		replacedBy[ var1 ] = lit( var1, POS );
	}

	
	boolarray_extend(varRemoved,var_cnt, newVar + 1);
	boolarray_extend(touchedVars, var_cnt,newVar + 1);
	boolarray_extend(eliminated , var_cnt,newVar + 1);
	boolarray_extend(elimWhite , var_cnt,newVar + 1);

	
	
	data->extend(newVar);
	var_cnt = newVar;
}

solution_t Coprocessor::preprocess( searchData& search, VEC_TYPE( CL_REF )* formula){

	data = &search;
	
	stopTime = get_microseconds() + ppTimeout * 1000;
	if( ppTimeout == -1 ){
		stopTime = -1;
	}

	clauses = formula;
	int32_t oldCls = VEC_SIZE( CL_REF, (*formula) );
	
	
	if( maxCls != -1 && maxCls < (int32_t)oldCls ) return UNKNOWN;
	
	
	solution = fillOcc(search.assi);
	if( solution != UNKNOWN ) return solution;
#ifndef SILENT
	cerr << "c rounds: " << rounds << " with p(" << pure << "),pp(" << prop_pure
		 << "),hte(" << hteR << "),bce(" << blocked << "),ve(" << ve << "),ee(" << ee << "),probL("<<probL<<"),probC("<<probC<<"), asymm("<<asymm<<"), " <<
		 "ternary(" << ternary << ") er(" << er << ")" << endl;
#endif

	for( cRound = 0 ; solution == UNKNOWN && cRound < rounds; ++cRound ){

		if(cRound < pure){
#ifndef SILENT
			cerr << "c remove pure" << endl;
#endif
			detectPure (search.assi);
			if( debug > 0 ) scanClauses( search.assi );
		}
		if( stopTime > get_microseconds() && cRound < prop_pure){
#ifndef SILENT
			cerr << "c propagate pure literal" << endl;
#endif
			solution = propagate( search.assi );
			if( solution != UNKNOWN ) break;
			reduceClauses ();
		}
		
		
		if(stopTime > get_microseconds() && cRound < ee){
#ifndef SILENT
			cerr << "c elim equivalences" << endl;
#endif
			if( searchEqui (search.assi) ){
				solution = applyEqui(search.assi);
				if( solution != UNKNOWN ) break;
			}
			if( debug > 0 ) scanClauses( search.assi );
		}
		
		if(stopTime > get_microseconds() && cRound < hteR){
#ifndef SILENT
			cerr << "c hte" << endl;
#endif
			
			solution = propagate( search.assi );
			if( solution != UNKNOWN ) break;
			
			solution = hte (search.assi, ( ( hteP * var_cnt) / 50.0f ));
			if( solution != UNKNOWN ) break;
			
			
			solution = propagate( search.assi );
			if( solution != UNKNOWN ) break;
		
			if( debug > 0 ) scanClauses( search.assi );
		}

		if(stopTime > get_microseconds() && cRound < blocked){
#ifndef SILENT
		 	cerr << "c remove block" << endl;
#endif
			solution = remove_blocked (search.assi);
			if( solution != UNKNOWN ) break;
			if( debug > 0 ) scanClauses( search.assi );
		}

		if(stopTime > get_microseconds() &&  (cRound < ve && (maxVEcls == -1 || maxVEcls >= (int32_t)oldCls )) ){
#ifndef SILENT
			cerr << "c elim variables" << endl;
#endif
			solution = eliminate_variables (search.assi);
			if( solution != UNKNOWN ) break;
			if( debug > 0 ) scanClauses( search.assi );
		}

		
		if(stopTime > get_microseconds() &&  (cRound < probL || cRound <probC || cRound <asymm )){
			solution = hyberBinProb(search.assi);
			if( solution != UNKNOWN ) break;

			for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
				
				CLAUSE & cl = gsa.get ((*clauses)[i]);
				if( CL_IS_IGNORED( cl ) ) continue;
				
				VEC_PUSH_BACK (CL_REF, subsQ, (*clauses)[i]);
			}
#ifndef SILENT
			cerr << "c subsumption and simplification" << endl;
#endif
			subsSimp(search.assi);
			if( debug > 0 ) scanClauses( search.assi );
		}
		if( stopTime > get_microseconds() && cRound < ternary ){
			#ifndef SILENT
				cerr << "c ternary" << endl;
			#endif
			solution = ternaryResolution(search.assi);
			if( solution != UNKNOWN ) break;
			if( debug > 0 ) scanClauses( search.assi );
		}
		if( stopTime > get_microseconds() && cRound < er ){
			#ifndef SILENT
				cerr << "c extended resolution" << endl;
			#endif
			solution = extendedResolution(search);
			if( solution != UNKNOWN ) break;
			if( debug > 0 ) scanClauses( search.assi );
		}

		
		if( ve > cRound && cRound + 1< rounds){
#ifndef SILENT
			cerr << "c clean occs" << endl;
#endif
			remIgnClOcc();
		}
	}
	
	
	if( solution == UNKNOWN ){
		solution = propagate (search.assi);
		
		reduceClauses ();
	}
	
	
	if( blackFile.size() != 0 ){
		VarFileParser vfp( blackFile );
		VEC_TYPE(var_t) evars;
		VEC_CREATE(var_t,evars);

		vfp.extract(evars);

		cerr << "c force variable elimination of " << VEC_SIZE(var_t,evars) << " variables" << endl;
		for( uint32_t i = 0 ; i < VEC_SIZE(var_t,evars); ++i ){
			
			resolveVar(search.assi, evars[i], (print_dimacs)? 2 : 1 );
		}
		reduceClauses ();
	}
#ifndef SILENT
	cerr << "c ==== pp results ====" << endl;
	cerr << "c cls pure "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, pureLits)-1; ++i ) cerr << " " << pureLits[i]; cerr << endl;
	cerr << "c BCE -cls "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, removedBlocked)-1; ++i ) cerr << " " << removedBlocked[i]; cerr << endl;
	cerr << "c VE -var  "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, removedVars)-1; ++i ) cerr << " " << removedVars[i]; cerr << endl;
	cerr << "c VE -cls  "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, removedVarCls)-1; ++i ) cerr << " " << removedVarCls[i]; cerr << endl;
	cerr << "c SUB -cls "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, removedSubsCls)-1; ++i ) cerr << " " << removedSubsCls[i]; cerr << endl;
	cerr << "c EQU -cls "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, removedEquiCls)-1; ++i ) cerr << " " << removedEquiCls[i]; cerr << endl;
	cerr << "c HTE -cls "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, hteRemoved)-1; ++i ) cerr << " " << hteRemoved[i]; cerr << endl;
	cerr << "c HTE -unit"; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, hteUnits)-1; ++i ) cerr << " " << hteUnits[i]; cerr << endl;
	cerr << "c PRB -var "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, foundProbUnits)-1; ++i ) cerr << " " << foundProbUnits[i]; cerr << endl;
	cerr << "c PRB +cls "; for(uint32_t i = 0; i < VEC_SIZE( uint32_t, asymmNewCls)-1; ++i ) cerr << " " << asymmNewCls[i]; cerr << endl;
	cerr << "c PRB +bin " << addedBinaries << endl;
	cerr << "c PRO -cls "; for(uint32_t i = 0; i < VEC_SIZE( CL_REF, removedPropCls); ++i ) cerr << " " << removedPropCls[i]; cerr << endl;
	cerr << "c formula diff: " << oldCls - (int32_t)VEC_SIZE( CL_REF, (*formula) ) << " clauses" << endl;
	cerr << "c UNDO stack:   " << postProcessStack.size() << endl;
	cerr << "c return cls:   " << VEC_SIZE( CL_REF, (*formula) ) << endl;
	switch( solution ){
		case UNKNOWN: cerr << "c solution: UNKNOWN" << endl; break;
		case UNSAT: cerr << "c solution: UNSAT" << endl; break;
		case SAT: cerr << "c solution: SAT" << endl; break;
	}
	cerr << "c ====================" << endl;
#endif
	if (print_dimacs) {
		uint32_t unitClauses = 0;
		
		for( var_t v = 1; v <= var_cnt; v++ ) if( ! assi_is_undef( search.assi,  v) ) unitClauses ++ ;
		cout << "p cnf " << var_cnt << " " << unitClauses + VEC_SIZE( CL_REF, (*formula) ) << endl;
		
		for( var_t v = 1; v <= var_cnt; v++ ){
			if( ! assi_is_undef( search.assi,  v) ) cout << nr( lit(v, assi_get_polarity( search.assi,v ) ) ) << " 0" << endl;
		}
		
		for (uint32_t j = 0; j < VEC_SIZE (CL_REF, (*formula)); ++j) {
			CLAUSE & cl = gsa.get ((*formula)[j]);
			for (uint32_t i = 0; i < CL_SIZE (cl); ++i) cout << nr (CL_GET_LIT (cl, i)) << " ";
			cout << "0" << endl;
		}
		exit (12);
	}
	return solution;
}

struct mysorter_s
{
	bool operator	 () (lit_t i, lit_t j)
	{
		return var (i) < var (j);
	}
};

void Coprocessor::freeStructures ()
{
	resetAssi();
	free( vOcc);
	free( lOcc);
	free( lAssi);
	VEC_CLEAR (lit_t, unitQ);
	VEC_CLEAR (CL_REF, subsQ);
	
	assi_destroy(tmpAssi, var_cnt);
	
	boolarray_destroy (varRemoved);
	boolarray_destroy (touchedVars);
	boolarray_destroy (eliminated);
	boolarray_destroy (elimWhite);
	for (uint32_t i = 0; i < max_index(var_cnt ) + 1; ++i) {
		VEC_DESTROY (CL_REF, occ[i]);
	}
	VEC_DESTROY (VEC_TYPE (CL_REF), occ);
}

void Coprocessor::addActiveCls()
{
	
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		CLAUSE & cl = gsa.get ((*clauses)[i]);
		if (!CL_IS_IGNORED (cl)) {
			for (uint32_t j = 0; j < CL_SIZE (cl); ++j) {
				if (boolarray_get (touchedVars, var (CL_GET_LIT (cl, j)))) {
					VEC_PUSH_BACK (CL_REF, subsQ, (*clauses)[i]);
					break;
				}
			}
		}
	}

	for (uint32_t v = 1; v <= var_cnt; ++v) {
		if (boolarray_get (touchedVars, v)) {
			if (resQ.contains (v)) resQ.update_item (v);
			else resQ.insert_item (v);
		}
		boolarray_set (touchedVars, v, false);
	}

}

void Coprocessor::remClOcc (CL_REF clause)
{
	CLAUSE & cl = gsa.get (clause);
	for (uint32_t j = 0; j < CL_SIZE (cl); j++) {
		const lit_t literal = CL_GET_LIT (cl, j);
		const var_t variable = var (literal);
		
#ifdef TRACK_CLAUSE
		if( variable == var( trackme ) ){
			cerr << "c remove clause from occurences:";
			if( CL_IS_IGNORED(cl) ) cerr << "(ignored)";
			for( uint32_t i = 0; i < CL_SIZE(cl); i++ ) cerr << " " << nr( CL_GET_LIT(cl,i) );
			cerr << endl;
		}
		if( cl.getFlag4() ) cerr << "c remove tracked clause from occurence lists" << endl;
#endif

		
		
		
		
		
		
		lOcc[ index(literal) ] = (lOcc[ index(literal) ] == 0) ? 0 : lOcc[ index(literal) ] -1;
		vOcc[variable]--;
		
		if (resQ.contains (variable)) {
			if (vOcc[variable] > 0) {
				resQ.update_item (variable);
			} else {
				resQ.remove_item (variable);
			}
		}
		
		for( uint32_t i = 0; i < VEC_SIZE (CL_REF, occ[ index(literal) ]); ++i ){
			if (clause == occ[ index(literal) ][i]) {
				
				VEC_ERASE_NO_ORDER (CL_REF, occ[ index(literal) ], i);
				break;
			}
		}
	}

}

void Coprocessor::remIgnClOcc(){
	for( var_t v = 1; v <= var_cnt; ++v ){
		const lit_t l1 = lit( v, POS);

		prefetchOcc( inv_lit(l1) );

		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF,occ[index(l1)]); ++j ){
			CLAUSE& cl = gsa.get( occ[index(l1)][j]);
			if( CL_IS_IGNORED(cl) ){
#ifdef TRACK_CLAUSE
				if( cl.getFlag4() ) cerr << "c remove tracked clause because it is ignored" << endl;
#endif
				VEC_ERASE_NO_ORDER(CL_REF, occ[index(l1)],j);
				j--;
			}
		}
		const lit_t l2 = lit( v, NEG);
		for(uint32_t j = 0 ; j < VEC_SIZE( CL_REF,occ[index(l2)]); ++j ){
			CLAUSE& cl = gsa.get( occ[index(l2)][j]);
			if( CL_IS_IGNORED(cl) ){
#ifdef TRACK_CLAUSE
				if( cl.getFlag4() ) cerr << "c remove tracked clause because it is ignored" << endl;
#endif
				VEC_ERASE_NO_ORDER(CL_REF, occ[index(l2)],j);
				j--;
			}
		}
	}
}

solution_t Coprocessor::resolveVar (assi_t assignment, var_t var1, int force)
{
	if (assi_get_polarity (assignment, var1) != UNDEF) return UNKNOWN;

	if ( boolarray_get(elimWhite,var1) ) return UNKNOWN;

#ifdef TRACK_CLAUSE
	
	if( var1 == var( trackme) ) cerr << "c try to resolve tracked variable " << var1 << endl;
	
#endif

	if( boolarray_get( eliminated, var1) ) return UNKNOWN;

	const uint32_t number_of_old_clauses = VEC_SIZE (CL_REF, (*clauses));
	
	prefetchOcc( lit(var1,POS) );
	prefetchOcc( lit(var1,NEG) );
	VEC_TYPE (CL_REF) & clauses_pos = occ[ index( lit(var1,POS) )];
	VEC_TYPE (CL_REF) & clauses_neg = occ[ index( lit(var1,NEG) )];

	uint32_t total;
#define IGNORE_IGNORED
#ifdef IGNORE_IGNORED
	total = VEC_SIZE (CL_REF, clauses_pos) + VEC_SIZE (CL_REF, clauses_neg);
#else
	total = 0;
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, clauses_pos); i++) {
		const CLAUSE& c1 = gsa.get (clauses_pos[i]);
		total = CL_IS_IGNORED( c1) ? total : total + 1;
	}
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, clauses_neg); i++) {
		const CLAUSE& c1 = gsa.get (clauses_neg[i]);
		total = CL_IS_IGNORED( c1) ? total : total + 1;
	}
#endif

	
	

	
	uint32_t newCls = 0;

	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, clauses_pos); i++) {
		
		const CLAUSE& c1 = gsa.get (clauses_pos[i]);
		if( CL_IS_IGNORED( c1) ) continue;
		for (uint32_t j = 0; j < VEC_SIZE (CL_REF, clauses_neg); j++) {
			
			const CLAUSE& c2 = gsa.get (clauses_neg[j]);
			if( CL_IS_IGNORED( c2) ) continue;
			if (CL_CHECK_RESOLVE ( c1, c2, var1)) ++newCls;
			
			if (newCls > total && force == 0) return UNKNOWN;
		}
	}
	if (newCls > total && force == 0) return UNKNOWN;

#ifdef TRACK_CLAUSE
	if( var1 == var( trackme) ){
		cerr << "c eliminate tracked variable " << var1 << endl;
		cerr << "c resolvents:" << endl;
	}
#endif

	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, clauses_pos); i++) {
		
		const CLAUSE& c1 = gsa.get (clauses_pos[i]);
		if( CL_IS_IGNORED( c1) ) continue;
		for (uint32_t j = 0; j < VEC_SIZE (CL_REF, clauses_neg); j++) {
			
			const CLAUSE& c2 = gsa.get (clauses_neg[j]);
			if( CL_IS_IGNORED( c2) ) continue;

			CLAUSE *cp = CL_RESOLVE_VAR (gsa.get (clauses_pos[i]), gsa.get (clauses_neg[j]), var1);
			if (cp != 0) {
				CL_REF resolve_clause = gsa.create (*cp);
				VEC_PUSH_BACK (CL_REF, (*clauses), resolve_clause);
				maxClLen = (maxClLen >= CL_SIZE (*cp)) ? maxClLen : CL_SIZE (*cp);
#ifdef TRACK_CLAUSE
				if( var1 == var( trackme ) ){
					cerr << "c";
					for( uint32_t k=0; k < CL_SIZE(*cp); k++ ) cerr << " " << nr( CL_GET_LIT( *cp,k));
					cerr << endl;
				}
#endif
				delete cp;
			}
		}
	}

	
	
	removedVars[cRound] ++;
	removedVarCls[cRound] += total - newCls;
	boolarray_set( eliminated, var1, true );
	boolarray_set( data->eliminated, var1, true );	
	uint32_t ci = 0;
	postEle elimination;
	if( force < 2 ){
		elimination.kind = 'v';
		elimination.e.var1 = var1;
		elimination.e.number = total;
		elimination.e.clauses = (CL_REF *) malloc (total * sizeof (CL_REF));
		boolarray_set( varRemoved, var1, true );
	}


#ifdef TRACK_CLAUSE
		if( var1 == var( trackme ) ) cerr << "c clauses to remove:" << endl;
#endif
	
	while (VEC_SIZE (CL_REF, clauses_pos) > 0) {
		const CL_REF clause = clauses_pos[0];
		CLAUSE& icl = gsa.get (clause);

#ifdef TRACK_CLAUSE
		if( var1 == var( trackme ) ){
			cerr << "c";
			if( CL_IS_IGNORED(icl) ) cerr << "(ignored)";
			for( uint32_t k=0; k < CL_SIZE(icl); k++ ) cerr << " " << nr( CL_GET_LIT( icl,k));
			cerr << endl;
		}
#endif

		if( CL_IS_IGNORED(icl)){
			VEC_ERASE_NO_ORDER(CL_REF, clauses_pos, 0 );
			continue;
		}


		CL_IGNORE (icl);
		remClOcc (clause);	
		
		if( force < 2 ) elimination.e.clauses[ci++] = clause;
	}

	while (VEC_SIZE (CL_REF, clauses_neg) > 0) {
		const CL_REF clause = clauses_neg[0];
		CLAUSE& icl = gsa.get (clause);
		
#ifdef TRACK_CLAUSE
		if( var1 == var( trackme ) ){
			cerr << "c";
			if( CL_IS_IGNORED(icl) ) cerr << "(ignored)";
			for( uint32_t k=0; k < CL_SIZE(icl); k++ ) cerr << " " << nr( CL_GET_LIT( icl,k));
			cerr << endl;
		}
#endif
		
		if( CL_IS_IGNORED(icl)){
			VEC_ERASE_NO_ORDER(CL_REF, clauses_neg, 0 );
			continue;
		}

		CL_IGNORE (icl);
		remClOcc (clause);	

		if( force < 2 ) elimination.e.clauses[ci++] = clause;
	}
	postProcessStack.push_back (elimination);

	for (uint32_t i = number_of_old_clauses; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		const CL_REF clause = (*clauses)[i];
		CLAUSE & cl = gsa.get (clause);
		if (CL_SIZE (cl) == 1){
			if( !enqueue(assignment,  CL_GET_LIT (cl, 0))) return UNSAT;
		}

		VEC_PUSH_BACK (CL_REF, subsQ, clause);

		for (uint32_t j = 0; j < CL_SIZE (cl); ++j) {
			const lit_t literal = CL_GET_LIT (cl, j);
			const var_t variable = var (literal);

			vOcc[variable]++;
			lOcc[ index(literal) ]++;

			VEC_PUSH_BACK (CL_REF, occ[ index(literal)], clause);		

			if (resQ.contains (variable)) resQ.update_item (variable);
			else resQ.insert_item (variable);
		}
	}
	return subsSimp (assignment);
}

solution_t Coprocessor::eliminate_variables(assi_t assignment){
	boolarray_clear(touchedVars, var_cnt+1);
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		
		CLAUSE & cl = gsa.get ((*clauses)[i]);
		if( CL_IS_IGNORED( cl ) ) continue;
		for (uint32_t j = 0; j < CL_SIZE (cl); ++j)
			boolarray_set (touchedVars, var (CL_GET_LIT (cl, j)), true);
		
		VEC_PUSH_BACK (CL_REF, subsQ, (*clauses)[i]);
	}
	
	resQ.clear();
	
	for( var_t v = 1; v <= var_cnt; ++v ){
		if( vOcc[v] > 0 ) resQ.insert_item(v);
	}

	while ( (VEC_SIZE (CL_REF, subsQ) > 0 || !resQ.is_empty ()) && stopTime > get_microseconds()) {
		
		solution = subsSimp (assignment);
		if (solution != UNKNOWN) return solution;
		
		
		if( vePropagate || simplifying){
			solution = propagate (assignment);
			if (solution != UNKNOWN) return solution;
		}
		
		if( simplifying ){
			if( VEC_SIZE(lit_t, unitQ) > 0 ) cerr << "c found " << VEC_SIZE(lit_t, unitQ) << " units during ve" << endl;
		}
		
		while (!resQ.is_empty ()) {
			const var_t variable = resQ.item_at (0);
			resQ.remove_item (variable);
			
			solution = resolveVar (assignment, variable);
			if (solution != UNKNOWN) { return solution; }
		}
		
		addActiveCls();
	}
	return solution;
}

solution_t Coprocessor::remove_blocked(assi_t assignment){
	
	litHeap.clear();
	for( var_t v = 1; v <= var_cnt; ++v ){
		if( lOcc[ index( lit(v,POS) ) ] > 0 ) litHeap.insert_item( lit(v,POS) );
		if( lOcc[ index( lit(v,NEG) ) ] > 0 ) litHeap.insert_item( lit(v,NEG) );
	}
	
	
	while( !litHeap.is_empty() ){
		
		lit_t l = litHeap.item_at( 0 );
		litHeap.remove_item(l);
		if(! assi_is_undef( assignment, var(l)) ) continue;

		
		if ( boolarray_get(elimWhite, var(l) ) ) continue;
		
		prefetchOcc( l );
		prefetchOcc( inv_lit(l) );

		VEC_TYPE( CL_REF )& clauses_lit1 = occ[index(l)];
		VEC_TYPE( CL_REF )& clauses_not_lit1 = occ[index(inv_lit(l))];
	  
		if( blockMaxOcc != -1 && ( (uint32_t)blockMaxOcc < VEC_SIZE( CL_REF, clauses_lit1 ) || (uint32_t)blockMaxOcc < VEC_SIZE( CL_REF, clauses_not_lit1 )) ) continue;
		
		
		for(uint32_t i = 0; i < VEC_SIZE( CL_REF, clauses_lit1 ); i++){
			CLAUSE& cl = gsa.get(clauses_lit1[i]);
			if( CL_IS_IGNORED( cl ) ) continue;
			
			if( blockMaxSize != -1 && (CL_SIZE( cl ) > (uint32_t)blockMaxSize )) continue;
			bool not_blocked = false;
			
			int count = 0;
			for( uint32_t j = 0; j < VEC_SIZE( CL_REF, clauses_not_lit1 ); j++){
				CLAUSE& ncl = gsa.get( clauses_not_lit1[j] );
				if( CL_IS_IGNORED( ncl ) ) continue;
				count ++;
				
				
				if( CL_CHECK_RESOLVE( cl, ncl, var(l) ) ){
					not_blocked = true; break;
				}
			}
		  
			
			if( count > 0 && !not_blocked ){
				removedBlocked[cRound]++;
				
				postEle pe;
				pe.kind='b';
				pe.b.lit1 = l;
				pe.b.clause = clauses_lit1[i];

				CL_IGNORE( cl );
				VEC_PUSH_BACK( postEle, postProcessStack, pe );

				
				remClOcc( clauses_lit1[i] );

				for( uint32_t j=0; j < CL_SIZE( cl ); j++ ){
					const lit_t cl_lit1 = CL_GET_LIT( cl, j);
					if( cl_lit1 == l ) continue;
					
					if( litHeap.contains( cl_lit1 ) ) litHeap.update_item( cl_lit1 );
					else litHeap.insert_item( cl_lit1 );
				}
				i --;
			}
		
		}
		
	}
	return UNKNOWN;
}

bool Coprocessor::searchEqui(assi_t assignment){
	
	SCCfinder* sf = new SCCfinder(var_cnt);
	sf->findSCCs(*clauses,assignment);
#ifndef SILENT
	cerr << "c found equivalences: " << sf->size() << endl;
#endif
	for( uint32_t i = 0 ; i < sf->size(); ++i ){
		addEquis( sf->getEqs(i));
	}

	if(dumpBiGraph){
		stringstream fn;
		fn << cmd.get( (const char*)"file" );
		fn << "-" << cRound << ".dot";
		sf->storeGraph(fn.str());
	}

	bool ret = (sf->size() > 0 );
	
	delete sf;
	return ret;
}

solution_t Coprocessor::applyEqui(assi_t assignment){

	
	for(var_t v = 1; v <=var_cnt;++v){
		if( assi_is_undef(assignment,v) ){
			if( replacedBy[ v ] != lit( v, POS ) ){
				boolarray_set( varRemoved, v, true );
#ifdef TRACK_CLAUSE
				cerr << "c remove variable " << v << endl;
#endif
				
				boolarray_set( data->eliminated, v, true );	
			}
		}
	}

	
	for( uint32_t ind = 0; ind < VEC_SIZE( CL_REF, (*clauses) ); ++ind ){
		CLAUSE& clause = gsa.get((*clauses)[ind]);
		if( CL_IS_IGNORED(clause) ) continue;	

		bool changed = false;
		
		
		uint32_t ci = 0;
		for(  ; ci < CL_SIZE( clause ) ; ++ci ){
			const lit_t lit1 = CL_GET_LIT( clause, ci );
			if( replacedBy[ var( lit1 ) ] != lit( var( lit1 ), POS ) ) break;
		}
		
		if( ci != CL_SIZE( clause ) ) remClOcc ((*clauses)[ind]);
		
		
		for( uint32_t i = 0 ; i < CL_SIZE( clause ) ; ++i ){
			const lit_t lit1 = CL_GET_LIT( clause, i );
			if( replacedBy[ var( lit1 ) ] != lit( var( lit1 ), POS )){
				changed = true;
				lit_t lit12 = NO_LIT;
				if( pol( lit1 ) == POS ) lit12 = replacedBy[ var( lit1 ) ];
				else  lit12 = inv_lit( replacedBy[ var( lit1 ) ] );
				CL_SET_LIT( clause, i , lit12 );
				lOcc[ index(lit1) ]--;
				vOcc[ var(lit1)] --;
			}
		}
		
		
		if( changed ){
			changed = false;

			
			for( uint32_t i = 0 ; i < CL_SIZE( clause ) && !changed ; ++i ){
				const lit_t l1 = CL_GET_LIT(clause, i );
				for( uint32_t j = i+1 ; j < CL_SIZE( clause ); ++j ){
					const lit_t l2 = CL_GET_LIT(clause, j );
					if( l2 == inv_lit( l1 ) ){ changed = true; break; }	
					if( l2 == l1 ){ CL_DELETE_INDEX( clause, j ); j--; }	
				}
			}

			if( changed ){ 
				removedEquiCls[cRound]++;
				CL_IGNORE( clause );
			} else { 
				if(  UNSAT == addSingleClause(assignment, (*clauses)[ind]) ){
					cerr << "unsat in equi applying" << endl;
					return UNSAT;
				}
			}
		}
		
	}

	
	postEle pe;
	pe.kind='e';
	VEC_PUSH_BACK( postEle, postProcessStack, pe );
	
	return UNKNOWN;
}

void Coprocessor::addEquis( const VEC_TYPE(lit_t)& eqs ){

	if( VEC_SIZE(lit_t, eqs) == 0 ) return;
	
	
	lit_t min = eqs[0];
	var_t minV = var( min );
	lit_t roots[ VEC_SIZE(lit_t, eqs) ];

	
	for( uint32_t j = 0 ; j < VEC_SIZE( lit_t, eqs ); ++j ){
	
		lit_t l = eqs[j];
		while( var(l) != var(replacedBy[ var(l) ]) ) l = pol(l) == POS ? replacedBy[ var(l) ] : inv_lit(replacedBy[ var(l) ] );
		roots[j] = l;
		
		if( var(l) < minV ){
			minV = var(l);
			min = l;
		}
	}
	
	
	for( uint32_t j = 0 ; j < VEC_SIZE( lit_t, eqs ); ++j ){
		const lit_t lit1 = roots[j];
		replacedBy[ var(lit1) ] = ( pol(lit1) == POS) ? min : inv_lit( min );
	}
}

solution_t Coprocessor::subsSimp (assi_t assignment)
{
	
	if( simplifying && !simpSubsSimp ) return UNKNOWN;

#ifdef TRACK_CLAUSE

#endif
	
	if( (int32_t)VEC_SIZE (CL_REF, subsQ) > maxQsize ) {
		VEC_CLEAR(CL_REF, subsQ);
		return UNKNOWN;
	}

	
	while (VEC_SIZE (CL_REF, subsQ) > 0 && stopTime > get_microseconds()) {

		
		CL_REF clause = subsQ[VEC_SIZE (CL_REF, subsQ) - 1];
		CLAUSE & cl = gsa.get (clause);
		VEC_POP_BACK (CL_REF, subsQ);

		
		if (CL_IS_IGNORED (cl)) continue;

		
		var_t var1 = var (CL_GET_LIT (cl, 0));
		for (uint32_t j = 1; j < CL_SIZE (cl); ++j) {
			if (vOcc[var (CL_GET_LIT (cl, j))] < vOcc[var1]) {
				var1 = var (CL_GET_LIT (cl, j));
			}
		}

		
		for (uint32_t pn = 0; pn < 2; ++pn) {

			prefetchOcc( (pn == 0) ? lit(var1,POS) : lit(var1,NEG) );

			VEC_TYPE (CL_REF) tmpO = (pn == 0) ? occ[ index( lit(var1,POS) )] : occ[ index( lit(var1,NEG) )];
			for (uint32_t i = 0; i < VEC_SIZE (CL_REF, tmpO); ++i) {

				CL_REF clause_oc = tmpO[i];
				CLAUSE & cl_oc = gsa.get (clause_oc);
				
				
				lit_t lit1_oc = NO_LIT;
				if (CL_IS_IGNORED (cl_oc) || clause_oc == clause || !CL_SUBSUMES_OR_SIMPLIFIES (cl, cl_oc, &lit1_oc)) {
					continue;
				}

				
				
				
				
				
				
				if (lit1_oc == NO_LIT) {
					
					
					if( !CL_IS_LEARNT(cl_oc) ) CL_SET_NO_LEARNT( cl );
					CL_IGNORE_AND_DELETE (cl_oc);
					remClOcc (clause_oc);
					removedSubsCls[cRound]++;
					
					i--;
					
				} else {
					if( debug > 2 ){
						cerr << "c simpl.";
						for( uint32_t abc = 0; abc < CL_SIZE(cl_oc); abc ++ ) cerr << " " << nr( CL_GET_LIT(cl_oc,abc) );
						cerr << " with ";
						for( uint32_t abc = 0; abc < CL_SIZE(cl); abc ++ ) cerr << " " << nr( CL_GET_LIT(cl,abc) );
						cerr << endl;
					}
					
					CL_DELETE_LIT (cl_oc, lit1_oc);
					vOcc[var(lit1_oc)]--;
					lOcc[index(lit1_oc)]--;

					
					var_t variable = var(lit1_oc);
					for (uint32_t it_clause = 0; it_clause < VEC_SIZE (CL_REF, occ[ index( lit(variable,POS) )]); ++it_clause) {
						if (clause_oc == occ[ index( lit(variable,POS) )][it_clause]) {
							VEC_ERASE (CL_REF, occ[ index( lit(variable,POS) )], it_clause);
							break;
						}
					}
					variable = var (lit1_oc);
					if (pol (lit1_oc) == POS) {
						
						for (uint32_t it_clause = 0; it_clause < VEC_SIZE (CL_REF, occ[ index( lit(variable,POS) )]); ++it_clause) {
							if (clause_oc == occ[ index( lit(variable,POS) )][it_clause]) {
								VEC_ERASE (CL_REF, occ[ index( lit(variable,POS) )], it_clause);
								break;
							}
						}
					} else {
						
						for (uint32_t it_clause = 0; it_clause < VEC_SIZE (CL_REF, occ[ index( lit(variable,NEG) )]); ++it_clause) {
							if (clause_oc == occ[ index( lit(variable,NEG) )][it_clause]) {
								VEC_ERASE (CL_REF, occ[ index( lit(variable,NEG) )], it_clause);
								break;
							}
						}
					}

					
					if (CL_SIZE (cl_oc) == 0) {
						cerr << "c unsat during subsumption (size 0)" << endl;
						return UNSAT;
					}

					if (CL_SIZE (cl_oc) == 1){
						if( !enqueue(assignment,  CL_GET_LIT (cl_oc, 0))){
							cerr << "c unsat dring subsumption (invalid unit)" << endl;
							return UNSAT;
						}
					}

					
					
					vOcc[var (lit1_oc)]--;
					lOcc[ index(lit1_oc) ]--;

					
					
					
					
					VEC_PUSH_BACK (CL_REF, subsQ, clause_oc);
				}
			}
		}
	}
	return solution;
}

solution_t Coprocessor::extendedResolution(searchData& search){

	
	if( search.var_cnt > erMaxV && ( VEC_SIZE(CL_REF, (*clauses)) / search.var_cnt < 10 ) ) return UNKNOWN;

	struct litPair {
		lit_t lits[2];
		uint32_t occ;
		litPair() : occ(0){}
		
		litPair& operator=(const litPair& other){
			occ = other.occ;
			lits[0] = other.lits[0];
			lits[1] = other.lits[1];
			return *this;
		}
		
		litPair(const litPair& other){
			occ = other.occ;
			lits[0] = other.lits[0];
			lits[1] = other.lits[1];
		}
	};

	uint32_t oldVars = var_cnt;
	
	
	litHeap.clear();
	
	if( UNSAT == propagate( search.assi ) ) return UNSAT;
	
	uint32_t oldVarCnt = var_cnt;
	
	
	
	
	if( erPairs * erIters + var_cnt < erMinV ){
		erMinO = 3;
		do {
			
			uint32_t tmp = erMinV - var_cnt;
			if( tmp > var_cnt ) erIters = erIters*2 + 1;
			
			erPairs = tmp / erIters + 1;
		
		} while ( erPairs > var_cnt );
	}
	
	
	extend(erPairs * erIters + var_cnt);

	if( debug > 3 ) cerr << "c size of heap: " << litHeap.size() << endl;

	uint32_t cOcc[ max_index(erPairs + var_cnt + 1) ];
	memset(cOcc,0, sizeof(uint32_t) *  max_index(erPairs + var_cnt+1) );

	if( litHeap.is_empty() ){
		
		for( var_t v = 1; v <= var_cnt; ++v ){
			if( lOcc[ index( lit(v,POS) ) ] >= erMinO ) litHeap.insert_item( lit(v,POS) );
			if( lOcc[ index( lit(v,NEG) ) ] >= erMinO ) litHeap.insert_item( lit(v,NEG) );
		}
	}

	
	for( uint32_t round = 0; round < erIters && stopTime > get_microseconds(); round ++ ){
	
#ifdef PARALLEL
 
 pthread_testcancel();
#endif
	
		if( litHeap.is_empty() ){
			
			for( var_t v = 1; v <= var_cnt; ++v ){
				if( lOcc[ index( lit(v,POS) ) ] >= erMinO ) litHeap.insert_item( lit(v,POS) );
				if( lOcc[ index( lit(v,NEG) ) ] >= erMinO ) litHeap.insert_item( lit(v,NEG) );
			}
		}
	
		if( debug > 1 ) cerr << "c er round " << round << endl;

		litPair pairs[erPairs];
		uint32_t pairsSize = 0;

		
		for( uint32_t j = 0 ; j < erTries * erPairs && !litHeap.is_empty(); ++j ){
			
			const lit_t l = litHeap.item_at( litHeap.size() - 1 );

			

			if(! assi_is_undef( search.assi, var(l)) ){
				--j;
				if( litHeap.contains(l) ) litHeap.remove_item(l);	
				continue;
			}

			
			if ( boolarray_get(elimWhite, var(l) ) ){
				--j;
				if( litHeap.contains(l) ) litHeap.remove_item(l);	
				continue;
			}

			prefetchOcc( l );
			const VEC_TYPE( CL_REF )& clauses_l = occ[index(l)];

			
			for(uint32_t i = 0; i < VEC_SIZE( CL_REF, clauses_l ); i++){
				CLAUSE& cl = gsa.get(clauses_l[i]);
				
				if( CL_IS_IGNORED( cl ) || (erMinSize != 0 && erMinSize > CL_SIZE(cl) )) continue;

				
				for( uint32_t k = 0; k < CL_SIZE( cl ); k++){
					const lit_t thisL = CL_GET_LIT(cl,k);
					if( thisL == l || !litHeap.contains(thisL) ) continue;
					cOcc[ index( thisL ) ] ++;
					if( thisL == inv_lit(l) ) cerr << "c found tautology in a clause in the formula for literal " << nr(l) << endl;
				}
			}

			
			lit_t maxL = NO_LIT; uint32_t maxO = 0;	
			for( lit_t tmpL = 0; tmpL < max_index( erPairs + var_cnt ); tmpL ++ ){
				if( cOcc[ index( tmpL ) ] > maxO ){
					maxO = cOcc[ index( tmpL ) ];
					maxL = tmpL;
				}
				
			}

			if( maxO < erMinO ){
				
				if( litHeap.contains(l) ) litHeap.remove_item(l);	
				
				--j;
				
				memset(cOcc,0, sizeof(uint32_t) *  max_index(erPairs + var_cnt+1) );
				continue;
			}

			litPair currentPair;	
			currentPair.lits[0] = l;
			currentPair.lits[1] = maxL;
			currentPair.occ = maxO;
			if( debug > 1 ) cerr << "c create pair with occ=" << maxO << endl;

			
			if( pairsSize < erPairs ) pairsSize++;
			for(uint32_t p = 0; p < pairsSize; p++ ){
				if( pairs[p].occ < currentPair.occ ){
					litPair tmpPair = pairs[p];
					pairs[p] = currentPair;
					currentPair = tmpPair;
				}
			}
		
			
			if( litHeap.contains(l) ) litHeap.remove_item( l );
			if( litHeap.contains(maxL) ) litHeap.remove_item( maxL );
			
			
			memset(cOcc,0, sizeof(uint32_t) *  max_index(erPairs + var_cnt+1) );
		}
		
		
		for(uint32_t p = 0; p < pairsSize; p++){
			uint32_t maxO = pairs[p].occ;
			const lit_t l = pairs[p].lits[0];
			const lit_t maxL = pairs[p].lits[1];
		
			
			
			var_t newVar = oldVarCnt + p + 1 + round*erPairs;
		
			if( debug > 1 ) cerr << "c replace " << newVar << " = " << nr(l) << " or " << nr(maxL) << "with occurences: " << maxO << endl;
			VEC_TYPE( CL_REF )& clauses_l = occ[index(l)];
			
			
			for(uint32_t i = 0; i < VEC_SIZE( CL_REF, clauses_l ); i++){
				CLAUSE& cl = gsa.get(clauses_l[i]);
				if( CL_IS_IGNORED( cl ) ) continue;
				
				uint32_t c = 0;
				uint32_t pos[2]; 
				
				for( uint32_t k = 0; k < CL_SIZE( cl ); k++){
					const lit_t thisL = CL_GET_LIT(cl,k);
					if( maxL == thisL || l == thisL ){ 
						pos[c] = k;					
						if( ++c == 2 ) break;
					}
				}
				
				if( c == 2 ){
					const CL_REF thisC = clauses_l[i];
					remClOcc (thisC);
					if( debug > 2 )	{
						cerr << "c change";
						for( uint32_t abc=0; abc < CL_SIZE(cl); abc++) cerr << " " << nr( CL_GET_LIT(cl, abc) );
						cerr << " with ";
					}
						CL_SET_LIT( cl, pos[0], lit(newVar,POS) );
						CL_DELETE_INDEX( cl, pos[1] );
					if( debug > 2 )	{
						for( uint32_t abc=0; abc < CL_SIZE(cl); abc++) cerr << " " << nr( CL_GET_LIT(cl, abc) );
						cerr << endl;
						if(  CL_SIZE(cl) == 1 ) cerr << "c added unit during extended resolution" << endl;
					}
					
					addSingleClause(search.assi, thisC);
					--i;	
				}
			}
			
			
			lit_t clauseLits[3];
			clauseLits[0] = lit( newVar, NEG );
			clauseLits[1] = l;
			clauseLits[2] = maxL;
			CL_REF clauseRs[3];
			clauseRs[0] = gsa.create( CL_CREATE_NO_LEARNT( clauseLits , 3 ) );
			clauseLits[0] = lit( newVar, POS );
			clauseLits[1] = inv_lit(l);
			clauseRs[1] = gsa.create( CL_CREATE_NO_LEARNT( clauseLits , 2 ) );
			clauseLits[1] = inv_lit(maxL);
			clauseRs[2] = gsa.create( CL_CREATE_NO_LEARNT( clauseLits , 2 ) );			
			for( int i = 0; i < 3; i++ ){
				addSingleClause( search.assi, clauseRs[i] );
				VEC_PUSH_BACK(CL_REF, (*clauses), clauseRs[i] );
			}
			
			if( debug > 2 ) {
				cerr << "c newly added clauses:" << endl << "c 1:";
				for( uint32_t abc = 0; abc < 3; abc++) cerr << " " << nr( CL_GET_LIT( gsa.get(clauseRs[0]),abc) );
				cerr << endl << "c 2:";
				for( uint32_t abc = 0; abc < 2; abc++) cerr << " " << nr( CL_GET_LIT( gsa.get(clauseRs[1]),abc) );
				cerr << endl << "c 3:";
				for( uint32_t abc = 0; abc < 2; abc++) cerr << " " << nr( CL_GET_LIT( gsa.get(clauseRs[2]),abc) );
				cerr << endl;
			}
		
		
			
			
			if( litHeap.contains(l) ) litHeap.update_item( l ); else litHeap.insert_item( l );
			if( litHeap.contains(maxL) ) litHeap.update_item( maxL ); else litHeap.insert_item( maxL );
			litHeap.insert_item( lit(newVar,POS) );
			litHeap.insert_item( lit(newVar,NEG) );
		
		}

		
		if( debug > 2 ) {
			cerr << "c round " << round << ": replacement pairs:" << endl;
			for(uint32_t p = 0; p < pairsSize; p++){
				cerr << "c " << pairs[p].occ << " times " << nr( pairs[p].lits[0] ) << " or " << nr( pairs[p].lits[1] ) << endl;
			}
		}
			
		
		if( UNSAT == propagate( search.assi ) ){
			cerr << "c found UNSAT during propagating units in extended resolution" << endl;
			return UNSAT;
		}
		
	}	



	return UNKNOWN;
}

solution_t Coprocessor::ternaryResolution(assi_t assignment){
	int countClauses = 0;
	for( var_t v = 1; v <= var_cnt; v++ ){
	
		if( !assi_is_undef( assignment, v) ) continue;	
	
		
		prefetchOcc(lit(v,NEG));
		prefetchOcc(lit(v,NEG));
		VEC_TYPE (CL_REF)& pos = occ[ index( lit(v,POS) ) ];
		VEC_TYPE (CL_REF)& neg = occ[ index( lit(v,NEG) ) ];
		
		
		if( VEC_SIZE( CL_REF, pos )==0 || VEC_SIZE( CL_REF, neg ) == 0 ) continue;

		
		const uint32_t psize = VEC_SIZE( CL_REF, pos );
		for( uint32_t i = 0; i < psize; ++i ){
			lit_t lits[2];
			
			;
			if( CL_IS_IGNORED(gsa.get( pos[i] )) || CL_SIZE( gsa.get( pos[i] ) ) != 3 ) continue;	
			
			
			uint32_t c = 0;	
			for( uint32_t pi = 0; pi < 3; pi ++ ){
				const lit_t tmpL = CL_GET_LIT(  gsa.get( pos[i] ) , pi );
				if( tmpL == lit(v,POS) ) continue;
				lits[ c++ ] = tmpL;
			}
			
			if( c > 2 ){
				cerr << "c for literal " << nr( lit(v,POS) ) << " found a clause in occ that does not contain this lit" << endl;
			}
			assert( c <= 2 );
			
			
			uint32_t sameLits = 0;	
			const uint32_t nsize = VEC_SIZE( CL_REF, neg );
			for( uint32_t j = 0; j < nsize; ++j ){
				const CLAUSE& negCl = gsa.get( neg[j] );
				if( CL_IS_IGNORED(negCl) || CL_SIZE(negCl) != 3 ) continue;	
				
				sameLits = 0;
				
				
				uint32_t ni = 0;
				for( ; ni < 3; ni ++ ){
					const lit_t tmpL = CL_GET_LIT( negCl, ni );
					if( tmpL == lit(v,NEG) ) continue;
					if( tmpL == lits[0] || tmpL == lits[1] ) sameLits ++;
					if( tmpL == inv_lit(lits[0]) || tmpL == inv_lit( lits[1]) ) break; 
				}
				
				
				if( ni == 3 && sameLits > 0 ){
					CLAUSE *cp = CL_RESOLVE_VAR ( gsa.get( pos[i] ) , negCl, v);
					if (cp != 0) {
						CL_REF resolve_clause = gsa.create (*cp);
					
						
						VEC_PUSH_BACK (CL_REF, (*clauses), resolve_clause);
						if( addSingleClause (assignment, resolve_clause) == UNSAT) return UNSAT;
						
						countClauses++;
					}
				}
				
			}
			
		}
	
	}	
	return UNKNOWN;
}


solution_t Coprocessor::failedLitProbing(assi_t assignment){

	
	uint32_t min = (double)resQ.size() * (double)probVarsP / 100.0;
	min = ( probVars != -1 && probVars < (int32_t)min ) ? probVars : min;
#ifndef SILENT
	cerr << "c failed literal probing for " << min << " variables" << endl;
#endif

	assi_t asumeAssi = assi_create(var_cnt);
	assi_t iAsumeAssi = assi_create(var_cnt);

	for( uint32_t i = 0 ; i < min; ++i){
		const var_t v = resQ.item_at (i);
		if(! assi_is_undef(assignment,v)) continue;

		lit_t assume = lit(v,NEG);
		
		assi_clear(asumeAssi, var_cnt);

		VEC_CLEAR( lit_t, tmpLits);
		if( !propagation.createAssignment(assignment, asumeAssi, assume, tmpLits) ){
			assume = inv_lit(assume);
			if( !enqueue(assignment, assume ) ) return UNSAT;
			foundProbUnits[cRound] ++;
			continue;
		}
		assi_undef_variable(asumeAssi, var(assume) );
		if( createBinaries ) createBinaryClauses( assignment, assume, tmpLits );

		assume = inv_lit(assume);
		
		assi_clear(iAsumeAssi, var_cnt);
		
		VEC_CLEAR( lit_t, tmpLits);
		if(!propagation.createAssignment(assignment, iAsumeAssi, assume, tmpLits) ){
			assume = inv_lit(assume);
			if( !enqueue(assignment, assume ) ) return UNSAT;
			foundProbUnits[cRound] ++;
			continue;
		}
		assi_undef_variable(iAsumeAssi, var(assume) );
		if( createBinaries ) createBinaryClauses( assignment, assume, tmpLits );

		VEC_CLEAR( lit_t, tmpLits);
		uint32_t oldUnits = VEC_SIZE( lit_t, unitQ);
		
		int32_t r = assi_get_equal_lits(iAsumeAssi, asumeAssi, assignment, unitQ, tmpLits, var_cnt);
		if( r == -1 ){
			cerr << "c Failed lit probing produced conflict" << endl;
			return UNSAT; 
		}
		else foundProbUnits[cRound] += r;
		for( uint32_t j = oldUnits; j < VEC_SIZE( lit_t, unitQ); ++j){
			if( assi_is_unsat(assignment, unitQ[j]) ) return UNSAT;
			assi_set_polarity( assignment, var(unitQ[j]), pol(unitQ[j]));
		}
		
		
		if( VEC_SIZE(lit_t, tmpLits) > 0 ) {
			VEC_PUSH_BACK( lit_t, tmpLits, assume );
			addEquis(tmpLits);
		}
	}
	return UNKNOWN;
}

solution_t Coprocessor::clauseProbing(assi_t assignment){
#ifndef SILENT
	cerr << "c clause probing" << endl;
#endif
	
	uint32_t min = ( probClVars != -1 && ((uint32_t)probClVars < litHeap.size())  ) ? probClVars : litHeap.size();
	for( uint32_t i = 0 ; i < min; ++i){
		const lit_t l = litHeap.item_at (i);
		prefetchOcc(l);
		VEC_TYPE (CL_REF)& lo = occ[index(l)];
		
		for( uint32_t j = 0 ; j< VEC_SIZE( CL_REF, lo ); ++j ){
			CLAUSE& cl = gsa.get(lo[j]);
			if( CL_IS_IGNORED(cl) ) continue;
			if( probClMaxS != -1 && (CL_SIZE(cl) > (uint32_t)probClMaxS)) continue;
			uint32_t size = 0;
			lit_t lits [CL_SIZE(cl)];
			uint32_t k = 0;
			for( ; k < CL_SIZE(cl); ++k ){
				const lit_t l2 = CL_GET_LIT(cl,k);
				
				if( assi_is_sat(assignment, l2 ) ) break;
				if( assi_is_unsat(assignment, l2 )) continue;
				lits[size++] = l2;
				
				if( lAssi[ index(l2)] == 0 ){	
					lAssi[ index(l2)] = assi_create(var_cnt);
					propagation.createAssignment(assignment, lAssi[ index(l2)], l2, tmpLits);
					VEC_CLEAR( lit_t, tmpLits);
					if( !propagation.createAssignment(assignment, lAssi[ index(l2)], l2, tmpLits) ){
						if( !enqueue(assignment, inv_lit(l2) ) ) return UNSAT;
						foundProbUnits[cRound] ++;
						size --;
						continue;
					}
					assi_undef_variable(lAssi[ index(l2)], var(l2) );
					if( createBinaries ) createBinaryClauses( assignment, l2, tmpLits ); 
				}
			}
			if( k < CL_SIZE(cl)) continue;	
			
			if( size < 2){
				if( size == 0 ) return UNSAT;
				if( size == 1 ) {
					if( !enqueue(assignment, lits[0])) return UNSAT;
					continue;
				}
			}
			
			for( var_t v =1; v <= var_cnt; ++v ){
				if( ! assi_is_undef(assignment,v)) continue;
				uint32_t k = 1;
				const pol_t p = assi_get_polarity( lAssi[ index(lits[0])], v );
				if( p == UNDEF ) continue;
				for( ; k < size; ++k ){
					if( !assi_variable_equal(lAssi[ index(lits[k])], v, p) ) break;
				}
				if( k == size){	
					if( !enqueue(assignment, lit(v,p) ) ) return UNSAT;
				}
			}
		}
	}
	return UNKNOWN;
}

solution_t Coprocessor::asymmBranch(assi_t assignment){
	
	uint32_t min = ( (double)(maxClLen) * aPercent / 100.0);
	min = (aMinSize == -1 || min >= (uint32_t)aMinSize) ? min : aMinSize;
#ifndef SILENT
	cerr << "c asymmetric branching for clause sizes >= " << min << endl;
#endif
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		CLAUSE & cl = gsa.get ((*clauses)[i]);
		if (CL_IS_IGNORED(cl) ) continue;
		if (CL_SIZE (cl) < min) continue;
		assi_clear( tmpAssi, var_cnt);
		VEC_CLEAR( lit_t, tmpLits);
		VEC_CLEAR( lit_t, tmpLits2);
		uint32_t j = 0 ;
		
		for(  ;j < CL_SIZE(cl); ++j ){
			const lit_t l = CL_GET_LIT(cl,j);
			if( assi_is_sat(assignment, l)) break;	
			if( !assi_is_undef(assignment, var(l))) continue;
			
			VEC_PUSH_BACK( lit_t, tmpLits, inv_lit(l) ); 
		}
		if( j != CL_SIZE(cl) || j < min) continue;	

		
		
		if( !propagation.conflictFree(assignment,tmpAssi,tmpLits,tmpLits2) && VEC_SIZE(lit_t,tmpLits2) != CL_SIZE(cl) ){
			
			CL_REF newCls;

			
			for( uint32_t j = 0 ; j < VEC_SIZE(CL_REF,tmpLits2); ++j ) tmpLits2[j] = inv_lit(tmpLits2[j]); 
			if( CL_IS_LEARNT(cl) ){
				newCls = gsa.create( CL_CREATE( &(tmpLits2[0]), VEC_SIZE(lit_t,tmpLits2) ) );
			} else {
				newCls = gsa.create( CL_CREATE_NO_LEARNT( &(tmpLits2[0]), VEC_SIZE(lit_t,tmpLits2) ) );
			}
			
			if( UNSAT == addSingleClause( assignment, newCls) ) return UNSAT;

			
			VEC_PUSH_BACK( CL_REF, (*clauses), newCls);
			asymmNewCls[cRound]++;
			
			
			CL_IGNORE( gsa.get ((*clauses)[i]) );
			remClOcc( (*clauses)[i] );
			VEC_ERASE_NO_ORDER( CL_REF, (*clauses), i );
			--i;
		}
	}
	return UNKNOWN;
}

solution_t Coprocessor::hyberBinProb(assi_t assignment){
	
	resQ.clear();
	
	for (var_t v = 1; v < var_cnt + 1; ++v){
		if( vOcc[v] > 0 ) resQ.insert_item (v);
		if( lOcc[index( lit(v, POS) )] > 0 ) litHeap.insert_item (index( lit(v, POS) ));
		if( lOcc[index( lit(v, NEG) )] > 0 ) litHeap.insert_item (index( lit(v, NEG) ));
	}
	
	propagation.clear();
	
	solution_t s =  propagation.init(clauses,assignment,unitQ);
	if( s == UNSAT ){
		cerr << "c ROP init failed" << endl;
	}

	
	if( cRound < probL || simpProbL ){
		if( UNSAT == failedLitProbing(assignment) ) return UNSAT;
	}
	if( cRound < probC || simpProbC ){
		if( UNSAT == clauseProbing(assignment)) return UNSAT;
	}
	if( cRound < asymm || simpPAsymm ){
		if( UNSAT == asymmBranch(assignment)) return UNSAT;
	}

	return UNKNOWN;
}

void Coprocessor::detectPure (assi_t assignment)
{
	
	for (uint32_t i = 1; i <=var_cnt; ++i) {
		if (!assi_is_undef( assignment, i) || vOcc[i] == 0 ) continue;
		
		if ( boolarray_get(elimWhite, i) ) continue; 
		
#ifdef TRACK_CLAUSE
		if( i == var( trackme) ){
			cerr << "c run pure on tracked variable " << endl;
		}
#endif
		
		
		lit_t l = lit(i,POS);
		bool foundPos = true;
		prefetchOcc(l);
		for (uint32_t j = 0; j < VEC_SIZE (CL_REF, occ[ index( l )]); ++j) {
			if (CL_IS_IGNORED (gsa.get (occ[ index( l )][j]))) continue;
			else {
				foundPos = false;
				break;
			}
		}
		
		bool foundNeg = true;
		l = inv_lit(l);
		prefetchOcc(l);
		for (uint32_t j = 0; j < VEC_SIZE(CL_REF, occ[ index( l )]); ++j) {
			if (CL_IS_IGNORED (gsa.get (occ[ index( l )][j]))) continue;
			else {
				foundNeg = false;
				break;
			}
		}

		
		if( (foundPos && foundNeg) || (!foundPos && !foundNeg) ) continue;

		pureLits[cRound]++;
		if( foundPos){
			this->enqueue(assignment, l );
		} else {
			this->enqueue(assignment, inv_lit(l) );
		}
	}
}

solution_t Coprocessor::hte(assi_t assignment, uint32_t first){

	int i, j, k, nrval, failed_flag, unit_flag = 0;
	int *bin_occ, **bin_lut, *bin_table, bin_total = 0;
	int *bin_que, *head, *tail, hla_size;

	

	bin_occ = (int * ) malloc( sizeof(int ) * (2*var_cnt + 1) );
	bin_que = (int * ) malloc( sizeof(int ) * (2*var_cnt + 1) );
	bin_lut = (int **) malloc( sizeof(int*) * (2*var_cnt + 1) );

	for( i = 0; i <= (int)(2*var_cnt); i++ ) bin_occ[ i ] = 0; 
	bin_occ += var_cnt;
	bin_lut += var_cnt;

	
	for( i = 0; i < (int) VEC_SIZE( CL_REF, (*clauses) ); i++ ){
		CLAUSE& cl = gsa.get( (*clauses)[i] );
		if( CL_IS_IGNORED(cl) ) continue;
	
	   if( CL_SIZE(cl) == 2 ){
		bin_total += 2;
		bin_occ[ nr(CL_GET_LIT(cl,0)) ]++;
		bin_occ[ nr(CL_GET_LIT(cl,1)) ]++;
		
		if( CL_IS_LEARNT(cl) ) CL_SET_NO_LEARNT( cl );
	   }
	}
	
	
	
	bin_table = (int*) malloc( sizeof(int) * bin_total );

	bin_total = 0;
	for( i = -(int)var_cnt; i <= (int)var_cnt; i++ )
	{
	    bin_lut[ i ] = bin_table + bin_total;
	    bin_total += bin_occ[ i ];
	    bin_occ[ i ] = 0;
	}

	for( i = 0; i < (int) VEC_SIZE( CL_REF, (*clauses) ); i++ ){
		CLAUSE& cl = gsa.get( (*clauses)[i] );
		if( CL_IS_IGNORED(cl) ) continue;
	   if( CL_SIZE(cl) == 2 && !CL_IS_LEARNT(cl) ){
	   		
			bin_lut[ nr(CL_GET_LIT(cl,0)) ][ bin_occ[ nr(CL_GET_LIT(cl,0)) ]++ ] = -nr(CL_GET_LIT(cl,1));
			bin_lut[ nr(CL_GET_LIT(cl,1)) ][ bin_occ[ nr(CL_GET_LIT(cl,1)) ]++ ] = -nr(CL_GET_LIT(cl,0));
	   }
	}

	int MyIntArray[ (2*var_cnt + 1) ];
	int* MarkArray = MyIntArray + var_cnt;
	for( i = -(int)var_cnt; i <= (int)var_cnt; i++ ) MarkArray[ i ] = 0;

	for( i = -(int)var_cnt; i <= (int)var_cnt; i++ )
	{
		prefetchOcc(lit(i));
	    if( i == 0 ) continue;

	    failed_flag = 0;
	    hla_size    = 1;

	    MarkArray[ i ] = i;

	    for( j = 0; j < bin_occ[ i ]; j++ )
	    {
		int imp = bin_lut[ i ][ j ];
		if( MarkArray[ imp ] == i ) continue;

	        head = bin_que;
	        tail = bin_que;

	        *(head++) = imp;	
	        MarkArray[ imp ] = i;
		hla_size++;

	        while( tail < head )
	        {
		    int lit = *(tail++);

		    for( k = 0; k < bin_occ[ lit ]; k++ )
		    {
			int _lit = bin_lut[ lit ][ k ];



			if( MarkArray[ _lit ] != i )
			{
			    if( MarkArray[ -_lit ] == i ) 
			    {
				failed_flag = 1;
				goto hla_failed_literal;
			    }

			    MarkArray[ _lit ] = i;
			    *(head++) = _lit;
			    hla_size++;
			}
		    }
		}
		MarkArray[ imp ] = 0;
	    }

	    
		
	    for( k = 0; k < (int) VEC_SIZE( CL_REF, occ[ index(lit(i)) ] ); k++ ){  
	     
			CL_REF clsidx = occ[ index(lit(i)) ][k];
			CLAUSE& cl = gsa.get( clsidx );
			if( CL_IS_IGNORED(cl) ) continue;
			if( CL_SIZE(cl) == 2){
			
				bool remClause = false;
				for( j = 0; j < (int) CL_SIZE(cl); j++ ){
					if( MarkArray[ - nr( CL_GET_LIT(cl,j) ) ] == i ){
						int s;
						for( s = 0; s < bin_occ[ nr( CL_GET_LIT(cl,0)) ]; s++ ) 
						if( bin_lut[ nr( CL_GET_LIT(cl,0)) ][ s ] == - nr( CL_GET_LIT(cl,1) ) )
							bin_lut[ nr( CL_GET_LIT(cl,0)) ][ s ] = 0;

						for( s = 0; s < bin_occ[ nr( CL_GET_LIT(cl,1)) ]; s++ ) 
						if( bin_lut[ nr( CL_GET_LIT(cl,1)) ][ s ] == - nr( CL_GET_LIT(cl,0) ) )
							bin_lut[ nr( CL_GET_LIT(cl,1)) ][ s ] = 0;

						
						CL_IGNORE(cl);
						remClause = true;
						
						 
						break;
					}
				}
			
				
				if( remClause ){
					remClOcc (clsidx);
					k--;
				}
			} 
	    }

	    for( j = 0; j < bin_occ[ i ]; j++ )
		MarkArray[ bin_lut[i][j] ] = i;

	    if( unit_flag == 0 )
	      for( k = 0; k < (int) VEC_SIZE( CL_REF, occ[ index(lit(i)) ] ); k++ )
	      {
			CL_REF clsidx = occ[ index(lit(i)) ][k];
			CLAUSE& cl = gsa.get( clsidx );
			if( CL_IS_IGNORED(cl) ) continue;
			bool ignClause = false;
			if( CL_SIZE(cl) > 2 ){
				for( j = 0; j < (int) CL_SIZE(cl); j++ ){
					if( MarkArray[ - nr( CL_GET_LIT(cl,j) ) ] == i ){
							
							ignClause = true;
							CL_IGNORE(cl);
							hteRemoved[cRound] ++;
							
							break;
					}
					else if( nr( CL_GET_LIT(cl,j) ) != i && MarkArray[ nr( CL_GET_LIT(cl,j) ) ] == i ) {
						 

						lit_t other = CL_GET_LIT( cl, j );
												
						 
						  uint32_t l = 0;
						 for(; l < VEC_SIZE(CL_REF, occ[ index(other) ]); l++ ){
						 	if( occ[ index(other) ][l] == clsidx ) {
						 		VEC_ERASE_NO_ORDER( CL_REF, occ[ index(other) ], l );
						 		break;
						 	}
						 }
						 
						 CL_DELETE_INDEX( cl, j );
						 
						 j--;
						 if( CL_SIZE(cl) == 1 ){
						 	if( UNSAT == addSingleClause ( assignment, clsidx) ){
						 		cerr << "c found UNSAT during HTE" << endl;
						 		return UNSAT;
						 	}
						 } else if( CL_SIZE(cl) == 0 ){
						 		cerr << "c found UNSAT during HTE" << endl;
						 		return UNSAT;
						 }
					}
				}
			}
			
			
			if(ignClause){
				remClOcc (clsidx);
				k--;
			} 
	      }

	    hla_failed_literal:;
		
	    if( failed_flag ){
				for( k = 0; k < (int)  VEC_SIZE( CL_REF, occ[ index(lit(i)) ] ); k++ ){
					CL_REF clsidx = occ[ index(lit(i)) ][k];
					CLAUSE& cl = gsa.get( clsidx );
					if( CL_IS_IGNORED(cl) ) continue;
						
				
						if( CL_SIZE(cl) == 2 ){
					int s;
						for( s = 0; s < bin_occ[ nr(CL_GET_LIT( cl, 0)) ]; s++ ) if( bin_lut[ nr(CL_GET_LIT( cl, 0)) ][ s ] == - nr(CL_GET_LIT( cl, 1)) ) bin_lut[ nr(CL_GET_LIT( cl, 0)) ][ s ] = 0;
						for( s = 0; s < bin_occ[ nr(CL_GET_LIT( cl, 1)) ]; s++ ) if( bin_lut[ nr(CL_GET_LIT( cl, 1)) ][ s ] == - nr(CL_GET_LIT( cl, 0)) ) bin_lut[ nr(CL_GET_LIT( cl, 1)) ][ s ] = 0;
						}

					 
					 
					 
					 remClOcc (clsidx);
						CL_SET_LIT(cl,0, lit(i) );
						while( CL_SIZE( cl ) > 1 ) CL_DELETE_INDEX( cl, CL_SIZE(cl) - 1 );
						
						if( UNSAT == addSingleClause ( assignment, clsidx) ){
							cerr << "c found UNSAT during HTE" << endl;
							return UNSAT;
						}
						hteUnits[cRound] ++;
						
						unit_flag = 1;
						k--;
				}
	    }
	}

	bin_occ -= (int)var_cnt;
	bin_lut -= (int)var_cnt;

	free( bin_occ );
	free( bin_lut );
	free( bin_que );
	free( bin_table );

	return UNKNOWN;
}

void Coprocessor::fill_hla(lit_t l){
	
}


void Coprocessor::reduceClauses ()
{

#ifdef PP_NO_NEW
	for (uint32_t ind = 0; ind < VEC_SIZE (CL_REF, (*clauses)); ++ind) {
		CLAUSE & cl = gsa.get ((*clauses)[ind]);
		if (CL_IS_IGNORED_AND_DELETED (cl)) {
			if( CL_IS_LEARNT(cl) && CL_SIZE(cl) == 2 ){
				cerr << "c remove learned binary clause in PP" << endl;
			}
			CL_DESTROY (cl);
			gsa.release ((*clauses)[ind]);
			VEC_ERASE_NO_ORDER( CL_REF, (*clauses), ind );
			ind --;	
		} else if( CL_IS_IGNORED( cl ) ) {
			if( CL_IS_LEARNT(cl) && CL_SIZE(cl) == 2 ){
				cerr << "c remove learned binary clause in PP" << endl;
			}
			VEC_ERASE_NO_ORDER( CL_REF, (*clauses), ind );
			ind --;	
		}
	}
#else
	
	VEC_CREATE (CL_REF, clausesR);
	VEC_RESIZE (CL_REF, clausesR, VEC_SIZE (CL_REF, (*clauses)), 0);

	uint32_t ind_cr = 0;
	for (uint32_t ind = 0; ind < VEC_SIZE (CL_REF, (*clauses)); ++ind) {
		CLAUSE & cl = gsa.get ((*clauses)[ind]);
		if (!CL_IS_IGNORED (cl)) {
			clausesR[ind_cr] = (*clauses)[ind];
			ind_cr++;
		} else if (CL_IS_IGNORED_AND_DELETED (cl)) {
			if( CL_IS_LEARNT(cl) && CL_SIZE(cl) == 2 ){
				cerr << "c remove learned binary clause in PP" << endl;
			}
			CL_DESTROY (cl);
			gsa.release ((*clauses)[ind]);
		} else {
			if( CL_IS_LEARNT(cl) && CL_SIZE(cl) == 2 ){
				cerr << "c remove learned binary clause in PP" << endl;
			}
		}
	}

	VEC_DESTROY (CL_REF, (*clauses));
	VEC_RESIZE (CL_REF, clausesR, ind_cr, 0);
	(*clauses) = VEC_COPY (CL_REF, clausesR);
	VEC_DESTROY (CL_REF, clausesR);
#endif
}


bool Coprocessor::enqueue(assi_t assignment, const lit_t l){
	if ( assi_is_unsat( assignment, l) )	return UNSAT;
	if ((assi_get_polarity (assignment, var(l)) == UNDEF)){
		assi_set_polarity (assignment, var(l), pol(l));
		VEC_PUSH_BACK( lit_t, unitQ, l );
	}
	return true;
}


solution_t Coprocessor::propagate (assi_t assignment)
{
	
	while (VEC_SIZE (lit_t, unitQ) != 0) {
		
		const lit_t uLit = unitQ[VEC_SIZE (lit_t, unitQ) - 1];
		const var_t uVar = var (uLit);
		const pol_t uPol = pol (uLit);
		VEC_POP_BACK (lit_t, unitQ);
	
		if ( assi_is_unsat( assignment, uLit) )	{
			#ifdef TRACK_CLAUSE
			cerr << "c UNSAT while propagating " << uVar << endl;
			#endif
			return UNSAT; 
		}
		prefetchOcc( inv_lit(uLit) );
#ifdef TRACK_CLAUSE
		if( uVar == var( trackme) ) cerr << "c propagate tracked variable " << uVar << endl;
#endif

		if ((assi_get_polarity (assignment, uVar) == UNDEF)){
			assi_set_polarity (assignment, uVar, uPol);
		}
		for (uint32_t i = 0; i < VEC_SIZE (CL_REF, occ[ index( inv_lit(uLit) )]); ++i) {
			CL_REF clause = occ[ index( inv_lit(uLit) )][i];
			CLAUSE & cl = gsa.get (clause);
			if (CL_IS_IGNORED (cl)) continue;
			if (CL_SIZE (cl) == 1){
				#ifdef TRACK_CLAUSE
				cerr << "c UNSAT while propagating " << uVar << endl;
				#endif
				return UNSAT ;
			}
			
			VEC_ERASE_NO_ORDER( CL_REF, occ[ index( inv_lit(uLit) )], i );
			i--;
			CL_DELETE_LIT (cl, inv_lit (uLit));
			
			if (CL_SIZE (cl) == 1) {
				if( !enqueue(assignment,  CL_GET_LIT (cl, 0)) ){
					#ifdef TRACK_CLAUSE
					cerr << "c UNSAT while propagating " << uVar << endl;
					#endif
					return UNSAT ;
				}
				
				removedPropCls[cRound]++;
			} else {
				for (uint32_t j = 0; j < CL_SIZE (cl); ++j){
					boolarray_set (touchedVars, var( CL_GET_LIT(cl, j) ), true);
				}
			}
		}
		
		for (uint32_t i = 0; i < VEC_SIZE (CL_REF, occ[ index( uLit )]); ++i) {
			CL_REF clause = occ[ index( uLit )][i];
			CLAUSE & cl = gsa.get (clause);
			remClOcc (clause);
			if (CL_IS_IGNORED (cl)) continue;
			CL_IGNORE_AND_DELETE (cl);
			for (uint32_t j = 0; j < CL_SIZE (cl); ++j)	boolarray_set (touchedVars, var (CL_GET_LIT (cl, j)), true);
			removedPropCls[cRound]++;
		}
	}
	return UNKNOWN;
}


void Coprocessor::createBinaryClauses( assi_t assignment, lit_t first, VEC_TYPE(lit_t)& others){
	lit_t lits[2];
	lits[0] = inv_lit(first);
	for( uint32_t i = 0 ; i < VEC_SIZE(lit_t,others); ++i){
		lits[1] = others[i];

		CL_REF clause = gsa.create( CL_CREATE_NO_LEARNT( lits, 2 ) );

		CLAUSE& cl = gsa.get(clause);
		


		VEC_PUSH_BACK (CL_REF, (*clauses), clause);
		addedBinaries++;
		
		for (uint32_t j = 0; j < 2; ++j) {
			const lit_t literal = lits[j];
			VEC_PUSH_BACK (CL_REF, occ[ index(literal)], clause);
			vOcc[ var  (literal)]++;
			lOcc[ index(literal)]++;
		}
	}

}

solution_t Coprocessor::addSingleClause (assi_t assignment, const CL_REF newCls){
	CLAUSE & cl = gsa.get (newCls);
	if (CL_SIZE (cl) < 2){
		if( CL_SIZE (cl) == 0 ){
			cerr << "c UNSAT due to enqueueing an empty clause" << endl;
			return UNSAT;
		}
		if( boolarray_get(eliminated, var(CL_GET_LIT(cl,0)) ) ){
			cerr << "c !ERROR! added unit clause with eliminated var " << var(CL_GET_LIT(cl,0)) << endl;
			assert(false);
		}
		const lit_t literal = CL_GET_LIT (cl, 0);
		
		if( !enqueue(assignment,  CL_GET_LIT (cl, 0))){
			cerr << "c UNSAT during enqueueing a unit clause" << endl;
			return UNSAT;
		}
		assi_set_polarity( assignment, var(CL_GET_LIT (cl, 0)), pol(CL_GET_LIT (cl, 0)));
		vOcc[ var   (literal)]++;
		lOcc[ index (literal)]++;
		return UNKNOWN;
	}
	uint32_t undef = 0; lit_t u = NO_LIT;
	maxClLen = (maxClLen >= CL_SIZE (cl)) ? maxClLen : CL_SIZE (cl);
	for (uint32_t j = 0; j < CL_SIZE (cl); ++j) {
		const lit_t literal = CL_GET_LIT (cl, j);
		
#ifdef TRACK_CLAUSE
		if( literal == trackme ){
			cerr << "c add clause to occurences:";
			for( uint32_t i = 0; i < CL_SIZE(cl); i++ ) cerr << " " << nr( CL_GET_LIT(cl,i) );
			cerr << endl;
		}
#endif



		if( boolarray_get(eliminated, var(literal) ) ){
			cerr << "c !ERROR! added clause with eliminated var " << var(literal) << " clause";
			for (uint32_t tj = 0; tj < CL_SIZE (cl); ++tj) cerr << " " << nr( CL_GET_LIT(cl,tj) );
			cerr << endl;
			assert(false);
		}

		VEC_PUSH_BACK (CL_REF, occ[ index(literal)], newCls);
		vOcc[ var  (literal)]++;
		lOcc[ index(literal)]++;
	}
	

	return UNKNOWN;
}

solution_t Coprocessor::fillOcc (assi_t assignment)
{
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		if( UNSAT == addSingleClause(assignment, (*clauses)[i]) ) return UNSAT;
	}
	return UNKNOWN;
}

void Coprocessor::resetStructures ()
{
	for( var_t v = 1; v <=var_cnt; ++v ){
		VEC_CLEAR( CL_REF, occ[index(lit(v,POS))] );
		VEC_CLEAR( CL_REF, occ[index(lit(v,NEG))] );
		
#ifdef TRACK_CLAUSE
		if( var(trackme) == v ) cerr << "c remove variable " << v << endl;
#endif
		
	}
	memset(vOcc, 0 , sizeof(uint32_t) * (var_cnt+1) );
	memset(lOcc, 0, sizeof(uint32_t) * max_index( var_cnt) );

	resQ.clear();
	VEC_CLEAR( CL_REF, subsQ );
	litHeap.clear();
}

void Coprocessor::resetAssi(){
	for( var_t v = 1; v <=var_cnt ; v++){
		if( lAssi[ index(lit(v,POS))] != 0 ){
			assi_destroy( lAssi[ index(lit(v,POS))], var_cnt);
		}
		if( lAssi[ index(lit(v,NEG))] != 0 ){
			assi_destroy( lAssi[ index(lit(v,NEG))], var_cnt);
		}
	}
}

void Coprocessor::postprocess (VEC_TYPE (assi_t) * assignments)
{

	for (uint32_t i = 0; i < VEC_SIZE (assi_t, (*assignments)); ++i){

		

		
		for (int32_t j = VEC_SIZE (postEle, postProcessStack) - 1; j >= 0; --j) {
			const postEle pe = postProcessStack[j];
			if (pe.kind == 'b') {
				
				uint32_t k = 0;
				CLAUSE & cl = gsa.get (pe.b.clause);
				for (; k < CL_SIZE (cl); ++k) {
					const lit_t l = CL_GET_LIT (cl, k);
					if (assi_variable_equal ((*assignments)[i], var (l), pol (l)))
						break;
				}
				if (k == CL_SIZE (cl)){
	#ifdef TRACK_CLAUSE
					if( var(trackme) == var(pe.b.lit1) ){
						cerr << "c tracked: blocked set var " << var (pe.b.lit1) << " to " << pol (pe.b.lit1) << endl;
						cerr << "c bl. clause:";
						for ( uint32_t ti=0; ti < CL_SIZE (cl); ++ti) {
							cerr << " " << nr( CL_GET_LIT (cl, ti) );
						} cerr << endl;
					}
	#endif
					assi_set_polarity ((*assignments)[i], var (pe.b.lit1), pol (pe.b.lit1));
				}
			} else {
				if (pe.kind == 'v') {
					const elimination_t & elim = pe.e;
					
					
					if( ! assi_is_undef( (*assignments)[i], elim.var1 ) ) continue;
			
					uint32_t k = 0;
					for (; k < elim.number; ++k) {
						uint32_t ind;
						lit_t literal = NO_LIT;
						CLAUSE & cl = gsa.get (elim.clauses[k]);
						for (ind = 0; ind < CL_SIZE (cl); ++ind) {
							const lit_t literal_c = CL_GET_LIT (cl, ind);
							const var_t variable_c = var (literal_c);
							
							if (variable_c == elim.var1)
								literal = literal_c;
							else if (assi_is_sat ((*assignments)[i], literal_c))
								break;
						}
						
						if (ind == CL_SIZE (cl)) {
							#ifdef TRACK_CLAUSE
								if( var(trackme) == var(literal) ){
									cerr << "c tracked: elimination set var " << var (literal) << " to " << pol (literal) << endl;
									cerr << "c eliminated clause:";
									for ( uint32_t ti=0; ti < CL_SIZE (cl); ++ti) {
										cerr << " " << nr( CL_GET_LIT (cl, ti) );
									} cerr << endl;
								}
							#endif
							assi_set_polarity ((*assignments)[i], var (literal), pol (literal));
							break;
						}
					}
					
					if( k == elim.number ){
						
						if( max_solutions == -1 || max_solutions > (int32_t)VEC_SIZE( assi_t, (*assignments) ) ){
							assi_t tmp = assi_copy( (*assignments)[i], var_cnt );
							assi_set_polarity( tmp, elim.var1, NEG );
							VEC_PUSH_BACK(assi_t, (*assignments), tmp );
						}
						assi_set_polarity ( (*assignments)[i] , elim.var1, POS );
					}
				
				} else {
					if( pe.kind == 'e' ){

						
						for( var_t var1 = 1 ; var1 < var_cnt+1; var1 ++ ){
							pol_t repl_pos_polarity = assi_get_polarity( (*assignments)[i], var(replacedBy[var1]) );
							if( repl_pos_polarity != UNDEF ){
								if( pol(replacedBy[var1]) == POS ){
									assi_set_polarity( (*assignments)[i], var1, repl_pos_polarity );
								} else {
									assi_set_polarity( (*assignments)[i], var1, inv_pol(repl_pos_polarity) );
								}
							}
						}

					}
			
				}
			}
		}				

	}	
}


void Coprocessor::postprocessSingle (assi_t assi)
{


	
}


bool Coprocessor::doSimplify(searchData& sd){
	
	if( lastTrailSize + simpDiff >= STACK_SIZE( lit_t, sd.trail ) && simplifyRejections < simpEvery ){
		simplifyRejections ++;
		
		return false;
	}
	
	simplifyRejections = 0;
	return true;
}

solution_t Coprocessor::simplifysearch (searchData& sd, VEC_TYPE (CL_REF)& formula, VEC_TYPE (CL_REF)& learnt)
{
	stopTime = get_microseconds() + simpTimeout*1000;
	if( simpTimeout == -1 ) stopTime = -1;

	if( stopTime < get_microseconds() ){
		cerr << "c simp timeout (" << simpTimeout << "s)" << endl;
		return UNKNOWN;
	}

	
	lastTrailSize = STACK_SIZE( lit_t, sd.trail );
#ifndef SILENT
	cerr << "c simplify at trailSize " << STACK_SIZE( lit_t, sd.trail )
	<< " pure(" << simpPure << ")"
	<< " hte(" << simpHte << ")"
	<< " bce(" << simpBlocked << ")"
	<< " ve(" << simpVe << ")"
	<< " ee(" << simpEe << ")"
	<< " probL(" << simpProbL << ")"
	<< " probC(" << simpProbC << ")"
	<< " asymm(" << simpPAsymm << ")"
	<< endl;
	simplifying = true;
#endif
	VEC_CLEAR (CL_REF, clausesR);
	
	resetStructures();
	clauses = &clausesR;
	
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, formula); ++i) {
		CLAUSE& cl = gsa.get( formula[i] );
		if( CL_IS_IGNORED(cl) ) continue;
		uint32_t j = 0;
		for( ; j < CL_SIZE(cl); ++j ) {
			if( assi_is_sat( sd.assi, CL_GET_LIT(cl,j)) ) {
				CL_IGNORE(cl);
				break;
			}
			if( assi_is_unsat(sd.assi, CL_GET_LIT(cl,j))){
				if( CL_SIZE(cl) == 1 ){
					cerr << "c simplify found UNSAT" << endl;
					return UNSAT;
				}
				CL_SWAP_LITS( cl, j, CL_SIZE(cl) - 1);
				CL_DELETE_INDEX( cl,  CL_SIZE(cl) - 1);
				--j;
			}
		}
		if( j != CL_SIZE(cl) ) continue;
		if( UNSAT == addSingleClause(sd.assi, formula[i]) ){
			simplifying = false;
			cerr << "c simplify found UNSAT" << endl;
			return UNSAT;
		}
	}
	VEC_PUSH_BACK_ANOTHER( CL_REF, (*clauses), (&(formula[0])), VEC_SIZE(CL_REF, formula) );

	if( UNSAT == propagate(sd.assi) ){
		cerr << "UNSAT during propagate" << endl;
		simplifying = false;
		return UNSAT;
	}
	uint32_t oldStackSize = postProcessStack.size();

	
	if( stopTime > get_microseconds() && simpPure ){
#ifndef SILENT
		cerr << "c pure" << endl;
#endif
		detectPure(sd.assi);
	}
	if( stopTime > get_microseconds() && simpHte){
#ifndef SILENT
		cerr << "c HTE" << endl;
#endif
		if( UNSAT == hte(sd.assi, ( ( hteP * var_cnt) / 50.0f )) ){
			simplifying = false;
			cerr << "c simplify found UNSAT" << endl;
			return UNSAT;
		}
	}
	if( stopTime > get_microseconds() && simpBlocked ){
#ifndef SILENT
		cerr << "c BCE" << endl;
#endif
		if( UNSAT == remove_blocked(sd.assi) ){
			cerr << "c UNSAT during BCE" << endl;
			simplifying = false;
			return UNSAT;
		}
	}
	if(stopTime > get_microseconds() && simpVe){
		
		VEC_CLEAR( CL_REF, subsQ );
#ifndef SILENT
		cerr << "c VE" << endl;
#endif
		if( UNSAT == eliminate_variables(sd.assi) ){
			simplifying = false;
			cerr << "c simplify found UNSAT" << endl;
			return UNSAT;
		}
	}
	if( stopTime > get_microseconds() && (simpProbL || simpProbC || simpPAsymm)){
		if( UNSAT == hyberBinProb(sd.assi) ){
			cerr << "UNSAT hyperBin" << endl;
			simplifying = false;
			return UNSAT;
		}
	}
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		
		CLAUSE & cl = gsa.get ((*clauses)[i]);
		if( CL_IS_IGNORED( cl ) ) continue;
		
		VEC_PUSH_BACK (CL_REF, subsQ, (*clauses)[i]);
	}
	
	if( UNSAT == propagate(sd.assi) ){
		cerr << "UNSAT during propagate" << endl;
		simplifying = false;
		return UNSAT;
	}

	
	VEC_PUSH_BACK_ANOTHER( CL_REF, (*clauses), (&(learnt[0])), VEC_SIZE(CL_REF, learnt) );
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, learnt); ++i) {
		CLAUSE& cl = gsa.get( learnt[i] );
		if( CL_IS_IGNORED(cl) ) continue;
		uint32_t j = 0;
		
		for( ; j < CL_SIZE(cl); ++j ) {
			const lit_t l = CL_GET_LIT(cl,j);
			
			if( assi_is_sat( sd.assi, l) || boolarray_get(eliminated, var(l) ) ) {
				CL_IGNORE(cl);
				break;
			}
			if( assi_is_unsat(sd.assi, l) ){
				if( CL_SIZE(cl) == 1 ){
					simplifying = false;
					cerr << "c simplify found UNSAT" << endl;
					return UNSAT;
				}
				CL_SWAP_LITS( cl, j, CL_SIZE(cl) - 1);
				CL_DELETE_INDEX( cl,  CL_SIZE(cl) - 1);
				--j;
			}
		}
		if( j != CL_SIZE(cl) ) continue; 
		if( UNSAT == addSingleClause(sd.assi, learnt[i]) ){
			simplifying = false;
			cerr << "c simplify found UNSAT" << endl;
			return UNSAT;
		}
	}

	if(stopTime > get_microseconds() && simpHte){
		cerr << "c HTE" << endl;
		
		
		if( UNSAT == propagate(sd.assi) ){
			cerr << "UNSAT during propagate" << endl;
			simplifying = false;
			return UNSAT;
		}
		
		if( UNSAT == hte(sd.assi, ( ( hteP * var_cnt) / 50.0f )) ){
			simplifying = false;
			cerr << "c simplify found UNSAT" << endl;
			return UNSAT;
		}
		
		
		if( UNSAT == propagate(sd.assi) ){
			cerr << "UNSAT during propagate" << endl;
			simplifying = false;
			return UNSAT;
		}
	}

	if(stopTime > get_microseconds() && simpEe){
		if( searchEqui (sd.assi) ){
			solution = applyEqui(sd.assi);
			if( solution != UNKNOWN ) {
				cerr << "c simplify found UNSAT" << endl;
				return solution;
			}
		}
	}
	cerr << "c subsumption" << endl;
	if( UNSAT == subsSimp(sd.assi) ) {
		cerr << "c UNSAT during subSimp" << endl;
		simplifying = false;
		return UNSAT;
	}
	if( UNSAT == propagate(sd.assi) ){
		cerr << "UNSAT during propagate" << endl;
		simplifying = false;
		return UNSAT;
	} else {
		cerr << "c simplified formula contains " << VEC_SIZE( lit_t, unitQ ) << " unit clauses" << endl;
	}

	
	VEC_CLEAR (CL_REF, formula );
	VEC_CLEAR (CL_REF, learnt );

	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		const CL_REF clause = (*clauses)[i];
		CLAUSE & cl = gsa.get (clause);
		if( !CL_IS_IGNORED(cl) ){
			if( CL_IS_LEARNT(cl)) VEC_PUSH_BACK( CL_REF, learnt, (*clauses)[i]);
			else VEC_PUSH_BACK( CL_REF, formula, (*clauses)[i]);
		} else if( CL_IS_IGNORED_AND_DELETED(cl) ){
			CL_DESTROY (cl);
			gsa.release ((*clauses)[i]);
		}
	}

	VEC_CLEAR (CL_REF, clausesR);

	simplifying = false;
	return UNKNOWN;
}


void Coprocessor::prefetchOcc( const lit_t l ) const {
	if( !usePrefetch ) return;
	
	const VEC_TYPE(CL_REF)& list =  occ[ index(l) ];
	const uint32_t size =  VEC_SIZE( CL_REF, list );
	for ( uint32_t j = 0 ; j < size; j ++ ){
		prefetch( gsa.addr(list[j]) );
	}
}

void Coprocessor::scanClauses (assi_t assignment){
	
	uint32_t unremovedIgnored = 0;
	uint32_t unremovedElim = 0;
	uint32_t unremovedRemoved = 0;
	uint32_t unremovedSat = 0;
	uint32_t partiallyRemoved = 0;
	uint32_t nonExistendLit = 0;
	
	for (uint32_t i = 0; i < VEC_SIZE (CL_REF, (*clauses)); ++i) {
		const CL_REF clause = (*clauses)[i];
		CLAUSE & cl = gsa.get (clause);

		uint32_t size = CL_SIZE(cl);
		uint32_t found = 0;
		bool containsElim =false;
		bool containsRemoved =false;
		bool isSat = false;
		bool ignored = CL_IS_IGNORED(cl);

		
		for( uint32_t j = 0; j < size; j++ ) {
			const lit_t l = CL_GET_LIT(cl,j);
			isSat = isSat || assi_is_sat( assignment, l );
			containsElim = containsElim || boolarray_get( eliminated, var(l) );
			containsRemoved = containsRemoved || boolarray_get( varRemoved, var(l) );
			for(uint32_t xyz = 0; xyz < VEC_SIZE( CL_REF, occ[ index(l) ] ); xyz ++ ){
				if( occ[ index(l) ][xyz] == clause ) { found ++; break; }
			}
		}
		
		if( isSat > 0 && found > 0 ){
			
			unremovedSat ++;
		}
		if( found > 0 && containsElim ) unremovedElim++;
		if( found > 0 && containsRemoved ) unremovedRemoved++;
		if( found > 0 && found < size ) partiallyRemoved++;
		if( ignored && found > 0 ) unremovedIgnored ++;
	}
	
	
	for( int i = -(int)(var_cnt); i <= (int) var_cnt; ++ i ){
		if( i == 0 ) continue;
		lit_t literal = lit(i);
		
		VEC_TYPE(CL_REF)& list =  occ[ index(literal) ];
		uint32_t size =  VEC_SIZE( CL_REF, list );
		
		for ( uint32_t j = 0 ; j < size; j ++ ){
			const CLAUSE& cl = gsa.get( list[j] );
			if( CL_IS_IGNORED(cl) ) continue;
			uint32_t k = 0 ;
			for( ; k < CL_SIZE(cl); k ++ ){
				if( CL_GET_LIT(cl,k) == literal ) break;
			}
			if( k == CL_SIZE(cl) ){
				nonExistendLit++;
			}
		}
	
	}
	
	
	
#ifdef TRACK_CLAUSE
		cerr << "c occurence list for tracked literal " << nr( trackme ) << " after blocked " << endl;
		for( uint32_t xyz = 0; xyz < VEC_SIZE( CL_REF, occ[ index(trackme) ] ); xyz ++ ){
			const CLAUSE& cl = gsa.get( occ[ index(trackme) ][xyz]);
			for( uint32_t xy = 0; xy < CL_SIZE( cl ); xy ++ ) cerr << nr( CL_GET_LIT( cl, xy ) ) << " ";
			cerr << endl;
		}
#endif

	cerr << "unremoved sat:        " << unremovedSat << endl;
	cerr << "unremoved eliminated: " << unremovedElim << endl;
	cerr << "unremoved removed:    " << unremovedRemoved << endl;
	cerr << "partially removed:    " << partiallyRemoved << endl;
	cerr << "unremoved ignored:    " << unremovedIgnored << endl;
	cerr << "non existend lit :    " << nonExistendLit << endl;
}

void Coprocessor::set_parameter( const StringMap& cm )
{
	
	if( cm.contains( (const char*)"sol" ) != 0 )  max_solutions = atoi( cm.get( (const char*)"sol" ).c_str() );
	
#ifdef USE_COMMANDLINEPARAMETER
	if( cm.contains( (const char*)"CP_debug" ) ) debug = atoi( cm.get( (const char*)"CP_debug" ).c_str() );
	if( cm.contains( (const char*)"CP_print" ) ) print_dimacs = (0 != atoi( cm.get( (const char*)"CP_print" ).c_str() ));
	if( cm.contains( (const char*)"CP_pure" ) ) pure = atoi( cm.get( (const char*)"CP_pure" ).c_str() );
	if( cm.contains( (const char*)"CP_ppure" ) ) prop_pure = atoi( cm.get( (const char*)"CP_ppure" ).c_str() );
	if( cm.contains( (const char*)"CP_bce" ) ) blocked = atoi( cm.get( (const char*)"CP_bce" ).c_str() );
	if( cm.contains( (const char*)"CP_ve" ) ) ve = atoi( cm.get( (const char*)"CP_ve" ).c_str() );
	if( cm.contains( (const char*)"CP_hte" ) ) hteR = atoi( cm.get( (const char*)"CP_hte" ).c_str() );
	if( cm.contains( (const char*)"CP_ee" ) ) ee = atoi( cm.get( (const char*)"CP_ee" ).c_str() );
	if( cm.contains( (const char*)"CP_er" ) ) er = atoi( cm.get( (const char*)"CP_er" ).c_str() );
	if( cm.contains( (const char*)"CP_probSize" ) ) probSize = atoi( cm.get( (const char*)"CP_probSize" ).c_str() );
	if( cm.contains( (const char*)"CP_probL" ) ) probL = atoi( cm.get( (const char*)"CP_probL" ).c_str() );
	if( cm.contains( (const char*)"CP_probC" ) ) probC = atoi( cm.get( (const char*)"CP_probC" ).c_str() );
	if( cm.contains( (const char*)"CP_asymm" ) ) asymm = atoi( cm.get( (const char*)"CP_asymm" ).c_str() );
	if( cm.contains( (const char*)"CP_ternary" ) ) ternary = atoi( cm.get( (const char*)"CP_ternary" ).c_str() );
	if( cm.contains( (const char*)"CP_rounds" ) ) rounds = atoi( cm.get( (const char*)"CP_rounds" ).c_str() );
	if( cm.contains( (const char*)"CP_mbSize" ) ) blockMaxSize = atoi( cm.get( (const char*)"CP_mbSize" ).c_str() );
	if( cm.contains( (const char*)"CP_mbOcc" ) ) blockMaxOcc = atoi( cm.get( (const char*)"CP_mbOcc" ).c_str() );
	if( cm.contains( (const char*)"CP_mVEcls" ) ) maxVEcls = atoi( cm.get( (const char*)"CP_mVEcls" ).c_str() );
	if( cm.contains( (const char*)"CP_mQsize" ) ) maxQsize = atoi( cm.get( (const char*)"CP_mQsize" ).c_str() );
	if( cm.contains( (const char*)"CP_maxCls" ) ) maxCls = atoi( cm.get( (const char*)"CP_maxCls" ).c_str() );
	if( cm.contains( (const char*)"CP_crBin" ) ) createBinaries = (0 != atoi( cm.get( (const char*)"CP_crBin" ).c_str() ));
	if( cm.contains( (const char*)"CP_vProp" ) ) vePropagate = (0 != atoi( cm.get( (const char*)"CP_vProp" ).c_str() ));
	if( cm.contains( (const char*)"CP_vizBi" ) ) dumpBiGraph = (0 != atoi( cm.get( (const char*)"CP_vizBi" ).c_str() ));
	if( cm.contains( (const char*)"CP_Spure" ) ) simpPure = (0 != atoi( cm.get( (const char*)"CP_Spure" ).c_str() ));
	if( cm.contains( (const char*)"CP_Sbce" ) ) simpBlocked = (0 != atoi( cm.get( (const char*)"CP_Sbce" ).c_str() ));
	if( cm.contains( (const char*)"CP_Sve" ) ) simpVe = (0 != atoi( cm.get( (const char*)"CP_Sve" ).c_str() ));
	if( cm.contains( (const char*)"CP_Shte" ) ) simpHte = (0 != atoi( cm.get( (const char*)"CP_Shte" ).c_str() ));
	if( cm.contains( (const char*)"CP_See" ) ) simpEe = (0 != atoi( cm.get( (const char*)"CP_See" ).c_str() ));
	if( cm.contains( (const char*)"CP_Sss" ) ) simpSubsSimp = (0 != atoi( cm.get( (const char*)"CP_Sss" ).c_str() ));
	if( cm.contains( (const char*)"CP_prefetch" ) ) usePrefetch = (0 != atoi( cm.get( (const char*)"CP_prefetch" ).c_str() ));
	if( cm.contains( (const char*)"CP_SprobL" ) ) simpProbL = (0 != atoi( cm.get( (const char*)"CP_SprobL" ).c_str() ));
	if( cm.contains( (const char*)"CP_SprobC" ) ) simpProbC = (0 != atoi( cm.get( (const char*)"CP_SprobC" ).c_str() ));
	if( cm.contains( (const char*)"CP_Sasymm" ) ) simpPAsymm = (0 != atoi( cm.get( (const char*)"CP_Sasymm" ).c_str() ));
	if( cm.contains( (const char*)"CP_prVar" ) ) probVars = atoi( cm.get( (const char*)"CP_prVar" ).c_str() );
	if( cm.contains( (const char*)"CP_prVarPer" ) ) probVarsP = atoi( cm.get( (const char*)"CP_prVarPer" ).c_str() );
	if( cm.contains( (const char*)"CP_prClVar" ) ) probClVars = atoi( cm.get( (const char*)"CP_prClVar" ).c_str() );
	if( cm.contains( (const char*)"CP_prClMaxS" ) ) probClMaxS = atoi( cm.get( (const char*)"CP_prClMaxS" ).c_str() );
	if( cm.contains( (const char*)"CP_prClMaxN" ) ) probClMaxN = atoi( cm.get( (const char*)"CP_prClMaxN" ).c_str() );
	if( cm.contains( (const char*)"CP_hteP" ) ) hteP = atof( cm.get( (const char*)"CP_hteP" ).c_str() );
	if( cm.contains( (const char*)"CP_erPairs" ) ) erPairs = atoi( cm.get( (const char*)"CP_erPairs" ).c_str() );
	if( cm.contains( (const char*)"CP_erMaxV" ) ) erMaxV = atoi( cm.get( (const char*)"CP_erMaxV" ).c_str() );
	if( cm.contains( (const char*)"CP_erMinV" ) ) erMinV = atoi( cm.get( (const char*)"CP_erMinV" ).c_str() );
	if( cm.contains( (const char*)"CP_erMSize" ) ) erMinSize = atoi( cm.get( (const char*)"CP_erMSize" ).c_str() );
	if( cm.contains( (const char*)"CP_erTries" ) ) erTries = atoi( cm.get( (const char*)"CP_erTries" ).c_str() );
	if( cm.contains( (const char*)"CP_erIters" ) ) erIters = atoi( cm.get( (const char*)"CP_erIters" ).c_str() );
	if( cm.contains( (const char*)"CP_erMinO" ) ) erMinO = atoi( cm.get( (const char*)"CP_erMinO" ).c_str() );
	if( cm.contains( (const char*)"CP_aMin" ) ) aMinSize = atoi( cm.get( (const char*)"CP_aMin" ).c_str() );
	if( cm.contains( (const char*)"CP_aPer" ) ) aPercent = atoi( cm.get( (const char*)"CP_aPer" ).c_str() );
	if( cm.contains( (const char*)"CP_simpEvery" ) ) simpEvery = atoi( cm.get( (const char*)"CP_simpEvery" ).c_str() );
	if( cm.contains( (const char*)"CP_simpDiff" ) ) simpDiff = atoi( cm.get( (const char*)"CP_simpDiff" ).c_str() );
	if( cm.contains( (const char*)"CP_pTime" ) ) ppTimeout = atoi( cm.get( (const char*)"CP_pTime" ).c_str() );
	if( cm.contains( (const char*)"CP_sTime" ) ) simpTimeout = atoi( cm.get( (const char*)"CP_sTime" ).c_str() );
	blackFile = cm.get( (const char*)"CP_blackFile" );
	whiteFile = cm.get( (const char*)"CP_whiteFile" );
	
	if( probVarsP > 100 ) probVarsP = 100;
#endif

	
	if( max_solutions != 1 ){
		simpPure = false;
		simpBlocked = false;
		pure = 0;
		blocked = 0;
	}

#ifndef TESTBINARY
#ifndef COMPETITION
	
	if( cm.contains( (const char*)"-h" ) || cm.contains( (const char*)"--help" ) )
	{
		cerr << "=== Coprocessor preprocessor information ===" << endl;
		cerr << " parameter  values info" << endl;
		cerr << " CP_print    0,1    print resulting formula" << endl;
		cerr << " CP_whiteFile name  name of variable-file of variables that must be eliminated" << endl;
		cerr << " CP_blackFile name  name of variable-file of variables has to be kept" << endl;
		cerr << " pp algorithms:" << endl;
		cerr << " CP_pure      0-n    rounds to perform pure lit elimination" << endl;
		cerr << " CP_ppure     0-n    rounds to propagate after plit elimination" << endl;
		cerr << " CP_bce       0-n    rounds to apply BCE" << endl;
		cerr << " CP_ve        0-n    rounds to apply VE" << endl;
		cerr << " CP_hte       0-n    rounds to apply HTE" << endl;
		cerr << " CP_ee        0-n    rounds to apply Equivalence Elimination" << endl;
		cerr << " CP_er        0-n    rounds to apply Extended Resolution" << endl;
		cerr << " CP_probL     0-n    rounds to apply failed literal probing" << endl;
		cerr << " CP_probC     0-n    rounds to apply clause probing" << endl;
		cerr << " CP_asymm     0-n    rounds to apply asymmetric branching" << endl;
		cerr << " CP_ternary   0-n    rounds to apply ternary resolution" << endl;
		cerr << " CP_rounds    0-n    maximal number of rounds" << endl;
		cerr << " simp algorithms:" << endl;
		cerr << " CP_Spure     0,1    pure lit elimination during simplification" << endl;
		cerr << " CP_Sbce      0,1    BCE during simplification" << endl;
		cerr << " CP_Sve       0,1    VE during simplification" << endl;
		cerr << " CP_Shte      0,1    HTE during simplification" << endl;
		cerr << " CP_See       0,1    Equivalence Elimination during simplification" << endl;
		cerr << " CP_Sss       0,1    Subsumption during simplification" << endl;
		cerr << " CP_SprobL    0,1    FLP during simplification" << endl;
		cerr << " CP_SprobC    0,1    clause probing during simplification" << endl;
		cerr << " CP_Sasymm    0,1    asymm. branching during simplification" << endl;
		cerr << " algorithm parameter:" << endl;
		cerr << " CP_pTime     -1-n   milli-seconds for preprocessing" << endl;
		cerr << " CP_sTime     -1-n   milli-seconds for a single simplification step" << endl;
		cerr << " CP_maxCls    -1-n   maximal number of clauses for preprocessing (-1 = any)" << endl;
		cerr << " CP_mVEcls    -1-n   maximal number of clauses for VE (-1 = all)" << endl;
		cerr << " CP_mQsize    -1-n   maximal number of queue elements for subsumtion and elimination(-1 = all)" << endl;
		cerr << " CP_mbSize    -1-n   maximal size of clause for BCE (-1 = any)" << endl;
		cerr << " CP_mbOcc     -1-n   maximal lit occurrence for BCE (-1 = any)" << endl;
		cerr << " CP_hteP      0-100  percent of literals for hte" << endl;
		cerr << " CP_vizBi     0,1    dump implication graphs of binary clauses" << endl;
		cerr << " CP_probSize  0-n    max. clause size that is used for probing (current most possible: 3)" << endl;
		cerr << " CP_crBin     0,1    create binary clause (hyper bin. resolution)" << endl;
		cerr << " CP_vProp     0,1    apply unit propagation during VE" << endl;
		cerr << " CP_prVar     -1-n   #vars for failed literal probing (-1 = all)" << endl;
		cerr << " CP_prVarPer  0-100  percent of variables to probe (limited by prVar)" << endl;
		cerr << " CP_prClVar   -1-n   #literals for clause probing (-1 = all)" << endl;
		cerr << " CP_prClMaxS  -1-n   maximum clause size for clause probing (-1 = any)" << endl;
		cerr << " CP_prClMaxN  -1-n   maximum nr of clauses per literal for clause probing (-1 = any)" << endl;
		cerr << " CP_aMin      -1-n   min. size of clause for asymmetric branching (-1 = any)" << endl;
		cerr << " CP_aPer      0-n    max. percent of clause size to try asymm. branching" << endl;
		cerr << " CP_erMaxV    0-n    maximum number variables, s.t. er is still performed" << endl;
		cerr << " CP_erMinV    0-n    perform er, s.t. at least this many variables are in the formula" << endl;
		cerr << " CP_erPairs   0-n    maximum number of pairs that are removed from the formula" << endl;
		cerr << " CP_erMSize   0-n    minimum clause size that is considered for removing pairs Z(0=all)" << endl;
		cerr << " CP_erMinO    0-n    minimum occurence before a pair is replaced" << endl;
		cerr << " CP_erTries   0-n    factor for pairs, how many more pairs are analyzed to find the maximum" << endl;
		cerr << " CP_erIters   0-n    how many times will the number of er-replacements will be repeated (iters*pairs new variables in total)" << endl;
		cerr << " CP_simpDiff  1-n    number of new trail literals to trigger next simplification" << endl;
		cerr << " CP_simpEvery 1-n    force simplification after X requests" << endl;
		cerr << " CP_prefetch  0,1    use prefetching during preprocessing" << endl;
		cerr << endl;
	}	
#endif
#endif

	
	if( whiteFile.size() != 0 ){
		VarFileParser vfp( whiteFile );
		VEC_TYPE(var_t) evars;
		VEC_CREATE(var_t,evars);
		vfp.extract(evars);
		for( uint32_t i = 0; i < VEC_SIZE(var_t, evars); ++i ){
			if( evars[i] <= var_cnt ) boolarray_set( elimWhite, evars[i], true);
		}		
		
	}

}



#ifdef COMPILE_LIBRARY

extern "C" Coprocessor* create_preprocessor(VEC_TYPE( CL_REF )* clause_set, uint32_t var_cnt, const StringMap& commandline ){
	return new Coprocessor( clause_set, var_cnt, commandline );
}

extern "C" void destroy_preprocessor( Coprocessor* heuristic ){
	delete heuristic;
}
#endif
