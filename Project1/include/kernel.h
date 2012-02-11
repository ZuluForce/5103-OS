#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

/** @file */

#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cpu.h"
#include "devices/char_device.h"
#include "devices/block_device.h"
#include "devices/clock_device.h"
#include "utility/id.h"

#define DEFAULT_TIMER 1000 //Not sure on this one, need to read Specs
#define DEFAULT_PRIORITY 0

/* ============ Modify this section to change scheduler used ============== */
//Also have to change dependency in Makefile
#include "scheduler/fcfs.h"
//#define SCHEDULER_TYPE cFCFS
typedef cFCFS schedulerType;
/* ======================================================================== */

#ifndef PROCESS_H_INCLUDED
__attribute__((error("process.h not included by scheduler")));
#endif

using namespace std;

static const char initProcessName[] = "main.trace";

class cKernel {
	private:
        cCPU CPUref;
		int clockTick;

		//Devices:
		BlockDevice bDevice;
		CharDevice cDevice;
		ClockDevice clockInterrupt;

		ProcessInfo *runningProc; /**< #ProcessInfo */

		cIDManager idGenerator;
		/* Need to formalize process storage datastructures
		 * to write this one.
		 */
		//void swapProcess(...);

		/* Process Scheduler */
		schedulerType scheduler;

		/** Swap a process on the cpu
		 *
		 *	Takes the process in its parameter and swaps it with the
		 *	one currently running in the cpu.
		 */
		void swapProcesses(ProcessInfo*);

	public:
		/** Default cKernel constructor
		 *
		 *	The default constructor initializes all internal datastructures
		 *	and loads the initial program (default: 'main.trace') but does
		 *	not run it.
		 */
		cKernel();
		~cKernel();

		/** Start the 'OS' Kernel
		 *
		 *	Starts the main kernel loop. The initial program is loaded and
		 * execution follows from there.
		 */
		void boot(); //Starts the kernel loop


		/** Initialize a Process
		 *
		 *	Initializes a process by loading program file contents, setting default
		 *	process values and adding it in a ready state to the scheduler.
		 */
        void initProcess(const char *filename, pidType parent, int priority = DEFAULT_PRIORITY);

        /** Cleans up a terminated process
		 *
		 *	Cleans up any memory and kernel entries associated with the terminated
		 *	process. Also removes the process from the scheduler.
		 */
        void cleanupProcess(pidType pid);

        void _sysCall(const char call);
};

#endif // KERNEL_H_INCLUDED
