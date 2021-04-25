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


#include "utils/conflictgraph.h"



void ConflictDump::addVec(std::string name, VEC_TYPE(lit_t)& lits, bool bigskip ){
	std::stringstream stream;
	stream << name << ":";
	for(uint32_t i = 0 ; i < VEC_SIZE( lit_t, lits ); ++i ) stream << " " << nr(lits[i]);
	addInfo( stream, bigskip);
}

void ConflictDump::addCls(std::string name, CL_REF cls, bool bigskip){
	std::stringstream stream;
	stream << name << ":";
	const CLAUSE& cl = gsa.get(cls);
	for(uint32_t i = 0 ; i < CL_SIZE( cl ); ++i ) stream << " " << nr( CL_GET_LIT( cl,i) );
	addInfo( stream, bigskip);
}

void ConflictDump::addInfo(std::stringstream& stream, bool bigskip){
	if(bigskip) myfile << "\\bigskip" << std::endl << std::endl;
	myfile << stream.str() << std::endl;
}

void ConflictDump::finalize_last(){
	myfile << "\\end{document}" << std::endl;
  myfile.close();
}

void ConflictDump::dump(int conflictNr, CL_REF conflict_clause, searchData& search ){
	
	std::stringstream filename;
	filename << conflictNr << "_conflict.tex";
	
	if( myfile.is_open() ) finalize_last();
	
  myfile.open ( filename.str().c_str() );
	
	TikZProlog(myfile);
	
	myfile << "Trail:";
	for(uint32_t i = 0 ; i < STACK_SIZE( lit_t, search.trail ); ++i ){
		const var_t var1 = var(search.trail[i]);
		if( search.VAR_LEVEL(var1) > 0 && search.VAR_REASON(var1).getCL() == 0 ) myfile << " ]";	
		myfile << " " << nr( search.trail[i] );
	}
	myfile << "\\\\" << std::endl;
	
	myfile << "Level:";
	for(uint32_t i = 0 ; i < STACK_SIZE( lit_t, search.trail ); ++i ){
		const var_t var1 = var(search.trail[i]);
		if( search.VAR_LEVEL(var1) > 0 && search.VAR_REASON(var1).getCL() == 0 ) myfile << " ]";	
		myfile << " " << search.VAR_LEVEL( var(search.trail[i]));
	}
	myfile << "\\\\" << std::endl;
	
	myfile << "Reason:";
	for(uint32_t i = 0 ; i < STACK_SIZE( lit_t, search.trail ); ++i ){
		const var_t var1 = var(search.trail[i]);
		if( search.VAR_LEVEL(var1) > 0 && search.VAR_REASON(var1).getCL() == 0 ) myfile << " ]";	
		myfile << " " << (search.VAR_REASON( var(search.trail[i])).getCL() != 0 );
	}
	myfile << "\\\\" << std::endl;
	
	for(uint32_t i = 0 ; i < STACK_SIZE( lit_t, search.trail ); ++i ){
		const lit_t lit1 = search.trail[i];
		const var_t var1 = var(search.trail[i]);
		
		const CL_REF reason = search.VAR_REASON(var1).getCL();
		if( reason == 0 ) continue;
		
		CLAUSE& rcl = gsa.get(reason);
		for( uint32_t j = 0 ; j < CL_SIZE( rcl ); ++j ){
			lit_t cl_lit1 = CL_GET_LIT( rcl, j );
			if( cl_lit1 != lit1 ) myfile << " " << nr( cl_lit1 );
		}
		myfile << " $\\rightarrow$ " << nr( lit1 ) << "\\\\" << std::endl;
	}


	myfile << "\\bigskip" << std::endl << std::endl;
	
	
	
	myfile << "\\MakeShortVerb{\\|}" << std::endl;
	myfile << "\\begin{tikzpicture}[scale=1]" << std::endl;
	
	lit_t llit1 = NO_LIT;
	lit_t ldec = NO_LIT;
	myfile << "% nodes for all literals on current trail */" << std::endl;
	myfile << "\\node (l" << nr(llit1) << ") [circle,draw] {root};" << std::endl;
	for(uint32_t i = 0 ; i < STACK_SIZE( lit_t, search.trail ); ++i ){
		const lit_t lit1 = search.trail[i];
		if( search.VAR_LEVEL( var( lit1 ) ) == 0 ){	
			myfile << "\\node (l"<< nr(lit1) <<") [circle, draw, right=of l" << nr(llit1) << "] {$l"<< nr(lit1) <<"$};" << std::endl;
		} else {
			if( search.VAR_REASON( var(lit1) ).getCL() == 0 ){	
				myfile << "% next decision level */" << std::endl;
				myfile << "\\node (l"<< nr(lit1) <<") [circle, draw, below=of l" << nr(ldec) << "] {$l"<< nr(lit1) <<"$};" << std::endl;
				ldec = lit1;
			} else {	
				myfile << "\\node (l"<< nr(lit1) <<") [circle, draw, right=of l" << nr(llit1) << "] {$l"<< nr(lit1) <<"$};" << std::endl;
			}
		}
		llit1 = lit1;
	}
	
	std::set<var_t> last_level_var1s;
	CLAUSE& ccl = gsa.get( conflict_clause );
	for( uint32_t i = 0 ; i < CL_SIZE( ccl ); ++i ){
		if( search.VAR_LEVEL( var(CL_GET_LIT( ccl, i ) ) ) == search.current_level ) last_level_var1s.insert( var( CL_GET_LIT( ccl, i ) ) );
	}
	
	lit_t clit1 = NO_LIT; 
	for(int32_t i = STACK_SIZE( lit_t, search.trail ) - 1; i>=0; i-- ){
		if( last_level_var1s.find( var(search.trail[i]) ) != last_level_var1s.end() ){
			clit1 = inv_lit(search.trail[i]); break;
		}
	}
	myfile << "% add conflict to graph */" << std::endl;
	
	myfile << "\\node (l"<< nr(clit1) <<") [circle, draw, below=of l" << nr( inv_lit(clit1)) << "] {$l"<< nr(clit1) <<"$};" << std::endl;
	myfile << "\\node (conflict) [circle, draw, right=of l" << nr( inv_lit(clit1)) << "] {$C$};" << std::endl;
	
	myfile << "% connections between nodes */" << std::endl;
	
	
	for(int32_t i = STACK_SIZE( lit_t, search.trail ) - 1; i>=0; i-- ){
		const lit_t lit1 = search.trail[i];
		const var_t var1 = var(lit1);
		if( search.VAR_REASON( var1 ).getCL() == 0 ) continue;
		if( search.VAR_LEVEL( var1 ) == 0 ) break;	
		
		CL_REF reason = search.VAR_REASON( var1 ).getCL();
		CLAUSE& rcl = gsa.get( reason );
		for(uint32_t j = 0 ; j < CL_SIZE( rcl ); ++j ){
			const lit_t clause_lit1 = CL_GET_LIT( rcl, j );
			if( clause_lit1 == lit1 ) continue;
		
			myfile << "\\draw [->] (l" << nr( inv_lit(clause_lit1) ) << ") to [thick] (l" << nr(lit1) << ");" << std::endl;
		}
	}
	
	myfile << "% arrows in conflict clause */" << std::endl;
	for( uint32_t i = 0 ; i < CL_SIZE( ccl ); ++i ){
		const lit_t clause_lit1 = CL_GET_LIT( ccl,i);
		if( clause_lit1 == clit1 ) continue;
		myfile << "\\draw [->] (l" << nr( inv_lit(clause_lit1) ) << ") to [thick] (l" << nr(clit1) << ");" << std::endl;
	}
	
	myfile << "% arrows to conflict */" << std::endl;
	
	myfile << "\\draw [->] (l" << nr( clit1 ) << ") to [thick] (conflict);" << std::endl;
	myfile << "\\draw [->] (l" << nr( inv_lit(clit1) ) << ") to [thick] (conflict);" << std::endl;
	
  myfile << "\\end{tikzpicture}" << std::endl;

	{
		myfile << "\\bigskip" << std::endl << std::endl;
		myfile << "conflict:";
		const lit_t lit1 = clit1;
		const CL_REF reason = conflict_clause;
		CLAUSE& rcl = gsa.get( reason );
		for( uint32_t j = 0 ; j < CL_SIZE( rcl ); ++j ){
			lit_t cl_lit1 = CL_GET_LIT( rcl, j );
			if( cl_lit1 != lit1 ) myfile << " " << nr( cl_lit1 );
		}
		myfile << " $\\rightarrow$ " << nr( lit1 ) << "\\\\" << std::endl;
	}

}

