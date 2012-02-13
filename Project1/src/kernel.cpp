#include "kernel.h"

/* ======== Static members of cKernel ======== */
void cKernel::sig_catch(int signum, siginfo_t *info, void *context) {
			cKernel::kernel_instance->sigHandler(signum, info);
}

cKernel* cKernel::kernel_instance;
/* =========================================== */

cKernel::cKernel()
:clockTick(0), idGenerator(1), runningProc(NULL) {
	/* Initialize trace output */
	initLog(traceLogFile);
	traceStream = getStream();

	pthread_mutex_init(&intLock, NULL);
	pthread_cond_init(&intCond, NULL);


	/* Load "main.trace" with parent 0 */
	initProcess(initProcessName, 0);
	assert( scheduler.numProcesses() == 1 ); //Make sure it loaded

	/* ==== Setup signal handlers ==== */
	charSigValue = CHARSIG;
	blockSigValue = BLOCKSIG;
	clockSigValue = CLOCKSIG;

	cKernel::kernel_instance = this;
	struct sigaction sa;

	sigset_t sa_set;
	sigfillset( &sa_set );

	sa.sa_sigaction = cKernel::sig_catch;
	sa.sa_mask = sa_set; //Block all signals while in the handler
	sa.sa_flags = SA_SIGINFO;

	sigdelset(&sa_set, SIGINT);
	sigdelset(&sa_set, clockSigValue);
	sigdelset(&sa_set, blockSigValue);
	sigdelset(&sa_set, charSigValue);

	sigprocmask(SIG_SETMASK, &sa_set, NULL);

	sigaction(clockSigValue, &sa, NULL);
	sigaction(blockSigValue, &sa, NULL);
	sigaction(charSigValue, &sa, NULL);

	return;
}

cKernel::~cKernel() {

	return;
}

/* Signal Handler */
void cKernel::sigHandler(int signum, siginfo_t *info) {
	if ( signum == clockSigValue /* && info->si_code == SI_TIMER */) {
		pthread_cond_signal(&intCond);

	} else if (signum == blockSigValue ) {
		printf("Received block signal\n");
	} else if ( signum == charSigValue ) {
		printf("Received char signal\n");
	} else {
		printf("Unknown signal received\n");
	}

	return;
}

void cKernel::initProcess(const char *filename, pidType parent, int priority) {
    struct stat fileinfo;
    if ( stat(filename, &fileinfo) < 0 ) {
    	/* File likely doesn't exist */
    	fprintf(stderr, "Program %s does not exist \n", filename);

    	return;
    }

    ProcessInfo *newProc = (ProcessInfo*) malloc( sizeof(ProcessInfo) );
    newProc->memory = fileinfo.st_size;

    newProc->processText = (char*) malloc( fileinfo.st_size );

    /* Read process contents into memory */
    int processFile = open( filename, S_IRUSR);
    if ( read(processFile, newProc->processText, fileinfo.st_size) <= 0 ) {
    	/* Error Creating Process */
    	perror("Error creating new process");
    	free(newProc->processText);
    	free(newProc);
    	close(processFile);

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
    newProc->PSW = 0;

	/* Place in storage datastructure */
	scheduler.initProcScheduleInfo(newProc);
	newProc->state = ready;

	scheduler.addProcess(newProc);

	#ifdef DEBUG
    printf("Created new process: %s \n", filename);
    printf("Process ID: %d \n", newProc->pid);
    printf("Parent ID: %d \n", newProc->parent);
    printf("Memory Usage: %lu bytes \n", newProc->memory);
    printf("Program Contents:\n%s \n", newProc->processText);
    #endif

    /* Cleanup */
    close(processFile);

    return;
}

void cKernel::cleanupProcess(ProcessInfo* proc) {
	idGenerator.returnID(proc->pid);

	free(proc->processText);
	free(proc);

	return;
}

void cKernel::swapProcesses(ProcessInfo *proc, bool switchMode) {
	if ( switchMode )
		cpu.setUserMode();

	if ( runningProc == proc )
		return;

	/* Store state of current process */
	if ( runningProc != NULL ) {
		runningProc->PC = cpu.getSetPC(proc->PC);
		runningProc->VC = cpu.getSetVC(proc->PC);
		runningProc->PSW = cpu.getSetPSW(proc->PSW);
	} else {
		cpu.getSetPC(proc->PC);
		cpu.getSetVC(proc->PC);
		cpu.getSetPSW(proc->PSW);
	}

	cpu.setText(proc->processText);

	runningProc = proc;

	return;
}

void cKernel::boot() {
	uint16_t localPSW;

	clockInterrupt.setTimer(DEFAULT_TIMER); //Setup clock interrupt

	//Make sure the lock is aquired or there is no point continuing
	assert( pthread_mutex_lock(&intLock) == 0);

	ProcessInfo* nextToRun;

	do {
		//Wait to receive signal from interrupt handler
		pthread_cond_wait(&intCond, &intLock);
		fprintf(traceStream, "ClockTick: %d\n", clockTick);

		nextToRun = scheduler.getNextToRun();

		//Swap out the previous process on the cpu
		swapProcesses(nextToRun);

		//Execute the new process
		cpu.run();

		localPSW = cpu.getPSW();

		if ( localPSW & PS_FINISHED) {
			localPSW ^= PS_FINISHED;
			cpu.setPSW(localPSW);

		} else if ( localPSW & PS_EXCEPTION ) {
			/* Terminate the process */
			printf("Process raised an exception\n");

			scheduler.removeProcess(runningProc);
			cleanupProcess(runningProc);
			runningProc = NULL;
		} else if ( localPSW & PS_SYSCALL ) {
			printf("Process made a system call\n");

			switch ( cpu.getOpcode() ) {
				case 'C':
					printf("System call is for a new process\n");
					printf("\tProcess Name = %s\n", cpu.getParam(1));
					printf("\tProcess Priority = %d\n", atoi(cpu.getParam(0)));

					initProcess(cpu.getParam(1), runningProc->pid, atoi(cpu.getParam(0)));
					break;

				case 'I':
					printf("System call is for device I/O\n");

					break;

				default:
					printf("Invalid system call\n");
					/* Kill the process */
					scheduler.removeProcess(runningProc);
					cleanupProcess(runningProc);
					runningProc = NULL;

					break;
			}

			localPSW ^= PS_SYSCALL;
			cpu.setPSW(localPSW);

		} else if ( localPSW & PS_TERMINATE ) {
			printf("Process %d has terminated\n", runningProc->pid);

			scheduler.removeProcess(runningProc);
			cleanupProcess(runningProc);
			runningProc = NULL;
		}

		++clockTick;
	} while( scheduler.numProcesses() > 0 );

	printf("All processes have finished executing. Exiting kernel.\n");

	return;
}
