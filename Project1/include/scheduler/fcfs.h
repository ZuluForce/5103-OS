#ifndef FCFS_H_INCLUDED
#define FCFS_H_INCLUDED

#include <cstdlib>
#include <queue>
#include <assert.h>
#include <pthread.h>

#include "scheduler/scheduler.h"
#include "utility/id.h"

using namespace std;

class cFCFS: public cScheduler {
	private:
		/* Internal Datastructures */
		queue<ProcessInfo*> readyQueue;
		vector<ProcessInfo*> blockedVector;

		ProcessInfo* runningProc;

		/* When all processes are blocked the scheduler waits
		 * on this condition variable
		 */
		pthread_mutex_t 	blockedLock;
		pthread_cond_t 		allBlocked;

		cIDManager blockedID;

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

struct fcfsInfo {
	unsigned int blockedIndex;
};

#endif // FCFS_H_INCLUDED
