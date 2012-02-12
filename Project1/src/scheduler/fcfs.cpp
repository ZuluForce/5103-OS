#include "scheduler/fcfs.h"

cFCFS::cFCFS(): blockedID(0) {
	runningProc = NULL;

	pthread_mutex_init(&blockedLock, NULL);
	pthread_cond_init(&allBlocked, NULL);


	return;
}

cFCFS::~cFCFS() {

	return;
}

void cFCFS::initProcScheduleInfo(ProcessInfo* proc) {
	assert( proc != NULL );

	fcfsInfo* newInfoStruct = (fcfsInfo*) malloc(sizeof(fcfsInfo));
	proc->scheduleData = newInfoStruct;

	return;
}

void cFCFS::addProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == ready );
	readyQueue.push(proc);

	return;
}

void cFCFS::setBlocked(ProcessInfo* proc) {
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
	fcfsInfo* info = (fcfsInfo*) proc->scheduleData;
	info->blockedIndex = newID;

	blockedVector[newID] = proc;

	pthread_mutex_unlock(&blockedLock);

	return;
}

void cFCFS::unblockProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == blocked );

	pthread_mutex_lock(&blockedLock);
	proc->state = ready;

	fcfsInfo* info = (fcfsInfo*) proc->scheduleData;
	assert(info->blockedIndex < blockedVector.size());

	readyQueue.push(blockedVector[info->blockedIndex]);
	blockedID.returnID(info->blockedIndex);

	pthread_mutex_unlock(&blockedLock);
	pthread_cond_signal(&allBlocked);

	return;
}

void cFCFS::removeProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == running );

	proc->state = terminated;
	free( proc->scheduleData );

	runningProc = NULL;

	return;
}

ProcessInfo* cFCFS::getNextToRun() {
	/* Find ready process which came first */
	if ( runningProc != NULL)
		readyQueue.push(runningProc);

	pthread_mutex_lock(&blockedLock);
	if ( readyQueue.size() == 0 ) {
		if ( blockedID.reservedIDs() > 0) { //Don't check vector size
			while ( readyQueue.size() && blockedID.reservedIDs() > 0)
				pthread_cond_wait(&allBlocked, &blockedLock);

		} else {
			/* Nothing is left to run */
			pthread_mutex_unlock(&blockedLock);
			return NULL;
		}
	}

	ProcessInfo* toRun = readyQueue.front();
	readyQueue.pop();

	runningProc = toRun;
	runningProc->state = running;

	pthread_mutex_unlock(&blockedLock);

	return toRun;
}

pidType cFCFS::numProcesses() {
	pidType total = 0;
	total += readyQueue.size();
	total += blockedVector.size();

	total += (runningProc == NULL) ? 0 : 1;

	return total;
}
