#include "vmm_core.h"

/* These could be passed around as parameters but it
 * would be more clumsy than is necessary
 */
extern INIReader* settings;
extern FILE* logStream;

cVMM* VMMCore;

cVMM::cVMM(vector<sProc*>& _procs, cPRPolicy& _PRM, cCleanDaemon& _cDaemon)
: procs(_procs),PRModule(_PRM),cDaemon(_cDaemon) {

	int page_bits = EXTRACTP(int,Global,page_bits);
	int offset_bits = EXTRACTP(int,Global,offset_bits);
	numFrames = EXTRACTP(uint32_t, Global, total_frames);

	/* ---- Print Init Info to Screen ---- */
	cout << "Virtual Memory Manager (VMM) started with settings:" << endl;
	cout << "\tPage Bits: " << page_bits << endl;
	cout << "\tOffset Bits: " << offset_bits << endl;
	cout << "\t# Frames (Global): " << numFrames << endl;
	cout << "\t# Processes: " << procs.size() << endl;

	if ( procs.size() == 0 ) {
		cerr << "No processes to run. Exiting..." << endl;
		exit(0);
	}

	if ( page_bits + offset_bits != 32 ) {
		cerr << "Word size is not 32 bits" << endl;
		cerr << "\tPage Bits: " << page_bits << endl;
		cerr << "\tOffset Bits: " << offset_bits << endl << endl;
		exit(-1);
	}

	PS = 1 << offset_bits;
	PT_Size = 1 << page_bits;

	VC = 0;
	cpu.addVC(&VC);
	ioCtrl = new cIOControl(procs.size(), &VC);

	pageInCount = pageOutCount = 0;

	initProcesses();

	VMMCore = this;

	int io_time = EXTRACTP(int, Timings, pf_time);
	int cs_time = EXTRACTP(int, Timings, cs_time);

	//This is the time to context switch and execute/pf all but the first process
	//+2: one is for cpu instruction time and the other for io_req time
	int timeCheck = ((procs.size() - 2) * (cs_time + 2));

	/* Precheck for potential circular faults */
	if ( numFrames < procs.size() &&
		io_time <= timeCheck) {
		cout << endl;
		cout << "!!!! Detected potential circular fault before starting !!!!" << endl;
		cout << "First page-in would be replaced before process 0 executes again" << endl;
		cout << "This may be a designed test so don't worry. Otherwise fix the problem or" << endl;
		cout << "use setting ignore_circular_fault_warn to skip this" << endl << endl;

		if ( !EXTRACTP(bool, Policy, ignore_circular_fault_warn) )
			exit(-1);
	}

	return;
}

cVMM::~cVMM() {

	return;
}

void cVMM::initProcesses() {
	cout << "Initializing all processes..." << endl;

	scheduler.addProcesses(procs);

	vector<sProc*>::iterator it;

	void* newPT;

	long pt_byte_size = sizeof(sPTE) * PT_Size;

	for ( it = procs.begin(); it != procs.end(); ++it) {
		/* Allocate new complete page table */
		//newPT = malloc( pt_byte_size );

		/* Clear the new table */
		//memset(newPT, '\0', pt_byte_size);

		(*it)->PTptr = (sPTE*) malloc( sizeof(sPTE) * PT_Size);
		memset((*it)->PTptr, '\0', pt_byte_size);
	}

	cout << "Finished initializing process' virtual memory" << endl;

	return;
}

void cVMM::cleanupProcess(sProc* proc) {
	assert(proc != NULL);

	/* We hold on to the process struct to print out stats
	 * in the end so we use PC values = -1 as a sentinel of
	 * a terminated process
	 */
	proc->PC = -1;
	proc->maxPC = -1;
	proc->finishTime = VC;

	int cleanupCount = 0;

	for ( int i = 0; i < PT_Size; ++i) {
		if ( proc->PTptr[i].flags[FI_PRESENT] ) {
			PRModule.returnFrame(proc->PTptr[i].frame);
			proc->PTptr[i].flags[FI_PRESENT] = false;

			++cleanupCount;
		}
	}

	cout << "Freed " << cleanupCount << " frames that the terminating process was using" << endl;
	fprintf(logStream, "Freed %d frames that the terminating process was using\n", cleanupCount);

	/* Free page table memory */
	//free(proc->PTptr);

	return;
}

