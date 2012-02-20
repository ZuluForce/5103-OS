#include "scheduler/multi_level.h"

cMultiLevel::cMultiLevel(): blockedID(0) {
	runningProc = NULL;
	currentLevel = 0;
	quantaUsed = 0;

	totalReady = totalBlocked = 0;

	pthread_mutex_init(&blockedLock, NULL);
	pthread_cond_init(&allBlocked, NULL);

	logStream = NULL;
	procLogger = NULL;

	readyQueues.resize(TOP_LEVEL + 1);
	blockedVector.resize(DEF_BLOCK_VEC_SIZE);

	/* Vector to store scheduling quanta for
	 * each level */
	levelQuantas.push_back(L0_QUANTA);
	levelQuantas.push_back(L1_QUANTA);
	levelQuantas.push_back(L2_QUANTA);
	levelQuantas.push_back(L3_QUANTA);
	return;
}

cMultiLevel::~cMultiLevel() {
	readyQueues.clear();
	blockedVector.clear();

	return;
}

void cMultiLevel::initProcScheduleInfo(ProcessInfo* proc) {
	assert( proc != NULL );

	sMultiInfo* newInfoStruct = (sMultiInfo*) malloc(sizeof(sMultiInfo));
	newInfoStruct->level = 0;
	proc->scheduleData = newInfoStruct;

	return;
}

void cMultiLevel::addProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == ready );
	readyQueues.at(0).push(proc);

	++totalReady;

	return;
}

void cMultiLevel::setBlocked(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == running );

	sMultiInfo* schedInfo = (sMultiInfo*) proc->scheduleData;

	pthread_mutex_lock(&blockedLock);
	proc->state = blocked;

	/* Even though it blocked it may have used its whole quanta */
	if ( quantaUsed >= levelQuantas[currentLevel] ) {
		if ( schedInfo->level != 3)
			++(schedInfo->level); //This is the queue it will be resumed in
	} else {
		if ( schedInfo->level != 0 )
			--(schedInfo->level);
	}

	unsigned int newID = blockedID.getID();
	if ( newID >= blockedVector.size() ) {
		/* Resize the vector */
		blockedVector.resize(blockedVector.size() * 2);
	}

	/* Carry the index with the process */
	schedInfo->blockedIndex = newID;

	blockedVector[newID] = proc;

	--totalReady;
	++totalBlocked;

	runningProc = NULL;
	fprintf(logStream, "Process %d has been blocked\n", proc->pid);
	pthread_mutex_unlock(&blockedLock);

	/* Update Process info for Top */
	procLogger->writeProcessInfo(proc);

	return;
}

void cMultiLevel::unblockProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == blocked );
	assert( totalBlocked > 0 );

	sMultiInfo* info = (sMultiInfo*) proc->scheduleData;

	pthread_mutex_lock(&blockedLock);
	proc->state = ready;

	assert(info->blockedIndex < blockedVector.size());

	//readyQueue.push(blockedVector[info->blockedIndex]);
	readyQueues.at(info->level).push(proc);
	blockedID.returnID(info->blockedIndex);

	++totalReady;
	--totalBlocked;
	traceUnblocked.push(proc->pid);

	pthread_mutex_unlock(&blockedLock);
	pthread_cond_signal(&allBlocked);

	procLogger->writeProcessInfo(proc);

	return;
}

void cMultiLevel::removeProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == running );

	proc->state = terminated;
	free( proc->scheduleData );
	procLogger->writeProcessInfo(proc);

	runningProc = NULL;

	return;
}

void cMultiLevel::printUnblocked() {
	pidType tempID = 0;

	while ( traceUnblocked.size() > 0 ) {
		tempID = traceUnblocked.front();
		traceUnblocked.pop();
		fprintf(logStream, "Process %d unblocked\n", tempID);
	}

	//For the output to look nicer
	if ( tempID != 0 )
		fprintf(logStream, "\n");

	return;
}

ProcessInfo* cMultiLevel::getNextToRun() {
	printUnblocked();

	sMultiInfo* info;

	pthread_mutex_lock(&blockedLock);

	if ( runningProc != NULL) {

		/* Continue running the current process if it hasn't used it quanta
		 * or there are no other processes to choose from
		 */
		if ( quantaUsed < levelQuantas[currentLevel] || totalReady == 0) {
			++quantaUsed;
			procLogger->writeProcessInfo(runningProc);
			return runningProc;
		}

		info = (sMultiInfo*) runningProc->scheduleData;

		/* Process has used its quanta so it needs to be moved
		 * down a level if possible. And descheduled. */
		switch( info->level ) {
			case 0:
			case 1:
			case 2:
				++(info->level);
				printf("Process %d moved to queue %d\n", runningProc->pid, info->level);
				readyQueues.at(info->level).push(runningProc);
				break;

			case 3:
				readyQueues.at(3).push(runningProc);
				break;

			default: //Shouldn't get here
				string msg = "Process in invalid queue level: " + info->level;
				throw((string) msg);
				break;
		}

		++totalReady;
		procLogger->writeProcessInfo(runningProc);
		runningProc = NULL;
	}

	if ( totalReady == 0 ) {
		if ( totalBlocked > 0) {
			fprintf(logStream, "Scheduler waiting for a process to unblock\n");
			while ( totalReady == 0)
				pthread_cond_wait(&allBlocked, &blockedLock);

			printUnblocked();

		} else {
			/* Nothing is left to run */
			pthread_mutex_unlock(&blockedLock);
			return NULL;
		}
	}

	ProcessInfo* toRun = NULL;
	vector<queue<ProcessInfo*> >::iterator it;
	int index = 0;

	for ( it = readyQueues.begin(); it != readyQueues.end(); ++it, ++index) {
		if ( (*it).size() > 0 ) {
			toRun = (*it).front();
			(*it).pop();

			info = (sMultiInfo*) toRun->scheduleData;
			info->level = index;
			currentLevel = index;
		}
	}

	assert(toRun != NULL);

	quantaUsed = 0;
	--totalReady;

	runningProc = toRun;
	runningProc->state = running;


	procLogger->writeProcessInfo(runningProc);

	pthread_mutex_unlock(&blockedLock);

	return toRun;
}

pidType cMultiLevel::numProcesses() {
	pidType total = 0;
	total += totalReady;
	total += totalBlocked;

	total += (runningProc == NULL) ? 0 : 1;

	return total;
}

void cMultiLevel::addLogger(FILE* _logStream) {
	assert(logStream == NULL);
	assert(_logStream != NULL);

	logStream = _logStream;
	return;
}

void cMultiLevel::addProcLogger(cProcessLogger* _procLogger) {
	assert(procLogger == NULL);
	assert(_procLogger != NULL);

	procLogger = _procLogger;
	return;
}
