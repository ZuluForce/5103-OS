#include "vmm_core.h"

cVMM::cVMM(INIReader* _settings, vector<sProc*>& _procs)
: procs(_procs), cpu(_settings) {
	settings = _settings;

	settings->addDefault("Global", "page_bits", "20");
	settings->addDefault("Global", "offset_bits", "12");
	settings->addDefault("Global", "total_frames", "100");

	/* Now get the actual values. Reverting to
	 * defaults if no others are available
	 */
	int page_bits = EXTRACTP(int,Global,page_bits);
	int off_bits = EXTRACTP(int,Global,offset_bits);
	numFrames = EXTRACTP(uint32_t, Global, total_frames);

	/* ---- Print Init Info to Screen ---- */
	cout << "Virtual Memory Manager (VMM) started with settings:" << endl;
	cout << "\tPage Bits: " << page_bits << endl;
	cout << "\tOffset Bits: " << off_bits << endl;
	cout << "\t# Frames (Global): " << numFrames << endl;
	cout << "\t# Processes: " << procs.size() << endl;

	return;
}

cVMM::~cVMM() {

	return;
}

int cVMM::start() {

	return 0;
}
