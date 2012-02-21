#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

/** @file */

#include <vector>
#include <sys/stat.h>
#include <semaphore.h>

#include "cpu.h"
#include "devices/char_device.h"
#include "devices/block_device.h"
#include "devices/clock_device.h"

#include "scheduler/allSchedulers.h"

#define DEFAULT_TIMER 250000 	//ClockTick every .25 seconds
#define CDEVICE_SCALE 4 		//Character device: 1 second
#define BDEVICE_SCALE 8 		//Block Device: 2 seconds
#define DEFAULT_CTIMER (DEFAULT_TIMER * CDEVICE_SCALE)
#define DEFAULT_BTIMER (DEFAULT_TIMER * BDEVICE_SCALE)
#define DEFAULT_PRIORITY 5

#ifndef PROCESS_H_INCLUDED
/** @cond */
__attribute__((error("process.h not included by scheduler")));
/** @endcond */
#endif

using namespace std;

static const char initProcessName[] = "main.trace";
static const char traceLogFile[] = "trace.log";
static const char procLogFile[] = "proc.log";

/** Core managing class for this simulated OS */
class cKernel {
	private:
		/* ---- Log Information ---- */
		FILE* traceStream;
		cProcessLogger procLogger;
		/* ------------------------- */

		/* CPU specific information */
        cCPU cpu;
		int clockTick;

		//For the signal handler to notify of a clock pulse
		/* Here I didn't use a semaphore because it is possible that
		 * more than multiple clockticks could come before the kernel
		 * next checked which then would result in the rapid execution
		 * of the following clock cycles.
		 */
		pthread_mutex_t intLock; /**< Lock for condition variable */
		pthread_cond_t intCond; /**< For synchronization with clocktick */
		/* ------------------------ */

		/* -------- Devices: -------- */
		cBlockDevice bDevice;
		cCharDevice cDevice;
		ClockDevice clockInterrupt;

		pthread_t deviceThread;

		sem_t DevSigSem; /**< A device interrupt has been received */
		sem_t BSigSem;	/**< A block device interrupt has been received */
		sem_t CSigSem;	/**< A char device interrupt has been received */
		/* -------------------------- */

		ProcessInfo *runningProc; /**< Process currently on the cpu */

		cIDManager idGenerator; /**< For generating new process PID's */

		/* Process Scheduler */
		cScheduler& scheduler;

		/** Swap a process on the cpu
		 *
		 *	Takes the process in its parameter and swaps it with the
		 *	one currently running in the cpu.
		 */
		void swapProcesses(ProcessInfo* proc, bool switchMode = true);

		/** Cleanup process memory and state
		 *
		 *	This is called when a process is being removed from the system
		 *	and its memory and any remaining state information needs to
		 *	be cleaned up.
		 */
		void cleanupProcess(ProcessInfo*);

		/* ------------ For Signal Handling ------------ */
		static cKernel *kernel_instance;

		/** Static function just for capturing signals */
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
		cKernel(cScheduler&);
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

		//Function to create class just for handling devices
		/** Thread function for handline device interrupts
		 *
		 *	Since limited work can be done within the signal handlers
		 *	and the main thread may block, a separate thread must be
		 *	present to handle device interrupts.
		 *
		 *	@see DevSigSem
		 *	@see CSigSem
		 *	@see BSigSem
		 *
		 *	@param void* This is a pointer to the kernel instance.
		 */
        friend void* deviceHandle(void*);
};

/** Struct containing kernel crash information
 *
 *	When the kernel crashes, important information is
 *	placed in here and then handled by the main function
 *	 in init.cpp.
 */
struct kernelError {
	string message;

	/* Add process and scheduler error info */
};

/** \fn void cKernel::sigHandler(int signum, siginfo_t *info)
 *	Handler for all signals.
 *
 *	For clock interrupt signals, the handler signals on a condition
 *	variable which the main thread will be waiting on.
 *
 *	For block and character devices, the appropriate semaphores are
 *	incremented which will unblock the waiting thread to act on them.
 *
 *	This function is called by cKernel::sig_catch
 *	@see deviceHandle(void*)
 *	@see cKernel::intCond
 *	@see DevSigSem
 *	@see CSigSem
 *	@see BSigSem
 *
 */

/** \def DEFAULT_TIMER
 *	Default timer for clock interrupt.
 */

/** @def CDEVICE_SCALE
 *	Time scale for character devices relative to default clock.
 */

/** @def BDEVICE_SCALE
 *	Time scale for block devices relative to default clock.
 */

/** @def DEFAULT_CTIMER
 *	Complete time for C Devices after scaling.
 */

/** @def DEFAULT_BTIMER
 *	Complete time for B Devices after scaling.
 */

/** @def DEFAULT_PRIORITY
 *	Default priority assigned to newly created processes. Only
 *	used if no other priority is provided.
 */

/** @var static const char initProcessName[]
 *	Name of the first program to run on the system.
 *
 *	When the kernel object is created, this program
 *	is loaded. It is run once cKernel::boot is called.
 */

/** @var static const char traceLogFile[]
 *	Name of trace log file.
 *
 *	Per clocktick informatino is logged here.
 *
 *	@see initLog(const char* filename)
 */

/** @var static const char procLogFile[]
 *	Name of log file for process info.
 *
 *	This file is created by the process logger
 *	and it is where top gathers most of its process
 *	info.
 *
 *	@see cProcessLogger
 */

#endif // KERNEL_H_INCLUDED