void ConflictDump::TikZProlog(std::ofstream& file){
	
	file << "\\documentclass{minimal}" << std::endl;
	file << "\\special{papersize=48in,36in}" << std::endl;
	file << "\\setlength{\\paperwidth}{48in}" << std::endl;
	file << "\\setlength{\\paperheight}{36in}" << std::endl;
	file << "\\setlength{\\textwidth}{46in}" << std::endl;
	file << "\\setlength{\\textheight}{34in}" << std::endl;
	file << "\\topskip0pt" << std::endl;
	file << "\\setlength{\\headheight}{0pt}" << std::endl;
	file << "\\setlength{\\headsep}{0pt}" << std::endl;
	file << "\\setlength{\\topmargin}{0pt}" << std::endl;
	file << "\\setlength{\\oddsidemargin}{0pt}" << std::endl;
	file << "\\usepackage[latin1]{inputenc}" << std::endl;
	file << "\\usepackage{shortvrb}" << std::endl;
	file << "\\setlength\\parindent{0pt}" << std::endl;
	file << "\\setlength\\parskip{10pt}" << std::endl;
	file << "\\usepackage{tikz}" << std::endl;
	file << "\\usetikzlibrary{positioning}" << std::endl;
	file << "\\begin{document}" << std::endl;

}

