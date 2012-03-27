#ifndef ROUND_ROBIN_H_INCLUDED
#define ROUND_ROBIN_H_INCLUDED

/** @file */

#include <assert.h>

#include "iniReader.h"
#include "data_structs.h"
#include "utility/id.h"
#include "utility/logger.h"

using namespace std;

/** Round Robin Scheduler
 *
 *	Nothing too much different here from project1 and since it
 *	isn't a core piece to this project it won't be documented in
 *	detail.
 */
class cRoundRobin {
	private:
        /* Internal Datastructures */
        queue<sProc*> readyQueue;
        vector<sProc*> blockedVector;

        int totalBlocked;

        int clockTicksUsed;

        sProc* runningProc;

        cIDManager blockedID;

	public:
		cRoundRobin();
		~cRoundRobin();

		void initProcScheduleInfo(sProc* proc);

		void addProcesses(vector<sProc*>& procs);
		void setBlocked(sProc*);
		void unblockProcess(sProc*);
		void removeProcess(sProc*);

		sProc* getNextToRun();
		int numProcesses();

};

/** Struct containing process info specific for Round-Robin scheduling */
struct roundRobinInfo {
	unsigned int blockedIndex; /**< Index position in blocked vector */
};

#endif // ROUND_ROBIN_H_INCLUDED
