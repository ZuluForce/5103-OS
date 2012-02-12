#ifndef ROUND_ROBIN_H_INCLUDED
#define ROUND_ROBIN_H_INCLUDED

#include "scheduler/scheduler.h"

class cRoundRobin: pulbic cScheduler {
	private:

	public:
		cRoundRobin();
		~cRoundRobin();

		void initProcScheduleInfo(ProcessInfo*);
		void addProcess(ProcessInfo*);
		void setBlocked(ProcessInfo*);
		void unblockedProcess(ProcessInfo*);
		void removeProcess(ProcessInfo*);

		ProcessInfo* getNextToRun();
		pidType numProcesses();

}


#endif // ROUND_ROBIN_H_INCLUDED
