#ifndef MULTI_LEVEL_H_INCLUDED
#define MULTI_LEVEL_H_INCLUDED

#include "scheduler/scheduler.h"

#define L0_QUANTA 1
#define L1_QUANTA 2
#define L2_QUANTA 4
#define L3_QUANTA 8
#define TOP_LEVEL 3

#define DEF_BLOCK_VEC_SIZE 4

using namespace std;

class cMultiLevel: public cScheduler {
	private:
		/* Internal Datastructures */
		int currentLevel;
		int quantaUsed;

		int totalReady;
		int totalBlocked;
		vector<queue<ProcessInfo*> > readyQueues;
		vector<int> levelQuantas;

		vector<ProcessInfo*> blockedVector;

		/* This is just for remembering what processes
		 * have been unblocked so the next call to
		 * getNextToRun can print them to the trace file.
		 * This is because getNextToRun is called
		 * synchronously with other trace output.
		 */
		queue<pidType> traceUnblocked;


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
		cMultiLevel();
		~cMultiLevel();

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

struct sMultiInfo {
	int blockedIndex;
	int level;
};

#endif // MULTI_LEVEL_H_INCLUDED
