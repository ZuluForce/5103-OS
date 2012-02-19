#ifndef FCFS_H_INCLUDED
#define FCFS_H_INCLUDED

/** @file */

#include <assert.h>

#include "scheduler/scheduler.h"

#define DEF_BLOCK_VEC_SIZE 4

using namespace std;

/** First-Come-First-Serve Scheduler */
class cFCFS: public cScheduler {
	private:
		/* Internal Datastructures */
		queue<ProcessInfo*> readyQueue;
		vector<ProcessInfo*> blockedVector;

		/* This is just for remembering what processes
		 * have been unblocked so the next call to
		 * getNextToRun can print them to the trace file.
		 * This is because getNextToRun is called
		 * synchronously with other trace output.
		 */
		queue<pidType> traceUnblocked;

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

		void printUnblocked();
};

/** Struct containing process info specific for FCFS scheduling */
struct fcfsInfo {
	unsigned int blockedIndex;
};

#endif // FCFS_H_INCLUDED
