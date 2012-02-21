#include "kernel.h"

/* ======== Static members of cKernel ======== */
void cKernel::sig_catch(int signum, siginfo_t *info, void *context) {
	cKernel::kernel_instance->sigHandler(signum, info);
}

cKernel* cKernel::kernel_instance;
/* =========================================== */

cKernel::cKernel(cScheduler& s)
:clockTick(0), idGenerator(1), runningProc(NULL), procLogger(procLogFile),
bDevice(DEFAULT_BTIMER), cDevice(DEFAULT_CTIMER), scheduler(s) {
	/* ---- Initialize trace output ----*/
	traceStream = initLog(traceLogFile);
	assert(traceStream != NULL);
	/* -------------------------------- */

	/* Send logging info to scheduler */
	scheduler.addLogger(traceStream);
	scheduler.addProcLogger(&procLogger);
	/* -------------------------------- */

	pthread_mutex_init(&intLock, NULL);
	pthread_cond_init(&intCond, NULL);

	sem_init(&DevSigSem, 0, 0); //Notifies that some device has finished
	sem_init(&BSigSem, 0, 0);
	sem_init(&CSigSem, 0, 0);

	/* -------- Initialize cpu data --------*/
	cpu.initTraceLog();
	cpu.initClockPulse(&intLock, &intCond);
	/* ----------------------------------- */


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
	sigdelset(&sa_set, SIGINT); //To easily kill the process

	sa.sa_sigaction = cKernel::sig_catch;
	sa.sa_mask = sa_set; //Block all signals while in the handler
	sa.sa_flags = SA_SIGINFO;

	//Allow main thread to receive clock interrupts
	sigdelset(&sa_set, clockSigValue);
	sigprocmask(SIG_SETMASK, &sa_set, NULL);

	sigaction(clockSigValue, &sa, NULL);
	sigaction(blockSigValue, &sa, NULL);
	sigaction(charSigValue, &sa, NULL);

	return;
}

cKernel::~cKernel() {
	pthread_cancel(deviceThread);
	pthread_join(deviceThread, NULL);

	closeLog();

	return;
}

/* Signal Handler */
void cKernel::sigHandler(int signum, siginfo_t *info) {
	if ( signum == clockSigValue /* && info->si_code == SI_TIMER */) {
		pthread_cond_signal(&intCond);

	} else if (signum == blockSigValue ) {
		sem_post(&BSigSem);
		sem_post(&DevSigSem);
	} else if ( signum == charSigValue ) {
		sem_post(&CSigSem);
		sem_post(&DevSigSem);
	} else {
		printf("Unknown signal received\n");
	}

	return;
}

void cKernel::initProcess(const char *filename, pidType parent, int priority) {
	assert( traceStream != NULL);
	assert( filename != NULL );

	fprintf(traceStream, "Creating process: %s with parent = %d  priority = %d\n",
			filename, parent, priority);

	if ( priority < 0 ) {
		printf("Invalid process priority: %d\n", priority);
		fprintf(traceStream, "Invalid process priority: %d\n", priority);
		return;
	}

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
    	if ( errno == 0 ) {
    		fprintf(traceStream, "New process has no text. Aborting creation.\n");
    		printf("New process has no text. Aborting creation.\n");
    	} else {
    		fprintf(traceStream, "Error reading process contents. Aborting creation.\n");
    		printf("Error reading process contents. Aborting creation.\n");
    	}

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
	procLogger.addProcess(newProc, filename);

	fprintf(traceStream, "\t New Process created with pid = %d\n", newProc->pid);

    /* Cleanup */
    close(processFile);

    return;
}

void cKernel::cleanupProcess(ProcessInfo* proc) {
	if ( proc == runningProc )
		runningProc = NULL;

	scheduler.removeProcess(proc);

	idGenerator.returnID(proc->pid);

	procLogger.rmProcess(proc);

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

	cpu.setMaxPC(proc->memory);
	cpu.setText(proc->processText);
	cpu.pid = proc->pid;

	runningProc = proc;

	return;
}

void* deviceHandle(void* args) {
	cKernel* kernel = (cKernel*) args;

	sigset_t sa;
	sigfillset(&sa);
	sigdelset(&sa, kernel->blockSigValue);
	sigdelset(&sa, kernel->charSigValue);
	sigdelset(&sa, SIGINT);

	sigprocmask(SIG_SETMASK, &sa, NULL);

	int error;

	while ( true ) {
		while ( (error = sem_wait(&kernel->DevSigSem)) && error == EINTR ) {
			perror("Problem with device handler");
			exit(-1);
		}

		if ( sem_trywait(&kernel->BSigSem) == 0) {
			printf("Received block signal\n");
			ProcessInfo* toUnblock = kernel->bDevice.timerFinished();
			assert( toUnblock != NULL );
			printf("Unblocking process\n");
			kernel->scheduler.unblockProcess(toUnblock);
		}

		if ( sem_trywait(&kernel->CSigSem) == 0) {
			printf("Received char signal\n");
			ProcessInfo* toUnblock = kernel->cDevice.timerFinished();
			assert( toUnblock != NULL );
			printf("Unblocking process\n");
			kernel->scheduler.unblockProcess(toUnblock);
		}

	}

	return NULL;
}

