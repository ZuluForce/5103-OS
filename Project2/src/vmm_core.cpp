#include "vmm_core.h"

//extern INIReader* settings;

cVMM::cVMM(vector<sProc*>& _procs, cPRPolicy& _PRM)
: procs(_procs), PRModule(_PRM) {

	int page_bits = EXTRACTP(int,Global,page_bits);
	int off_bits = EXTRACTP(int,Global,offset_bits);
	numFrames = EXTRACTP(uint32_t, Global, total_frames);

	/* ---- Print Init Info to Screen ---- */
	cout << "Virtual Memory Manager (VMM) started with settings:" << endl;
	cout << "\tPage Bits: " << page_bits << endl;
	cout << "\tOffset Bits: " << off_bits << endl;
	cout << "\t# Frames (Global): " << numFrames << endl;
	cout << "\t# Processes: " << procs.size() << endl;

	PS = 1 << off_bits;
	PT_Size = 1 << page_bits;

	VC = 0;

	initProcesses();

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
		newPT = malloc( pt_byte_size );

		/* Clear the new table */
		memset(newPT, '\0', pt_byte_size);

		(*it)->PTptr = (sPTE*) malloc( sizeof(sPTE) * PT_Size);
	}

	cout << "Finished initializing process' virtual memory" << endl;

	return;
}

void cVMM::printResults() {
	cout << "Collecting/Recording results" << endl;
	string logName = EXTRACTP(string, Results,file);
	cout << "LogName: " << logName << endl;

	if ( logName.compare("") == 0 ) {
		cerr << "No results file indicated!!" << endl;
		return;
	}

	ofstream log;
	log.open(logName.c_str(), ios::out | ios::trunc);

	if ( !log.is_open() ) {
		cerr << "Error opening result log file " << logName << endl;
	}

	/* Gather all the potential opitons */
	dynamic_bitset<> options(1 << (sizeof(eResOptions) + 1));
	options.none();

	/* Global Options */
	options.set(G_CS, EXTRACTP(bool, Results, g_cs));
	options.set(G_PF, EXTRACTP(bool, Results, g_pf));
	options.set(G_ET, EXTRACTP(bool, Results, g_et));

	/* Local per-process options */
	options.set(L_CS, EXTRACTP(bool, Results, l_cs));
	options.set(L_PF, EXTRACTP(bool, Results, l_pf));
	options.set(L_ET, EXTRACTP(bool, Results, l_et));

	/* ---- Print Setting Info for Python Script ---- */
	log << toBinary(options.to_ulong()) << endl;
	log << PS << ":" <<numFrames << ":" << PRModule.name();
	log << ":" << EXTRACTP(string, Results,title) << endl;

	int g_cs, g_pf, g_et;
	g_cs = g_pf = g_et = 0;

	vector<sProc*>::iterator it;

	for ( it = procs.begin(); it != procs.end(); ++it ) {
		log << "Process " << (*it)->pid << ":" << endl;

		g_cs += (*it)->cswitches;
		if ( options.test(L_CS) )
			log << "Context Switches: " << (*it)->cswitches << endl;

		g_pf += (*it)->pageFaults;
		if ( options.test(L_PF) )
			log << "Page Faults: " << (*it)->pageFaults << endl;

		g_et += (*it)->clockTime;
		if ( options.test(L_ET) )
			log << "Execution Time: " << (*it)->clockTime << endl;

		log << endl; //For the python script to work
	}

	log << "\nGlobal VMM info:" << endl;
	if ( options.test(G_CS) )
		log << "Context Switches: " << g_cs << endl;

	if ( options.test(G_PF) )
		log << "Page Faults: " << g_pf << endl;

	if ( options.test(G_ET) )
		log << "Execution Time: " << g_et << endl;
}


int cVMM::start() {
	sProc* runningProc;

	uint8_t result;

	while ( scheduler.numProcesses() ) {
		cout << "Looping" << endl;

		runningProc = scheduler.getNextToRun();
		++(runningProc->clockTime);

		cpu.switchProc(runningProc);

		result = cpu.run();

		if ( result & CPU_TERM ) {
			scheduler.removeProcess(runningProc);
			runningProc = NULL;
		}
	}

	/* Gather simulation data and output it */
	printResults();

	/* ------------------------------------ */
	return 0;
}
