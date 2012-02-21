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
		if ( schedInfo->level != 3) {
			++(schedInfo->level); //This is the queue it will be resumed in
			fprintf(logStream, "Process %d will be moved down to level %d after unblock\n",
					proc->pid, schedInfo->level);
		}
	} else {
		if ( schedInfo->level != 0 ) {
			--(schedInfo->level);
			fprintf(logStream, "Process %d will be moved up to level %d after unblock\n",
					proc->pid, schedInfo->level);
		}
	}

	unsigned int newID = blockedID.getID();
	if ( newID >= blockedVector.size() ) {
		/* Resize the vector */
		blockedVector.resize(blockedVector.size() * 2);
	}

	/* Carry the index with the process */
	schedInfo->blockedIndex = newID;

	blockedVector[newID] = proc;

	++totalBlocked;
	runningProc = NULL;

	quantaUsed = 0;
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

void cMultiLevel::printLevels() {
	int level = 0;
	vector<queue<ProcessInfo*> >::iterator it;
	//queue<ProcessInfo*>::iterator itq;

	fprintf(logStream, "------Multi-Level Queue Info------\n");
	printf("------Multi-Level Queue Info------\n");
	fprintf(logStream, "---------Blocked not shown--------\n");
	printf("---runningProc not included---\n");

	for ( it = readyQueues.begin(); it < readyQueues.end(); ++it, ++level) {
		fprintf(logStream, "Level %d: %d elements\n", level, (int) (*it).size());
		printf("Level %d: %d elements\n", level, (int) (*it).size());

		//Regular queue doesn't have iterators
		//for ( itq = (*it).begin(); itq < (*itq).end(); ++itq) {
		//	printf("%d -->", (*itq)->pid);
		//}
	}

	fprintf(logStream, "------------ End Info ------------\n");
	printf("------------ End Info ------------\n");

	return;
}

ProcessInfo* cMultiLevel::getNextToRun() {
	printUnblocked();
	printf("currentLevel = %d  quantaUsed = %d  totalReady = %d  levelQuanta = %d\n",
			currentLevel, quantaUsed, totalReady, levelQuantas[currentLevel]);

	fprintf(logStream, "currentLevel = %d  quantaUsed = %d  totalReady = %d  levelQuanta = %d\n",
			currentLevel, quantaUsed, totalReady, levelQuantas[currentLevel]);

	sMultiInfo* info;

	pthread_mutex_lock(&blockedLock);

	if ( runningProc != NULL) {

		/* Continue running the current process if it hasn't used it quanta
		 * or there are no other processes to choose from
		 */
		if ( quantaUsed < levelQuantas[currentLevel]) {
			++quantaUsed;
			procLogger->writeProcessInfo(runningProc);

			fprintf(logStream, "Scheduling same process\n\n");
			printf("Scheduling same process\n\n");
			pthread_mutex_unlock(&blockedLock);
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
				fprintf(logStream, "Process %d moved to queue %d\n", runningProc->pid, info->level);
				printf("Process %d moved to queue %d\n", runningProc->pid, info->level);
				readyQueues.at(info->level).push(runningProc);
				break;

			case 3:
				fprintf(logStream, "Process %d put back in level 3 queue\n", runningProc->pid);
				printf("Process %d put back in level 3 queue\n", runningProc->pid);
				readyQueues.at(3).push(runningProc);
				break;

			default: //Shouldn't get here
				string msg = "Process in invalid queue level: " + info->level;
				throw((string) msg);
				break;
		}

		quantaUsed = 0;
		runningProc->state = ready;
		++totalReady;
		procLogger->writeProcessInfo(runningProc);
		runningProc = NULL;
	}

	printLevels();

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

			break;
		}
	}

	assert(toRun != NULL);

	quantaUsed = 1;
	--totalReady;

	runningProc = toRun;
	runningProc->state = running;


	procLogger->writeProcessInfo(runningProc);

	fprintf(logStream, "\n\n"); //A lot is being printed so some extra whitespace doesn't hurt
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
