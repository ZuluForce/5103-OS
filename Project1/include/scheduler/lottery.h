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

#include "scheduler/scheduler.h"
#include "utility/id.h"

using namespace std;

class cLottery: public cScheduler {
private:
	queue<ProcessInfo*> readyQueue;
	vector<ProcessInfo*> blockedVector;

	ProcessInfo* runningProc;

	pthread_mutex_t blockedLock;
	pthread_cond_t allBlocked;

	cIDManager blockedID;

public:
	cLottery();
	~cLottery();

	void initProcSceduleInfo(ProcessInfo*);
	void addProcess(ProcessInfo*);
	void setBlocked(ProcessInfo*);
	void unblockProcess(ProcessInfo*);
	void removeProcess(ProcessInfo*);

	ProcessInfo* getNextToRun();

	pidType numProcesses();

};

struct lotteryInfo {
	unsigned int blockedIndex;
};

#endif /* LOTTERY_H_ */
