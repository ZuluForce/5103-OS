#include "vmm_core.h"

cVMM::cVMM(INIReader* _settings, vector<sProc*>& _procs)
: procs(_procs) {
	settings = _settings;

	settings->addDefault("Global", "page_bits", "20");
	settings->addDefault("Global", "offset_bits", "12");
	settings->addDefault("Global", "total_frames", "100");

	/* Now get the actual values. Reverting to
	 * defaults if no others are available
	 */
	int page_bits = EXTRACTP(int,Global,page_bits);
	int off_bits = EXTRACTP(int,Global,offset_bits);

	cout << "VMM started with " << procs.size() << " processes" << endl;

	return;
}

cVMM::~cVMM() {

	return;
}
