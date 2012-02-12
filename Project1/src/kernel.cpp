#include "kernel.h"

/* ======== Static members of cKernel ======== */
void cKernel::sig_catch(int signum, siginfo_t *info, void *context) {
			cKernel::kernel_instance->sigHandler(signum, info);
}

cKernel* cKernel::kernel_instance;
/* =========================================== */

cKernel::cKernel()
:clockTick(0), idGenerator(1), runningProc(NULL) {

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
	printf("A signal was received\n");

	if ( signum == clockSigValue /* && info->si_code == SI_TIMER */) {
		printf("Received clock signal\n");
	} else if (signum == blockSigValue ) {
		printf("Received block signal\n");
	} else if ( signum == charSigValue ) {
		printf("Received char signal\n");
	}

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
    newProc->PSW = 0;

	/* Place in storage datastructure */
	scheduler.initProcScheduleInfo(newProc);
	scheduler.addProcess(newProc);

    newProc->state = ready;

	//#ifdef DEBUG
    printf("Created new process: %s\n", filename);
    printf("Process ID: %d\n", newProc->pid);
    printf("Parent ID: %d\n", newProc->parent);
    printf("Memory Usage: %lu bytes\n", newProc->memory);
    printf("Program Contents:\n%s\n", newProc->processText);
    //#endif

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

void cKernel::swapProcesses(ProcessInfo *proc) {
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

	ProcessInfo *nextToRun = scheduler.getNextToRun();

	while ( scheduler.numProcesses() > 0 ) {
		swapProcesses(nextToRun);

		printf("Running Process %d\n", runningProc->pid);
		cpu.run();
		printf("Time until signal: %d\n", clockInterrupt.getTime());

		localPSW = cpu.getPSW();

		if ( localPSW & PS_EXCEPTION ) {
			/* Terminate the currently running process */
			//#ifdef DEBUG
			printf("Process raised an exception\n");
			//#endif

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
					/* In this emulated implementation, the cpu is aware
					 * of what is a system call so if it returns saying
					 * there is s syscall but the kernel doesn't recognize
					 * it then there is a major problem
					 */
					printf("Invalid system call\n");

					kernelError error;
					error.message = "Invalid system call";
					throw ((kernelError) error);
			}
		}

		pause(); //Have this here temporarily to check for signals
	}

	return;
}
