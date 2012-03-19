#ifndef CLEANINGDAEMON_H_INCLUDED
#define CLEANINGDAEMON_H_INCLUDED

#include "iniReader.h"
#include "Policy/frameAlloc.h"

using namespace std;

class cCleanDaemon {
	private:
		cFrameAllocPolicy& FAPolicy;

		uint32_t min_thresh;
		uint32_t clean_amnt;

	public:
		cCleanDaemon(cFrameAllocPolicy& _FA);
		~cCleanDaemon();

		uint32_t checkClean();
};

#include "vmm_core.h"

#endif // CLEANINGDAEMON_H_INCLUDED