void cKernel::boot() {
	uint16_t localPSW;

	clockInterrupt.setTimer(DEFAULT_TIMER); //Setup clock interrupt

	//Make sure the lock is aquired or there is no point continuing
	assert( pthread_mutex_lock(&intLock) == 0);

	ProcessInfo* nextToRun;

	assert( pthread_create(&deviceThread, NULL, deviceHandle, this) == 0);


	do {

		nextToRun = scheduler.getNextToRun();
		assert(nextToRun != NULL);

		//Wait to receive signal from interrupt handler
		pthread_cond_wait(&intCond, &intLock);
		printf("ClockTick: %d\n", clockTick);
		fprintf(traceStream, "ClockTick: %d\n", clockTick);

		++(nextToRun->totalCPU); //It is being allocated a CPU cycle

		//Swap out the previous process on the cpu
		swapProcesses(nextToRun);

		//Execute the new process
		cpu.run();

		localPSW = cpu.getPSW();

		if ( localPSW & PS_FINISHED) {
			localPSW ^= PS_FINISHED;
			cpu.setPSW(localPSW);

		} else if ( localPSW & PS_EXCEPTION ) {
			printf("Process raised an exception\n");

			if ( localPSW & PS_ABNORMAL ) {
				fprintf(traceStream, "Process %d raised an abnormal termination exception. Cleaning up soon.\n", runningProc->pid);
			} else {
				fprintf(traceStream, "Process %d raised an exception. Terminating process.\n", runningProc->pid);
			}

			cleanupProcess(runningProc);
			runningProc = NULL;
		} else if ( localPSW & PS_SYSCALL ) {

			switch ( cpu.getOpcode() ) {
				case 'C':
					printf("System call for a new process\n");
					printf("\tProcess Name = %s\n", cpu.getParam(1));
					printf("\tProcess Priority = %d\n", atoi(cpu.getParam(0)));

					initProcess(cpu.getParam(1), runningProc->pid, atoi(cpu.getParam(0)));
					break;

				case 'I': {
					printf("System call for device I/O\n");

					char* devClassStr = cpu.getParam(0);
					assert( strlen(devClassStr) > 0 );
					char devClass = devClassStr[0];

					if ( !(devClass == 'B' || devClass == 'C') ) {
						printf("Invalid device type: %c\n", devClass);
						fprintf(traceStream, "Invalid device type: %c\n", devClass);
						cleanupProcess(runningProc);
						runningProc = NULL;

						break;
					}

					cpu.executePrivSet(3, clockTick);
					localPSW = cpu.getPSW();

					//Check to make sure everything is ok
					if ( localPSW & PS_EXCEPTION ) {
						printf("Process %d raised an exception during device I/O\n", runningProc->pid);
						fprintf(traceStream, "Process %d raised an exception during device I/O\n", runningProc->pid);

						cleanupProcess(runningProc);
						runningProc = NULL;
					}

					//Put in request for device I/O

					switch ( devClass ) {
						case 'B': {
							printf("Scheduling pid = %d for I/O on block device\n", runningProc->pid);
							fprintf(traceStream, "Scheduling pid = %d for I/O on block device\n", runningProc->pid);
							bDevice.scheduleDevice(runningProc);
							printf("Blocking process in scheduler\n");
							scheduler.setBlocked(runningProc);
							break;
						}

						case 'C': {
							printf("Scheduling pid = %d for I/O on character device\n", runningProc->pid);
							fprintf(traceStream, "Scheduling pid = %d for I/O on character device\n", runningProc->pid);
							cDevice.scheduleDevice(runningProc);
							scheduler.setBlocked(runningProc);
							break;
						}

						default: {
							fprintf(traceStream, "Invalid Device Class %c: Terminating Process\n", devClass);
							cleanupProcess(runningProc);
							runningProc = NULL;
							break;
						}
					}

					break;
				} //End Device I/O syscall

				default:
					printf("Invalid system call\n");
					/* Kill the process */
					cleanupProcess(runningProc);
					runningProc = NULL;

					break;
			}

			localPSW ^= PS_SYSCALL;
			cpu.setPSW(localPSW);

		} else if ( localPSW & PS_TERMINATE ) {
			printf("Process %d has terminated\n", runningProc->pid);
			fprintf(traceStream, "Process %d has termintated: VC = %d\n", runningProc->pid, cpu.getSetVC(0));

			cleanupProcess(runningProc);
			runningProc = NULL;
		}

		++clockTick;
		fprintf(traceStream, "\n");
		fflush(traceStream);
	} while( scheduler.numProcesses() > 0 );

	fprintf(traceStream, "All processes have finished executing. Exiting kernel.\n");
	printf("All processes have finished executing. Exiting kernel.\n");

	return;
}
