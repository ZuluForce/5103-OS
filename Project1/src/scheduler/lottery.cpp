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

	pthread_mutex_lock(&blockedLock);
	unsigned int newID = readyID.getLowID();
	if(newID >= readyVector.size()) {
		readyVector.resize(readyVector.size() * 2);
	}

	/* Carry the index with the process */
	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	info->readyIndex = newID;

	readyVector.at(newID) = proc;

	++totalReady;

	totalTickets += proc->priority;
	pthread_mutex_unlock(&blockedLock);


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

	info->blockedIndex = newID;

	blockedVector.at(newID) = proc;

	//--totalReady; If it was running then it was already removed from totalReady
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
	totalTickets += proc->priority; //Put tickets into pool

	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	assert(info->blockedIndex < blockedVector.size());

	//put back into readyVector
	unsigned int lowID = readyID.getLowID();
	if ( lowID >= readyVector.size() )
		readyVector.resize(readyVector.size() * 2);

	readyVector.at(lowID) = proc;
	info->readyIndex = lowID;

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

	lotteryInfo* info = (lotteryInfo*) proc->scheduleData;
	printf("Removing process at ready index: %d\n", info->readyIndex);
	readyVector.at(info->readyIndex) = NULL;

	readyID.returnID(info->readyIndex);

	totalTickets -= proc->priority;

	proc->state = terminated;
	free(proc->scheduleData);

	runningProc = NULL;

	pthread_mutex_unlock(&blockedLock);

	procLogger->writeProcessInfo(proc);
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
	printUnblocked();

	lotteryInfo* info;

	pthread_mutex_lock(&blockedLock);

	if(runningProc != NULL) {
		runningProc->state = ready;
		procLogger->writeProcessInfo(runningProc);

		runningProc = NULL;
		++totalReady;
	}

	if(totalReady == 0){
		if(totalBlocked > 0) {
			while(totalReady == 0)
				pthread_cond_wait(&allBlocked, &blockedLock);

			printUnblocked();
		}
		else {
			/* Nothing is left to run */
			pthread_mutex_unlock(&blockedLock);
			return NULL;
		}
	}

	vector<ProcessInfo*>::iterator iter;
	fprintf(logStream, "Ticket Info (pid:# tickets)\n\t");
	for ( iter = readyVector.begin(); iter < readyVector.end(); ++iter ) {
		if (*iter != NULL) {
			fprintf(logStream, "%d: %d tickets  ", (*iter)->pid, (*iter)->priority);
		}
	}
	fprintf(logStream, "\n");
	printf("\n");

	//Set bounds for tickets and choose a random ticket
	int low = 0, high = totalTickets;
	int ticket = rand() % (high - low + 1) + low;
	assert(ticket >= low && ticket <= totalTickets);

	//Initialize bounds to determine which process has ticket
	int lowerBound = low;
	int upperBound = low;
	ProcessInfo* toRun;

	//Iterate through readyVector
	for(iter = readyVector.begin(); iter < readyVector.end(); ++iter) {
		if(*iter != NULL) {

			upperBound += (*iter)->priority;
			assert(lowerBound != upperBound);

			if(/*ticket >= lowerBound && */ ticket <= upperBound) {
				toRun = *iter;

				--totalReady;

				fprintf(logStream, "\t!!--Lottery Ticket: %d  Total Tickets: %d  Winner: pid %d--!!\n",
							ticket, totalTickets, toRun->pid);
				break;
			}

		}
	}

	assert(toRun != NULL);
	assert(toRun->state == ready);

	runningProc = toRun;
	runningProc->state = running;
	procLogger->writeProcessInfo(runningProc);

	fprintf(logStream, "\n");
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





















