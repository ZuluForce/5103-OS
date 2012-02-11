#include "kernel.h"

cKernel::cKernel()
:clockTick(0), idGenerator(1), runningProc(NULL) {

	/* Load "main.trace" with parent 0 */
	initProcess(initProcessName, 0);
	assert( scheduler.numProcesses() == 1 ); //Make sure it loaded

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
    	perror("Error creating new process");
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
	scheduler.initProcScheduleInfo(newProc);
	scheduler.addProcess(newProc);

    newProc->state = ready;

    printf("Created new process: %s\n", filename);
    printf("Process ID: %d\n", newProc->pid);
    printf("Parent ID: %d\n", newProc->parent);
    printf("Program Contents:\n%s\n", newProc->processText);

    /* Cleanup */
    close(processFile);

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


void cKernel::boot() {
	ProcessInfo *nextToRun = scheduler.getNextToRun();

	while ( scheduler.numProcesses() > 0 ) {

	}
	return;
}
