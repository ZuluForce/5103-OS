#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

typedef unsigned int pidType;

enum eProcState {ready, running, blocked, terminated};

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

	/* Total memory being used by the process */
    unsigned long memory;
};

#endif // PROCESS_H_INCLUDED
