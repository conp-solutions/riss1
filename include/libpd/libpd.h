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

#ifndef _LIBPD_LIBPD_H
#define _LIBPD_LIBPD_H

#ifndef _LIBPD_ACTIVATED

	
	#define LIBPD_INIT();

	
	#define LIBPD_IMPORT(___name, ___type_const)
	#define LIBPD_WRITE_VALUE(___name, ___pointer)
	#define LIBPD_WRITE_ARRAY(___name, ___pointer, ___size)
	#define LIBPD_OPEN_COMPOSITE(___name)
	#define LIBPD_CLOSE_COMPOSITE(___name)
	#define LIBPD_WRITE_CMP_VALUE(___name, __key, ___pointer)
	#define LIBPD_WRITE_CMP_ARRAY(___name, __key, ___pointer, ___size)
	
	
	#define LIBPD_COUNTER_SET(___name, ___value)
	#define LIBPD_COUNTER_INC(___name)
	#define LIBPD_COUNTER_DEC(___name)	
	
	
	#define LIBPD_FLOAT_SET(___name, ___value)

	
	#define LIBPD_START_TIMER(___name)
	#define LIBPD_END_TIMER(___name)
	
#else

	#include <libpd/solver.h>
	#include "time.h"
	
#endif

#endif

