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
#define DEFAULT_PRIORITY 5

/* ============ Modify this section to change scheduler used ============== */
//Also have to change dependency in Makefile
#include "scheduler/fcfs.h"
//#define SCHEDULER_TYPE cFCFS
typedef cFCFS schedulerType;
/* ======================================================================== */

#ifndef PROCESS_H_INCLUDED
/** @cond */
__attribute__((error("process.h not included by scheduler")));
/** @endcond */
#endif

using namespace std;

static const char initProcessName[] = "main.trace";

class cKernel {
	private:
        cCPU cpu;
		int clockTick;

		//Devices:
		BlockDevice bDevice;
		CharDevice cDevice;
		ClockDevice clockInterrupt;

		ProcessInfo *runningProc; /**< #ProcessInfo */

		cIDManager idGenerator;

		/* Process Scheduler */
		schedulerType scheduler;

		/** Swap a process on the cpu
		 *
		 *	Takes the process in its parameter and swaps it with the
		 *	one currently running in the cpu.
		 */
		void swapProcesses(ProcessInfo* proc, bool switchMode = true);

		void cleanupProcess(ProcessInfo*);

		/* ------------ For Signal Handling ------------ */
		static cKernel *kernel_instance;
		static void sig_catch(int signum, siginfo_t *info, void *context);

		void sigHandler(int signum, siginfo_t *info);

		/** @cond */
		int charSigValue;
		int blockSigValue;
		int clockSigValue;
		/** @endcond */
		/* --------------------------------------------- */

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
		 *
		 * \exception kernelError
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

/** Struct containing kernel crash information
 *
 *	When the kernel crashes, important information is
 *	placed in here and then handled by the main function
 *	 in init.cpp.
 */
struct kernelError {
	string message;

	/* Add process and scheduler info */
};

/** \fn void cKernel::sigHandler(int signum, siginfo_t *info)
 *	Handler for all signals.
 *
 *	Each device uses a timer from SIGRTMIN -> SIGRTMAX. Because these are
 *	not required to be compile time constants, a switch statement cannot be
 *	used. When a signal is received by the kernel, careful consideration must
 *	be made to the current state. Below is each signal and how it is handled:
 *
 */

/** \def DEFAULT_TIMER
 *	Default timer value for devices. Measured in microseconds.
 */

/** \def DEFAULT_PRIORITY
 *	Default priority assigned to newly created processes. Only
 *	used if no other priority is provided.
 */

/** \var static const char initProcessName[]
 *	Name of the first program to run on the system.
 *
 *	When the kernel object is created, this program
 *	is loaded. It is run once cKernel::boot is called.
 */

#endif // KERNEL_H_INCLUDED
