/*
 * lottery.h
 *
 *  Created on: Feb 14, 2012
 *      Author: Dylan
 */

#ifndef LOTTERY_H_INCLUDED
#define LOTTERY_H_INCLUDED

#include <cstdlib>
#include <queue>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#include "scheduler/scheduler.h"
#include "utility/id.h"

#define DEF_BLOCK_VEC_SIZE 4

using namespace std;

class cLottery: public cScheduler {
private:
	vector<ProcessInfo*> readyVector;
	vector<ProcessInfo*> blockedVector;

	queue<pidType> traceUnblocked;

	int totalReady;
	int totalBlocked;
	int totalTickets;

	ProcessInfo* runningProc;

	pthread_mutex_t blockedLock;
	pthread_cond_t allBlocked;

	cIDManager blockedID;
	cIDManager readyID;

	FILE* logStream;
	cProcessLogger* procLogger;

public:
	cLottery();
	~cLottery();

	void initProcScheduleInfo(ProcessInfo*);
	void addProcess(ProcessInfo*);
	void setBlocked(ProcessInfo*);
	void unblockProcess(ProcessInfo*);
	void removeProcess(ProcessInfo*);

	ProcessInfo* getNextToRun();

	pidType numProcesses();

	void addLogger(FILE* _logStream);
	void addProcLogger(cProcessLogger* _procLogger);

	void printUnblocked();

};

struct lotteryInfo {
	unsigned int readyIndex;
	unsigned int blockedIndex;
};

#endif /* LOTTERY_H_ */
