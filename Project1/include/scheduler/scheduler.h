#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "process.h"

/* Abstract Interface for Schedulers */
class cScheduler {
	public:
		/* Notification to scheduler so it can update its datastructures */
		virtual void initProcScheduleInfo(ProcessInfo*) = 0;
		virtual void addProcess(ProcessInfo*) = 0;
		virtual void setBlocked(ProcessInfo*) = 0;
		virtual void unblockProcess(ProcessInfo*) = 0;
		virtual void removeProcess(ProcessInfo*) = 0;

		/* Get next process to run */
		virtual ProcessInfo* getNextToRun() = 0;

		/* Meta Information */
		virtual pidType numProcesses() = 0;
};

#endif // SCHEDULER_H_INCLUDED
