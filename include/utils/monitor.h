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


#ifndef _MONITOR_H
#define _MONITOR_H



#include "defines.h"

#ifndef USE_DATAMONITOR

	#define MON_REG( name, type, dataT )
	#define MON_EVENT( name, adress )
	#define MON_WAIT_GUI()
	#define MON_WAIT_WINDOW()

#else

	
	#include "DataMonitorLib.hh"

	extern DataMonitor dm;

	
	#define MON_WAIT_GUI() dm.waitForGUI()
	
	
	#define MON_WAIT_WINDOW() dm.waitForWindow()
	
	
	
	#define MON_REG( name, type, dataT ) dm.reg( name, type,dataT )
	#define MON_EVENT( name, adress ) dm.event( name, adress )
	


#endif


#endif
