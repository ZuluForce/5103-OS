#include "Policy/cleaningDaemon.h"

extern INIReader* settings;
extern cVMM* VMMCore;
extern FILE* logStream;

cCleanDaemon::cCleanDaemon(cFrameAllocPolicy& _FA)
: FAPolicy(_FA) {

	min_thresh = EXTRACTP(int, Policy, clean_min);
	clean_amnt = EXTRACTP(int, Policy, cleanup_amnt);

	return;
}

cCleanDaemon::~cCleanDaemon() {

	return;
}

uint32_t cCleanDaemon::checkClean() {
	if ( FAPolicy.checkOpen(true) < min_thresh ) {
		/* Time for some cleanup */
		fprintf(logStream, "\n###-------- Cleaning Daemon Starting --------###\n");

		return clean_amnt;
	}

	return 0;
}
