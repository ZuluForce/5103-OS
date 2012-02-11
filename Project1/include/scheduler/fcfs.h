#ifndef FCFS_H_INCLUDED
#define FCFS_H_INCLUDED

#include "process.h"
#include "scheduler/scheduler.h"

class cFCFS: public cScheduler {
	private:
		/* Internal Datastructures */

	public:
		cFCFS();
		~cFCFS();

		void initProcScheduleInfo(ProcessInfo*);
		void setBlocked(ProcessInfo*);
		void setRunnable(ProcessInfo*);
		void removeProcess(ProcessInfo*);

		virtual unsigned int getNextToRun();
};

#endif // FCFS_H_INCLUDED
