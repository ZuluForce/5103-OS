#ifndef PR_FIFO_H_INCLUDED
#define PR_FIFO_H_INCLUDED

#include <queue>
#include "vmm_core.h"

using namespace std;

class cPRFifo: public cPRPolicy {
	private:
		//FIFO history of the pages
		queue<sPTE*> pageHist;
		queue<unsigned int> pageOwners;

		cFrameAllocPolicy& FAPolicy;

	public:
		cPRFifo(cFrameAllocPolicy& _FAPolicy);
		~cPRFifo();

		const char* name() { return "fifo"; };

		void getPage(sProc* proc, uint32_t page);

		void unpinFrame(uint32_t frame);
};

#endif // PR_FIFO_H_INCLUDED
