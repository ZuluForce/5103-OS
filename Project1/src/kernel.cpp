#include "kernel.h"


cKernel::cKernel(cCPU& cpu)
:clockTick(0), CPUref(cpu) {
	/* Initialize datastructures */

	/* Load "main.trace" with parent 0*/
	initProcess(INIT_PROGRAM, 0);

	/* Setup clock interrupt. Do near end. */
	clockInterrupt.setTimer(DEFAULT_TIMER);

	return;
}

cKernel::~cKernel() {

	return;
}

void cKernel::initProcess(char *filename, int parent) {
    struct stat fileinfo;
    stat(filename, &fileinfo);

    ProcessInfo *newProc = malloc( sizeof(ProcessInfo) );
    newProc->processMem = malloc( fileinfo.st_size );

    /* Read process contents into memory */
    int processFile = open( filename, S_IRUSR);
    read(processFile, newProc->processMem, fileinfo.st_size);
    newProc->PC = 0;
    newProc->VC = 0;

    /* Initialize everything else */
    newProc->parent = parent;
    newProc->pid = 0 /* Implement id system */;
    newProc->startCPU = clockTick;
    newProc->totalCPU = 0;

	/* Place in storage datastructure */

    newProc->state = ready;
    return;
}

void cKernel::_sysCall(char call) {
	switch ( call ) {
		case 'C':
			break;

		case 'P':
			/* Called when a user process tries to execute a priveleged instr */
			/* Kill the user process */
			break;

		default:
			/* Invalid Sys Call, Abort process? */
			break;
	}

	return;
}
