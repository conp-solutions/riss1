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

#ifndef RISSMAIN_H
#define RISSMAIN_H

#include "sat/component.h"

#include "utils/stringmap.h"
#include "utils/microtime.h"

#include "structures/c_assignment.h"
#include "structures/literal_system.h"

#include "macros/clause_macros.h" 
#include "utils/clause_container.h"
#include "macros/vector_macros.h"

#include "types.h"
#include "sat/solver.h"


int32_t rissmain( StringMap& commandline );

#endif

#endif 
