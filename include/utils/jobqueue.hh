/*
	jobqueue.hh
	This file is part of riss.
	
	25.05.2009
	Copyright 2009 Norbert Manthey
*/

#ifndef __JOBQUEUE
#define __JOBQUEUE

#include "utils/waitfunc.hh"

#include <queue>

#include <pthread.h>
#include <semaphore.h>

#include <iostream>

// TODO: create certain amount of threads using a static function
// handle addJob requests

class JobQueue{

public:
	struct Job{
		void* (*function)(void *argument);
		void* argument;
		Job(){
			function = 0;
			argument = 0;
		};
	};
private:

	std::queue<Job> _jobqueue;
	sem_t _queueLocker;
	size_t _cpus;
	size_t _activecpus;
	size_t _currentWorkerNumber;
	sem_t* _sleepSem;
	volatile int* _threadState;	// -1=toBeDestroyed 0=sleeping 1=working
	volatile int _workState;
	
	pthread_t *_threads;

	size_t getNextWorkerNumber(){
		size_t tmp;
		// lock queue
		sem_wait( &_queueLocker );
		tmp = _currentWorkerNumber;
		_currentWorkerNumber ++;
		// unlock queue
		sem_post( &_queueLocker );	
		return tmp;
	}

	// should be private!
	void wakeUpAll(){
			// TODO: only those, who are sleeping( should be no problem... )
			for(size_t i = 0; i<_cpus; i++){
				// std::cerr << "wake up " << i << std::endl;
				sem_post( & (_sleepSem[i]) );
			}
	}
	
public:

	/** create a job queue for a certain number of cpus/threads
	*/
	JobQueue( size_t cpus=0 ){
		_cpus = 0;
		_workState = 0;
		_activecpus = 0;
		_currentWorkerNumber = 0;
		if( _cpus != 0 ) init(cpus);
	}

	/** init the queue for a number of threads
	*
	* inits only, if the queue has not been initialized before with another number of cpus
	*
	* @param cpus number of working threads
	*/
	void init( size_t cpus ){
		if( _cpus != 0 || cpus == 0 ) return;
		sem_init(&_queueLocker, 0, 1);	// only one can take the semaphore
		_cpus = cpus;
		_activecpus = 0;
		_currentWorkerNumber = 0;
		
		_sleepSem = new sem_t [ _cpus ];
		_threadState = new int [ _cpus ];
		_threads = new pthread_t [ _cpus ];
		
		_workState = 0;
		
		// create threads
			
		// std::cerr << "create threads" << std::endl;
		for(size_t i = 0; i< _cpus; i++){
			// std::cerr << "\t" << i << std::endl;
			sem_init(&(_sleepSem[i]), 0, 0);	// no space in semaphore
			_threadState[i] = 0;
			pthread_create(& (_threads[i]), 0, JobQueue::thread_func , (void*)this);	// create thread
		}
	}

	~JobQueue(){
		setState( -1 );
		wakeUpAll();

	}
	
	/*
	void waitForAll(){
	
	}
	*/
	
	int getThredState( uint32_t thread ){
		return _threadState[thread];
	}

	void setState( int workState ){
		if( _workState == 0 && workState == 1 ){	
			// std::cerr << "wake up all!" << std::endl;
			_workState = workState;
			// set all the workStates before waking the threads up, to do not care about racing conditions!
			for(size_t i = 0; i< _cpus; i++) _threadState[i] = _workState;
			// wake Up all worker
			wakeUpAll();
		}
		// std::cerr << "current workState: " << _workState << std::endl;
		_workState = workState;				
	}

	Job getNextJob(){
		Job j;
		// lock queue
		sem_wait( &_queueLocker );
		if( _jobqueue.size() != 0 )
		{
			j = _jobqueue.front();
			_jobqueue.pop();
		}
		// unlock queue
		sem_post( &_queueLocker );
		return j;
	}
	
	bool addJob( Job j ){
		// lock queue
		sem_wait( &_queueLocker );
		_jobqueue.push( j );
		// unlock queue
		sem_post( &_queueLocker );
		return true;
	}

	/**	returns the size
	*	if multithread is needed, set locked to true
	*/
	size_t size(bool locked = false){
		if(!locked){
			return _jobqueue.size();
		} else {
			size_t qsize;
			// lock queue
			sem_wait( &_queueLocker );
			qsize = _jobqueue.size();
			// unlock queue
			sem_post( &_queueLocker );
			return qsize;
		}
	}

	bool allSleeping(){
		for( size_t i = 0; i < _cpus; ++i ) if( _threadState[i] > 0 ) return false;
		return true;
	}	

	void *run(){
		size_t myNumber = getNextWorkerNumber();
		sem_t* semaph = &(_sleepSem[myNumber]);
		
		// std::cerr << "worker["<< myNumber << "]: state is " << _workState << std::endl;
	
		// keep thread until workState is -1 (terminate)
		while( _workState != -1 ){
		
			// check whether there is some work, do it
			JobQueue::Job job = getNextJob();
			if( job.function == 0 )
			{
				// nothing to do -> sleep

				_threadState[ myNumber ] = 0;
				sem_wait( semaph );	// wait until waked up again
				_threadState[ myNumber ] = _workState;

			} else {
				// work on job!
				job.function( job.argument );
			}
		
			// check every round, whether to stop or not
			if( isAdressContentZero( &(_workState) ) ){
				// show last number ( 0 ) and sleep
				// std::cerr << "worker["<< myNumber << "]:sleep" << std::endl;
				_threadState[ myNumber ] = 0;
				sem_wait( semaph );
				// wake up and show new number
				// std::cerr << "worker["<< myNumber << "]:wake up with state" << _workState << std::endl;
				_threadState[ myNumber ] = _workState;
			}
		}
		_threadState[ myNumber ] = _workState;		
		return 0;	// pointer!
	}

	static void *thread_func(void *d)
	{
		JobQueue *jqueue = (JobQueue*)d;
		return jqueue->run();
	}

};

#endif