/* Any time page/s are removed outside of the normal demand
 * paging and PR policy this should be called to prevent
 * false positives on the circular fault check.
 *
 * For example, when the cleaning daemon comes through this
 * should be called so the cpu doesn't think that the process
 * execution order caused a circular fault when in fact it was
 * an unfortunate circumstance of the cleaning daemon.
 */
void cVMM::clearCircChecks() {
	vector<sProc*>::iterator it;

	for ( it = procs.begin(); it != procs.end(); ++it) {
		(*it)->circularFaultCheck = false;
	}

	return;
}

sIOContext* cVMM::pageOut(sProc* proc, uint32_t page, sIOContext* ctx) {
	assert(proc != NULL);

	ioCtrl->scheduleIO(proc, page, IO_OUT, ctx);
	++pageOutCount;
}

sIOContext* cVMM::pageIn(sProc* proc, uint32_t page, eIOType iotype, sIOContext* ctx) {
	assert(proc != NULL);

	ioCtrl->scheduleIO(proc, page, iotype, ctx);
	++pageInCount;
}

void cVMM::tickController(int times) {
    //ioCtrl->tick(times);
    for (int i = 0; i < times; i++){
        ++VC;
        ioCtrl->tick();
    }

	return;
}

void cVMM::printResults() {
	cout << "Collecting/Recording results" << endl;
	fprintf(logStream, "Collecting/Recording resuts\n");

	string logName = EXTRACTP(string, Results,file);
	cout << "LogName: " << logName << endl << endl;
	fprintf(logStream, "LogName: %s\n", logName.c_str());

	if ( logName.compare("") == 0 ) {
		cerr << "No results file indicated!!" << endl;
		return;
	}

	ofstream log;
	log.open(logName.c_str(), ios::out | ios::trunc);

	if ( !log.is_open() ) {
		cerr << "Error opening result log file " << logName << endl;
	}

	/* ---- Print Setting Info for Python Script ---- */
	log << PS << ":" << numFrames << ":" << PRModule.name();
	log << ":" << EXTRACTP(string, Results,title) << endl;

	int g_cs, g_pf, g_et, g_tlbhit, g_tlbmiss;
	g_cs = g_pf = g_et = g_tlbhit = g_tlbmiss = 0;

	vector<sProc*>::iterator it;

	for ( it = procs.begin(); it != procs.end(); ++it ) {
		log << "Process " << (*it)->pid << ":" << endl;

		g_cs += (*it)->cswitches;
		log << "Context Switches: " << (*it)->cswitches << endl;

		g_pf += (*it)->pageFaults;
		log << "Page Faults: " << (*it)->pageFaults << endl;

		g_et += (*it)->clockTime;
		log << "Execution Time: " << (*it)->clockTime << endl;

		log << "Total Time: " << (*it)->finishTime << endl;

		g_tlbhit += (*it)->tlbhit;
		log << "TLB Hits: " << (*it)->tlbhit << endl;

		g_tlbmiss += (*it)->tlbmiss;
		log << "TLB Miss: " << (*it)->tlbmiss << endl;

		log << endl; //For the python script to work
	}

	log << "\nGlobal VMM info:" << endl;
	log << "Context Switches: " << g_cs << endl;
	log << "Page Faults: " << g_pf << endl;
	log << "Execution Time: " << g_et << endl;
	log << "Page In: " << pageInCount << endl;
	log << "Page Out: " << pageOutCount << endl;
	log << "TLB Hits: " << g_tlbhit << endl;
	log << "TLB Miss: " << g_tlbmiss << endl;

	fprintf(logStream, "Test Parameters: \n");
	fprintf(logStream, "\tPage Size: %d\n", PS);
	fprintf(logStream, "\tFrame Count: %d", numFrames);
	fprintf(logStream, "\tPR Module: %s\n", PRModule.name());
}


