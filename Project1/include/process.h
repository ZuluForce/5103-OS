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
	/* I was thinking we could save the name so the output of
	 * Top would look nice later.
	 */
	//string name;

    unsigned int parent, pid;
    unsigned int startCPU, totalCPU;

	eProcState state; /**< #eProcState */
	uint16_t PSW;
	int priority;

	/* Saved Sate */
	unsigned int PC;
	int VC;

	/* Program Content */
    char *processText;

    /* Pointer to Scheduler information:
     * If the scheduler needs information that is not provided in
     * the standard process struct, it can be allocated elsewhere
	 * and assigned to this pointer. The scheduling algorithm should
	 * cast it to the correct type.
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
 *	Invariant State:
 *	\li Process is on the cpu
 *
 *	Potential Transitions:
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
