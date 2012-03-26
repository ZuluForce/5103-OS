#include "round_robin.h"

extern INIReader* settings;
extern FILE* logStream;

cRoundRobin::cRoundRobin(): blockedID(0) {

	runningProc = NULL;
	clockTicksUsed = 0;

	blockedVector.resize(4);
	totalBlocked = 0;

	return;

}

cRoundRobin::~cRoundRobin() {

	return;
}

void cRoundRobin::initProcScheduleInfo(sProc* proc) {
	assert( proc != NULL );

	roundRobinInfo* newInfoStruct = (roundRobinInfo*) malloc(sizeof(roundRobinInfo));
	proc->scheduleData = newInfoStruct;

	return;
}

void cRoundRobin::addProcesses(vector<sProc*>& procs) {
	vector<sProc*>::iterator it;

	for ( it = procs.begin(); it != procs.end(); ++it ) {
		initProcScheduleInfo(*it);

		readyQueue.push(*it);
	}

	return;
}

void cRoundRobin::setBlocked(sProc* proc) {
	assert( proc != NULL );
	assert( proc == runningProc );

	fprintf(logStream, "Scheduler: Blocking process %d\n", proc->pid);

	unsigned int newID = blockedID.getID();
	if ( newID >= blockedVector.size() ) {
		/* Resize the vector */
		blockedVector.resize(blockedVector.size() * 2);
	}

	/* Carry the index with the process */
	roundRobinInfo* info = (roundRobinInfo*) proc->scheduleData;
	info->blockedIndex = newID;

	blockedVector.at(newID) = proc;

	++totalBlocked;
	runningProc = NULL;

	return;
}

void cRoundRobin::unblockProcess(sProc* proc) {
	assert( proc != NULL );
	assert( totalBlocked > 0 );

	fprintf(logStream, "Scheduler: Unblocking process %d\n", proc->pid);

	roundRobinInfo* info = (roundRobinInfo*) proc->scheduleData;
	assert(info->blockedIndex < blockedVector.size());

	readyQueue.push(blockedVector[info->blockedIndex]);
	blockedID.returnID(info->blockedIndex);

	--totalBlocked;

	return;
}

void cRoundRobin::removeProcess(sProc* proc) {
	assert( proc != NULL );
	assert( proc == runningProc );

	free( proc->scheduleData );

	runningProc = NULL;
	clockTicksUsed = 0;

	return;
}

sProc* cRoundRobin::getNextToRun() {

	if ( runningProc != NULL) {
		readyQueue.push(runningProc);

		runningProc = NULL;
	}

	if ( readyQueue.size() == 0 ) {
		return NULL;
	}

	sProc* toRun = readyQueue.front();
	readyQueue.pop();

	assert(toRun != NULL);

	runningProc = toRun;

	return toRun;
}

int cRoundRobin::numProcesses() {
	int total = 0;
	total += readyQueue.size();
	total += totalBlocked;

	total += (runningProc == NULL) ? 0 : 1;

	return total;
}