sProc* cVMM::getProcess(unsigned int id) {
	return procs.at(id);
}

int cVMM::start() {
	sProc* runningProc;

	uint8_t result;

	queue<uint64_t>& finishedIO = ioCtrl->getFinishedQueue();
	queue<sPTE*>& finishedIOPage = ioCtrl->getFinishedPTEQueue();
	uint64_t finishInfo = 0;
	uint32_t fPID = 0;
	uint32_t fFrame = 0;
	sPTE* fPTE = NULL;

	cout << "-*-Virtual Counter: " << this->VC << "-*-" << endl;
	fprintf(logStream, "Virtual Counter: %d\n", VC);

	while ( scheduler.numProcesses() ) {

		//Process all finished I/O
		while ( !finishedIO.empty() ) {
			finishInfo = finishedIO.front();
			fPTE = finishedIOPage.front();
			finishedIO.pop();
			finishedIOPage.pop();

			fPID = finishInfo / ( 0x100000000 );
			fFrame = finishInfo % ( 0x100000000 );

			cout << "***Unblocking Process " << fPID << "***" << endl;
			scheduler.unblockProcess(procs.at(fPID));

			PRModule.finishedIO(procs.at(fPID), fPTE);
			//PRModule.unpinFrame(fFrame);
			PRModule.unpinFrame(fPTE->frame);

			fPID = 0;
			fFrame = 0;
		}

		runningProc = scheduler.getNextToRun();
		/* This means there are still valid processes but none
		 * can currently run.
		 */
		if ( runningProc == NULL ) {
			++VC;
			cout << "Virtual Counter: " << VC << endl;
			fprintf(logStream, "-*-Virtual Counter: %d-*-\n", VC);

			ioCtrl->tick();
			continue;
		}

		//++(runningProc->clockTime);

		cpu.switchProc(runningProc);

		result = cpu.run();

		if ( result & CPU_TERM ) {
			scheduler.removeProcess(runningProc);
			cout << "Process " << runningProc->pid << " termintated" << endl;
			fprintf(logStream, "Process %d terminated\n", runningProc->pid);
			/* Cleanup page table */
			cleanupProcess(runningProc);

			runningProc = NULL;
		} else if ( result & CPU_PF || result & CPU_CIRC_PF) {
			cout << "**Process "<< runningProc->pid << " Page Faulted**" << endl;
			fprintf(logStream, "**Process %d page faulted**\n", runningProc->pid);

			++runningProc->pageFaults;

			ePRStatus result;

			if ( result & CPU_CIRC_PF ) {
				cout << "CPU thinks there may be a circular fault" << endl;
				fprintf(logStream, "CPU thinks there may be a circular fault\n");

				result = PRModule.resolveCircularPF(runningProc, cpu.getFaultPage());

				switch ( result ) {
				case PR_NO_AVAIL:
				case PR_NO_ACTION:
					//So we don't get future false positives
					runningProc->circularFaultCheck = false;

					break;

				case PR_SERVICED:
				case PR_SERVICED_IO:
					cout << "Blocking process" << endl;
					scheduler.setBlocked(runningProc);
					break;

				default:
					cout << "Invalid result value from PRModule: " << result << endl;
					break;
				}
			} else {
				result = PRModule.resolvePageFault(runningProc, cpu.getFaultPage());

				switch( result ) {
				case PR_SERVICED:
				case PR_SERVICED_IO:
					cout << "Blocking process" << endl;
					scheduler.setBlocked(runningProc);
					break;

				case PR_NO_AVAIL:
				case PR_NO_ACTION:
					runningProc->circularFaultCheck = false;
					break;
				}
			}

		} else {
			PRModule.finishedQuanta(runningProc);
		}

		/* Cleaning Daemon checks if cleaning is needed and
		 * notifies the PR module how many to clean, if any */
		if ( PRModule.clearPages(cDaemon.checkClean()) )
			clearCircChecks();

		cout << endl;
		fprintf(logStream, "\n");
	}

	/* Gather simulation data and output it */
	printResults();

	/* ------------------------------------ */
	return 0;
}
