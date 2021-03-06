#include "init.h"

int main(int argc, char **argv) {
	cout << "Initializing emulated OS" << endl;
	/* Parse command line arguments */
	const char* scheduler = "\0";

	for ( int i = 1; i < argc; ++i) {
		if ( strcmp(argv[i], "-scheduler") == 0) {
			if ( ++i < argc )
				scheduler = argv[i];
			else {
				fprintf(stderr, "Scheduler type not specified");
				exit(1);
			}
		}
	}

	cScheduler* schedulerInstance;

	if ( strcmp(scheduler, "fcfs") == 0) {
		cout << "Using First-Come-First-Serve scheduler" << endl;
		schedulerInstance = new cFCFS();

	} else if ( strcmp(scheduler, "rr") == 0) {
		cout << "Using Round-Robin scheduler" << endl;
		schedulerInstance = new cRoundRobin();

	} else if ( strcmp(scheduler, "lottery") == 0) {
		cout << "Using Lottery scheduler" << endl;
		schedulerInstance = new cLottery();

	} else if ( strcmp(scheduler, "multi") == 0) {
		cout << "Using Multi-Level Queue scheduler" << endl;
		schedulerInstance = new cMultiLevel();

	} else {
		cout << "Scheduler type not specified" << endl;
		exit(1);
	}

	/* Initialize the Kernel */
    cKernel kernel = cKernel(*schedulerInstance);

	/* Boot kernel and catch any exceptions */
	try {
		kernel.boot();
	} catch (const std::string& emsg) {
		cerr << "Error Caught: " << emsg << endl;
		exit(-1);
	} catch (const kernelError& kerr) {
		cerr << "Kernel Exception: " << kerr.message << endl;
		exit(-1);
	}

	delete schedulerInstance;

	return 0;
}
