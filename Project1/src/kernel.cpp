#include "kernel.h"

cKernel::cKernel(cCPU& cpu)
:clockTick(0), CPUref(cpu), idGenerator(1) {
	/* Initialize datastructures */

	/* Load "main.trace" with parent 0*/
	for ( int i = 0; i < 20000; ++i) {
		initProcess(initProcessName, i);
	}

	/* Setup clock interrupt. Do near end. */
	clockInterrupt.setTimer(DEFAULT_TIMER);

	return;
}

cKernel::~cKernel() {

	return;
}

void cKernel::initProcess(const char *filename, pidType parent, int priority) {
    struct stat fileinfo;
    if ( stat(filename, &fileinfo) < 0 ) {
    	/* File likely doesn't exist */
    	fprintf(stderr, "Program %s does not exist\n", filename);

    	return;
    }

    ProcessInfo *newProc = (ProcessInfo*) malloc( sizeof(ProcessInfo) );
    newProc->memory = sizeof(ProcessInfo);
    newProc->memory += fileinfo.st_size;

    newProc->processText = (char*) malloc( fileinfo.st_size );

    /* Read process contents into memory */
    int processFile = open( filename, S_IRUSR);
    if ( read(processFile, newProc->processText, fileinfo.st_size) <= 0 ) {
    	/* Error Creating Process */
    	free(newProc->processText);
    	free(newProc);

    	return;
    }

    newProc->PC = 0;
    newProc->VC = 0;

    /* Initialize everything else */
    newProc->parent = parent;
    newProc->pid = idGenerator.getID();
    newProc->startCPU = clockTick;
    newProc->totalCPU = 0;
    newProc->priority = priority;

	/* Place in storage datastructure */

    newProc->state = ready;

    printf("Created new process: %s\n", filename);
    printf("Process ID: %d\n", newProc->pid);
    printf("Parent ID: %d\n", newProc->parent);
    printf("Program Contents:\n%s\n", newProc->processText);

    return;
}

void cKernel::cleanupProcess(pidType pid) {

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
