#include "scheduler/round_robin.h"

cRoundRobin::cRoundRobin(): blockedID(0) {
	runningProc = NULL;
	clockTicksUsed = 0;

	pthread_mutex_init(&blockedLock, NULL);
	pthread_cond_init(&allBlocked, NULL);

	logStream = NULL;
	procLogger = NULL;

	blockedVector.resize(DEF_BLOCK_VEC_SIZE);
	totalBlocked = 0;

	return;
}

cRoundRobin::~cRoundRobin() {

	return;
}

void cRoundRobin::initProcScheduleInfo(ProcessInfo* proc) {
	assert( proc != NULL );

	roundRobinInfo* newInfoStruct = (roundRobinInfo*) malloc(sizeof(roundRobinInfo));
	proc->scheduleData = newInfoStruct;

	return;
}

void cRoundRobin::addProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == ready );
	readyQueue.push(proc);

	return;
}

void cRoundRobin::setBlocked(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == running );

	pthread_mutex_lock(&blockedLock);
	proc->state = blocked;

	unsigned int newID = blockedID.getID();
	if ( newID >= blockedVector.size() ) {
		/* Resize the vector */
		blockedVector.resize(blockedVector.size() * 2);
	}

	/* Carry the index with the process */
	roundRobinInfo* info = (roundRobinInfo*) proc->scheduleData;
	info->blockedIndex = newID;

	blockedVector[newID] = proc;

	++totalBlocked;
	runningProc = NULL;
	pthread_mutex_unlock(&blockedLock);

	/* Update Process info for Top */
	procLogger->writeProcessInfo(proc);

	return;
}

void cRoundRobin::unblockProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == blocked );
	assert( totalBlocked > 0 );

	pthread_mutex_lock(&blockedLock);
	proc->state = ready;

	roundRobinInfo* info = (roundRobinInfo*) proc->scheduleData;
	assert(info->blockedIndex < blockedVector.size());

	readyQueue.push(blockedVector[info->blockedIndex]);
	blockedID.returnID(info->blockedIndex);

	--totalBlocked;

	pthread_mutex_unlock(&blockedLock);
	pthread_cond_signal(&allBlocked);

	procLogger->writeProcessInfo(proc);

	return;
}

void cRoundRobin::removeProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == running );

	proc->state = terminated;
	free( proc->scheduleData );

	runningProc = NULL;

	/* Remove process from log */

	return;
}

ProcessInfo* cRoundRobin::getNextToRun() {
	/* Find ready process which came first */

	//This makes it non-preemptive
	if ( runningProc != NULL) {
	    if ((clockTicksUsed) > QUANTUM){
	        runningProc->state == ready;
	        readyQueue.push(runningProc);
	    } else{
	        clockTicksUsed++;
            procLogger->writeProcessInfo(runningProc);
            return runningProc;
	    }
	}

	pthread_mutex_lock(&blockedLock);
	if ( readyQueue.size() == 0 ) {
		if ( totalBlocked > 0) {
			while ( readyQueue.size() == 0)
				pthread_cond_wait(&allBlocked, &blockedLock);

		} else {
			/* Nothing is left to run */
			pthread_mutex_unlock(&blockedLock);
			return NULL;
		}
	}

	ProcessInfo* toRun = readyQueue.front();
	readyQueue.pop();

	assert(toRun != NULL);

	runningProc = toRun;
	clockTicksUsed++;
	runningProc->state = running;


	procLogger->writeProcessInfo(runningProc);

	pthread_mutex_unlock(&blockedLock);


	return toRun;
}

pidType cRoundRobin::numProcesses() {
	pidType total = 0;
	total += readyQueue.size();
	total += totalBlocked;

	total += (runningProc == NULL) ? 0 : 1;

	return total;
}

void cRoundRobin::addLogger(FILE* _logStream) {
	assert(logStream == NULL);
	assert(_logStream != NULL);

	logStream = _logStream;
	return;
}

void cRoundRobin::addProcLogger(cProcessLogger* _procLogger) {
	assert(procLogger == NULL);
	assert(_procLogger != NULL);

	procLogger = _procLogger;
	return;
}
