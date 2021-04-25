/*
	lock.hh
	This file is part of riss.
	
	10.01.2011
	Copyright 2011 Norbert Manthey
*/

#ifndef __LOCK_H
#define __LOCK_H

#include <queue>

#include <pthread.h>
#include <semaphore.h>

#include <iostream>

class Lock{

private:

	sem_t _lock;	// actual semaphore
	int _max;		// users for the semaphore
public:

	/** create an unlocked lock
	*
	* @param max specify number of maximal threads that have entered the semaphore
	*/
	Lock(int max = 1)
	 : _max( max )
	{
		// create semaphore with no space in it
		sem_init(&(_lock), 0, max);
	}
	
	/** release all used resources
	*/
	~Lock(){
		
	}
	
	/** tries to lock
	*
	* 
	* @return true, if locking was successful
	*/
	bool lock(){
		int err = sem_trywait( &_lock );
		return err == 0;
	}
	//@param transitive allow multiple locking of the same thread ?
	
	/** releases the lock again
	*
	* should only be called by the thread that is currently owns the lock
	*/
	void unlock(){
		sem_post( &_lock );
	}

	/** waits until the lock is given to the calling thread
	*/	
	void wait(){
		sem_wait( &_lock );
	}

};

#endif
