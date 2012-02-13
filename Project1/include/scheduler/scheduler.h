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

/** @fn virtual void cScheduler::initProcScheduleInfo(ProcessInfo*) = 0
 *	Initialize scheduler specific information within the ProcessInfo struct
 *
 *	This method is called after the kernel has initialized all process data
 *	but before it is marked ::ready. This gives the scheduler an oportunity to
 *	initialize any scheduler specific data and assign it to the ProcessInfo::scheduleData
 *	member.
 *
 *	Implementation Requirements:
 *	\li No required actions.
 */

/** @fn virtual void cScheduler::addProcess(ProcessInfo*) = 0
 *	Transfer control of process state and scheduling.
 *
 *	After this is called, the kernel core no longer keeps track
 *	of the given process. Once the process is created and
 *	deemed ::ready by the kernel it is handed off here. The
 *	scheduler is then in charge of state transitions when the
 *	kernel gives it appropriate notifications.
 *
 *	Implementation Requirements:
 *	\li Store the process in some location. It should be recognized
 *	as ready given its location but the datastructures and organization
 *	are implementation specific.
 *
 *	@param ProcessInfo* Process to add under scheduler's control
 */

 /** @fn virtual void cScheduler::setBlocked(ProcessInfo*) = 0
  *	Set a process into a ::blocked state.
  *
  *	The kernel will call the scheduler with this function when the
  *	process has done an operation which causes it to block (I\O).
  *
  *	Implementation Requirements:
  *	\li Process must be marked ::blocked and scheduler state should
  *	be changed accordingly.
  *	\li After this call a process should not be considered for a scheduling decision
  *
  *	@warning Must be thread safe. Signal handler/s may block during schedule decision.
  */

/** @fn virtual void cScheduler::unblockProcess(ProcessInfo*) = 0
 *	Unblock a process and make it ready.
 *
 *	When a process has completed a blocking call the kernel
 *	will notify the scheduler that it should be unblocked.
 *	This operation should be very fast since it will likely be
 *	called from a signal handler.
 *
 *	Implementation Requirements:
 *	\li The process must be unblocked and marked ::ready. It must be
 *	available for scheduling with the next call to ::getNextToRun
 *
 *	@param ProcessInfo* Process to unblock
 *
 *	@warning Must be thread safe. Signal handler/s may unblock during schedule decision.
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
 *	Implementation Requirements:
 *	\li Scheduler should deallocate any resources it assigned to the
 *	process within the ProcessInfo::scheduleData member.
 *	\li The Scheduler should remove any pointers to the give process
 *	to avoid dereferncing a dead pointer.
 *	\li Implementations must not deallocate any memory except that mentioned
 *	above. This is handled by the kernel.
 *	\li Implementations should mark the process as ::terminated.
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
 *
 *	Implementation Requirements:
 *	\li Block if there are > 0 processes but none can run.
 *	\li A process listed as ::running within the scheduler should be
 *	treated as ready when this method is called.
 *	\li A process returned as the 'next to run' should be marked as
 *	being in a ::running state before returning.
 *	\li If there are no more remaining processes this should return NULL.
 *
 *	@return ::ProcessInfo* Ready process to run next. May be the same as the currenlty running one.
 *
 *	@warning Must be thread safe with block and unblock methods.
 */

/** @fn virtual pidType cScheduler::numProcesses() = 0
 *	How many processes are in the scheduler
 *
 *	This returns how many processes, both running and blocked, are being
 *	handled by the scheduler.
 *
 *	Implementation Requirements:
 *	\li Return how many processes, running and blocked, are in the scheduler.
 *
 *	\return pidType
 */
#endif // SCHEDULER_H_INCLUDED
