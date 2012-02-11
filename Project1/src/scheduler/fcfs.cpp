#include "scheduler/fcfs.h"

cFCFS::cFCFS() {

	return;
}

cFCFS::~cFCFS() {

	return;
}

void cFCFS::initProcScheduleInfo(ProcessInfo* proc) {
	assert( proc != NULL );

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

	return;
}

void cFCFS::unblockProcess(ProcessInfo* proc) {
	assert( proc != NULL );
	assert( proc->state == blocked );

	return;
}

void cFCFS::removeProcess(ProcessInfo* proc) {
	assert( proc != NULL );

	return;
}

ProcessInfo* cFCFS::getNextToRun() {
	/* Find ready process which came first */
	ProcessInfo* toRun = readyQueue.front();
	readyQueue.pop();

	return toRun;
}

pidType cFCFS::numProcesses() {
	pidType total = 0;
	total += readyQueue.size();
	total += blockedQueue.size();

	return total;
}
