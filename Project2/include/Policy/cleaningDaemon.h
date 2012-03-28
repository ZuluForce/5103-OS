#ifndef CLEANINGDAEMON_H_INCLUDED
#define CLEANINGDAEMON_H_INCLUDED

/** @file */

#include "iniReader.h"
#include "Policy/frameAlloc.h"

using namespace std;

/** Cleaning Daemon for system */
class cCleanDaemon {
	private:
		cFrameAllocPolicy& FAPolicy;

		uint32_t min_thresh;
		uint32_t clean_amnt;

	public:
		cCleanDaemon(cFrameAllocPolicy& _FA);
		~cCleanDaemon();

		/** Check if pages need to be cleaned
		 *
		 *	The VMM core calls this and the daemon checks
		 *	with the FA module to see how many frames are
		 *	available. If it is below its threshold it returns
		 *	how many should be cleaned. This value is passed
		 *	to the PR module which makes the policy decisions
		 *	on which to replace.
		 */
		uint32_t checkClean();
};

#include "vmm_core.h"

#endif // CLEANINGDAEMON_H_INCLUDED
