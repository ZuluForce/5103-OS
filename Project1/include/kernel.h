#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <string>
#include <vector>

#include "cpu.h"
#include "devices/clock_device.h"

#define DEFAULT_TIMER 1000 //Not sure on this one, need to read Specs
#define INIT_PROGRAM "main.trace"

using namespace std;

typedef enum {ready, running, blocked, term} eProcState;

struct ProcessInfo {
	/* I was thinking we could save the name so the output of
	 * Top would look nice later.
	 */
	//string name;

    unsigned int parent, pid;
    unsigned int startCPU, totalCPU;

	eProcState state;
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
};

class cKernel {
	private:
        cCPU& CPUref;
		int clockTick;

		//Devices:
		//vector<BlockDevice*> B_Devices;
		//vector<CharDevice*> C_Devices;
		ClockDevice clockInterrupt;

		ProcessInfo *RunningProc;

		/* Need to formalize process storage datastructures
		 * to write this one.
		 */
		//void swapProcess(...);

	public:
		cKernel(cCPU& cpu);
		~cKernel();


        void initProcess(char *filename, int parent);

        void _sysCall(char call);
};

#endif // KERNEL_H_INCLUDED
