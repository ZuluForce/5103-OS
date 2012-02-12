#ifndef FCFS_H_INCLUDED
#define FCFS_H_INCLUDED

#include <queue>
#include <assert.h>

#include "scheduler/scheduler.h"

using namespace std;

class cFCFS: public cScheduler {
	private:
		/* Internal Datastructures */
		queue<ProcessInfo*> readyQueue;
		queue<ProcessInfo*> blockedQueue;

		ProcessInfo* runningProc;

	public:
		cFCFS();
		~cFCFS();

		void initProcScheduleInfo(ProcessInfo*);
		void addProcess(ProcessInfo*);
		void setBlocked(ProcessInfo*);
		void unblockProcess(ProcessInfo*);
		void removeProcess(ProcessInfo*);

		ProcessInfo* getNextToRun();

		pidType numProcesses();
};

#endif // FCFS_H_INCLUDED
