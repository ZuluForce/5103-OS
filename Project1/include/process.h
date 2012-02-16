#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

/** @file */

#include <inttypes.h>

typedef unsigned int pidType;

/** Enumeration for process states
 *
 *	Each values defines a current state and possible
 *	transitions.
 */
enum eProcState {ready, running, blocked, terminated};

/** Structure for containing process state and data
 *
 *	This struture is created in the kernel when a process
 *	is initialized. It contains all process data needed for
 *	execution and for the kernel/scheduler to make desciions
 *	on it.
 */
struct ProcessInfo {
	pidType procFileLine; //Index into process file

    unsigned int parent, pid;
    unsigned int startCPU, totalCPU;

	eProcState state;
	uint16_t PSW;
	int priority;

	/* Saved Sate */
	unsigned int PC;
	int VC;

	/* Program Content */
    char *processText;

	/** Scheduler specific data
	 *
	 *	Check specific scheduler docs for the contents of this
	 *	pointer. Since the process struct remains static,
	 *	this gives the ability for schedulers to store their
	 *	own state without the kernel having to know ahead
	 *	of time.
	 */
    void *scheduleData;

	/* Total memory being used by the process */
    unsigned long memory;
};

/** \var eProcState ready
 *	Process is ready to be run.
 *
 *	Invariant State:\n
 *	\li Kernel has initialized it at some point
 *	\li Process should be preparred to run
 *
 *	Potential Transitions:
 *	\li ::running - Scheduler picks it to run next
 */

/** \var eProcState running
 *	Process is currently running
 *
 *	A running process should implicilty be considered
 *	ready. The kernel may not notify the scheduler to
 *	transition the process to ready before asking for a
 *	a scheduling decision. It is acceptable for the scheduler
 *	to make a process ready without the kernel's consent when
 *	it is being asked for a scheduling decision, assuming it was
 *	previously running.
 *
 *	Invariant State:
 *	\li Process is on the cpu
 *
 *	Potential Transitions:
 *	\li ::ready - Scheduler picks someone else to run
 *	\li ::blocked - Makes blocking system call
 *	\li ::terminated - Causes exception in cpu or finished normally
 */

/** \var eProcState blocked
 *	Process is blocked and cannot run.
 *
 *	Invariant State:
 *	\li Process is blocked (for now it can only block on I/O)
 *
 *	Potential Transitions:
 *	\li ::ready - Kernel notifies scheduler that I/O has finished
 */

/** \var eProcState terminated
 *	Process has been terminated. It will be cleaned up soon.
 *
 *	Invariant State:
 *	\li Process either caused cpu exception or finished
 *	\li Process can no longer run
 *
 *	Potential Transitions:
 *	\li None
 */

#endif // PROCESS_H_INCLUDED
