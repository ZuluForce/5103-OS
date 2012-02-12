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

/** @fn virtual void cScheduler::addProcess(ProcessInfo*) = 0
 *	Transfer control of process state and scheduling.
 *
 *	After this is called, the kernel core no longer keeps track
 *	of the given process. Once the process is created and
 *	deemed ready by the kernel it is handed off here. The
 *	scheduler is then in charge of state transitions when the
 *	kernel gives it appropriate notifications.
 *
 *	@param ProcessInfo* Process to add under scheduler's control
 */

/** @fn virtual void cScheduler::unblockProcess(ProcessInfo*) = 0
 *	Unblock a process and make it ready.
 *
 *	When a process has completed a blocking call the kernel
 *	will notify the scheduler that it should be unblocked.
 *	This operation should be very fast since it will likely be
 *	called from a signal handler.
 *
 *	@param ProcessInfo* Process to unblock
 */

/** @fn virtual void cScheduler::removeProcess(ProcessInfo*) = 0
 *	Remove a process from the control of the scheduler.
 *
 *	When a process terminates, either through normal means or an
 *	exception, the kernel will call this function to release a process
 *	from the scheduler's control. The scheduler should clean up any
 *	internal state for the process. Deallocation of process resources
 *	is left to the kernel.
 *
 *	@param ProcessInfo* Process to remove from scheduler
 */

/** @fn virtual ProcessInfo* cScheduler::getNextToRun() = 0
 *	Query the scheduler for next process to run.
 *
 *	After this function is called, it should be assumed by any
 *	scheduler implementation that the kernel will run the given
 *	process (unless otherwise notified). The currently running
 *	process should implicitly be considered for running next (again).
 *
 *	If there are processes left but all are blocked. This function should
 *	block until it receives a signal that a process is unblocked.
 *
 *	@return ProcessInfo* Ready process to run next. May be the same as the currenlty running one.
 */

#endif // SCHEDULER_H_INCLUDED
