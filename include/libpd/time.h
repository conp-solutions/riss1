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

#ifndef _LIBPD_SOLVER_TIME_H
#define _LIBPD_SOLVER_TIME_H

#include <sys/time.h>

#include <libpd/common/types.h>
#include <libpd/solver/output.h>
#include <libpd/solver/pfdata.h>

extern libpd_time_t libpd_initial_timestamp;
void libpd_set_initial_timestamp(void);


#define __LIBPD_READ_TIMEVAL(___val) 	((( (libpd_time_t) (___val).tv_sec ) * ((libpd_time_t)1000000)) + ( (libpd_time_t) (___val).tv_usec ))


#define __LIBPD_GET_TIMEVAL() \
	({\
		struct timeval __tv_start;\
		\
		gettimeofday(&__tv_start, NULL);\
		(( (libpd_time_t) __tv_start.tv_sec ) * ((libpd_time_t)1000000) + ( (libpd_time_t) __tv_start.tv_usec ));\
	})

#ifdef _LIBPD_ACTIVATED
	
	#define LIBPD_START_TIMER(___name) \
		if (___libpd_pfdata_collect_##___name)\
		{\
			struct timeval __tv_start;\
			\
			gettimeofday(&__tv_start, NULL);\
			___libpd_pfdata_value_##___name = (( (libpd_time_t) __tv_start.tv_sec ) * ((libpd_time_t)1000000) + ( (libpd_time_t) __tv_start.tv_usec )) - ___libpd_pfdata_value_##___name;\
		}

	
	#define LIBPD_END_TIMER(___name) \
		if (___libpd_pfdata_collect_##___name)\
		{\
			struct timeval __tv_end;\
			\
			gettimeofday(&__tv_end, NULL);\
			___libpd_pfdata_value_##___name = (( (libpd_time_t) __tv_end.tv_sec ) * ((libpd_time_t)1000000) + ( (libpd_time_t) __tv_end.tv_usec )) - ___libpd_pfdata_value_##___name;\
			\
			if (___libpd_pfdata_fast_write_##___name) {\
				libpd_time_t ___data = ___libpd_pfdata_value_##___name;\
				LIBPD_WRITE_VALUE(___name, &___data);\
			}\
		}

#else

	#define LIBPD_START_TIMER(___name)
	#define LIBPD_END_TIMER(___name)	

#endif

	
	#define LIBPD_CURRENT_EXECUTION_TIMESTAMP() \
		({\
			struct timeval __tv_end;\
			libpd_time_t ___time;\
			\
			gettimeofday(&__tv_end, NULL);\
			___time = ((libpd_time_t)(( (libpd_time_t) __tv_end.tv_sec ) * ((libpd_time_t)1000000)) + (libpd_time_t)( (libpd_time_t) __tv_end.tv_usec )) - (libpd_time_t)libpd_initial_timestamp;\
			\
			___time;\
		})	

#endif

