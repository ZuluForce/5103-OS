#include "scheduler/lottery.h"


cLottery::cLottery() : blockedID(0), readyID(0) {
	runningProc = NULL;

	pthread_mutex_init(&blockedLock, NULL);
	pthread_cond_init(&allBlocked, NULL);

	logStream = NULL;
	procLogger = NULL;

	blockedVector.resize(DEF_BLOCK_VEC_SIZE);
	readyVector.resize(DEF_BLOCK_VEC_SIZE);
	totalBlocked = 0;
	totalReady = 0;

	totalTickets = 0;

	srand(time(NULL));


	return;
}

cLottery::~cLottery() {
	return;
}

void cLottery::initProcScheduleInfo(ProcessInfo* proc) {
	assert(proc != NULL);

	lotteryInfo* newInfoStruct = (lotteryInfo*) malloc(sizeof(lotteryInfo));
	proc->scheduleData = newInfoStruct;

	return;

}

void cLottery::addProcess(ProcessInfo* proc) {
	assert(proc != NULL);
	assert(proc->state == ready);

	unsigned int newID = readyID.getID();
	if(newID >= readyVector.size()) {
		readyVector.resize(readyVector.size() * 2);
	}

	/* Carry the index with the process */
	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	info->readyIndex = newID;

	readyVector.at(newID) = proc;

	++totalReady;

	totalTickets += proc->priority;


	return;
}

void cLottery::setBlocked(ProcessInfo* proc) {
	assert(proc != NULL);
	assert(proc->state == running);

	pthread_mutex_lock(&blockedLock);
	proc->state = blocked;

	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	readyVector.at(info->readyIndex) = NULL;
	totalTickets -= proc->priority;

	readyID.returnID(info->readyIndex);


	unsigned int newID = blockedID.getID();
	if (newID >= blockedVector.size()) {
		/* Resize the vector */
		blockedVector.resize(blockedVector.size() * 2);
	}

	/* Carry the index with the process */
	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	info->blockedIndex = newID;

	blockedVector[newID] = proc;

	--totalReady;
	++totalBlocked;
	runningProc = NULL;
	fprintf(logStream, "Process %d has been blocked\n", proc->pid);
	pthread_mutex_unlock(&blockedLock);

	/* Update Process into for Top */
	procLogger->writeProcessInfo(proc);

	return;
}

void cLottery::unblockProcess(ProcessInfo* proc) {
	assert(proc != NULL);
	assert(proc->state == blocked);
	assert(totalBlocked > 0);

	pthread_mutex_lock(&blockedLock);

	proc->state = ready;

	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	assert(info->blockedIndex < blockedVector.size());

	//put back into readyVector
	unsigned int lowID = readyID.getLowID();
	readyVector.at(lowID) = proc;

	blockedID.returnID(info->blockedIndex);

	++totalReady;
	--totalBlocked;
	traceUnblocked.push(proc->pid);

	pthread_mutex_unlock(&blockedLock);
	pthread_cond_signal(&allBlocked);

	procLogger->writeProcessInfo(proc);

	return;

}

void cLottery::removeProcess(ProcessInfo* proc) {
	assert(proc != NULL);
	assert(proc->state == running);

	pthread_mutex_lock(&blockedLock);

	proc->state = terminated;
	free(proc->scheduleData);

	runningProc = NULL;

	/* Remove process from log */

	pthread_mutex_unlock(&blockedLock);

	return;


}

void cLottery::printUnblocked() {
	pidType tempID;

	while (traceUnblocked.size() > 0) {
		tempID = traceUnblocked.front();
		traceUnblocked.pop();
		fprintf(logStream, "Process %d unblocked\n", tempID);
	}

	return;


}

ProcessInfo* cLottery::getNextToRun() {
	/* Find ready process which came first */
	printUnblocked();

	//This makes it non-preemptive
	if(runningProc != NULL) {
		procLogger->writeProcessInfo(runningProc);
		return runningProc;
	}

	pthread_mutex_lock(&blockedLock);

	if(totalReady == 0){
		if(totalBlocked > 0) {
			while(totalReady == 0)
				pthread_cond_wait(&allBlocked, &blockedLock);

			printUnblocked();
		}
		else {
			/* Nothing is left to run */
			pthread_mutex_unlock(&blockedLock);
		}
	}

	//Set bounds for tickets and choose a random ticket
	int low = 0, high = totalTickets;
	int ticket = rand() % (high - low + 1) + low;
	assert(ticket >= 0 && ticket <= totalTickets);

	//Initialize bounds to determine which process has ticket
	int lowerBound = 0;
	int upperBound;
	ProcessInfo* toRun;

	//Iterate through readyVector
	vector<ProcessInfo*>::iterator iter;
	for(iter = readyVector.begin(); iter < readyVector.end(); ++iter) {
		if(*iter != NULL) {

			upperBound += (*iter)->priority;
			assert(lowerBound != upperBound);

			if(ticket >= lowerBound && ticket <= upperBound) {
				toRun = *iter;
				lotteryInfo* info = (lotteryInfo*) toRun->scheduleData;

				//Not sure if I can do this with the iterator object
				readyVector.at(info->readyIndex) = NULL;

				readyID.returnID(info->readyIndex);
				--totalReady;
				break;
			}
			lowerBound = upperBound;
		}
	}

	assert(toRun != NULL);
	runningProc = toRun;
	runningProc->state = running;
	procLogger->writeProcessInfo(runningProc);

	pthread_mutex_unlock(&blockedLock);

	return toRun;

}

pidType cLottery::numProcesses() {
	pidType total = 0;

	total += totalReady;
	total += totalBlocked;

	total += (runningProc == NULL) ? 0 : 1;

	return total;
}

void cLottery::addLogger(FILE* _logStream) {
	assert(logStream == NULL);
	assert(_logStream != NULL);

	logStream = _logStream;
	return;
}

void cLottery::addProcLogger(cProcessLogger* _procLogger) {
	assert(procLogger == NULL);
	assert(_procLogger != NULL);

	procLogger = _procLogger;
	return;
}





















