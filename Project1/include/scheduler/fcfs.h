#ifndef FCFS_H_INCLUDED
#define FCFS_H_INCLUDED

#include <cstdlib>
#include <queue>
#include <assert.h>
#include <pthread.h>

#include "scheduler/scheduler.h"
#include "utility/id.h"

#define DEF_BLOCK_VEC_SIZE 4

using namespace std;

class cFCFS: public cScheduler {
	private:
		/* Internal Datastructures */
		queue<ProcessInfo*> readyQueue;
		vector<ProcessInfo*> blockedVector;

		int totalBlocked;

		ProcessInfo* runningProc;

		/* When all processes are blocked the scheduler waits
		 * on this condition variable
		 */
		pthread_mutex_t 	blockedLock;
		pthread_cond_t 		allBlocked;

		cIDManager blockedID;

		/* Logging */
		FILE* logStream;
		cProcessLogger* procLogger;

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

		void addLogger(FILE* _logStream);
		void addProcLogger(cProcessLogger* _procLogger);
};

struct fcfsInfo {
	unsigned int blockedIndex;
};

#endif // FCFS_H_INCLUDED
