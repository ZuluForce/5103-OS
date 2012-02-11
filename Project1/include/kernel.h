#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cpu.h"
#include "process.h"
#include "devices/clock_device.h"
#include "utility/id.h"

#define DEFAULT_TIMER 1000 //Not sure on this one, need to read Specs
#define DEFAULT_PRIORITY 0

/* ============ Modify this section to change scheduler used ============== */
#include "scheduler/fcfs.h"
#define SCHEDULER_TYPE cFCFS
/* ======================================================================== */

using namespace std;

static const char initProcessName[] = "main.trace";

class cKernel {
	private:
        cCPU& CPUref;
		int clockTick;

		//Devices:
		//vector<BlockDevice*> B_Devices;
		//vector<CharDevice*> C_Devices;
		ClockDevice clockInterrupt;

		ProcessInfo *runningProc;

		cIDManager idGenerator;
		/* Need to formalize process storage datastructures
		 * to write this one.
		 */
		//void swapProcess(...);

		/* Process Scheduler */
		SCHEDULER_TYPE scheduler;

	public:
		cKernel(cCPU& cpu);
		~cKernel();


        void initProcess(const char *filename, pidType parent, int priority = DEFAULT_PRIORITY);
        void cleanupProcess(pidType pid);

        void _sysCall(const char call);
};

#endif // KERNEL_H_INCLUDED
