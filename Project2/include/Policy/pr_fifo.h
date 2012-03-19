#ifndef PR_FIFO_H_INCLUDED
#define PR_FIFO_H_INCLUDED

#include <queue>
#include <sstream>
#include "vmm_core.h"

using namespace std;

class cPRFifo: public cPRPolicy {
	private:
		//FIFO history of the pages
		queue<sPTE*> pageHist;
		queue<unsigned int> pageOwners;

		cFrameAllocPolicy& FAPolicy;

		uint32_t PTSize;

	public:
		cPRFifo(cFrameAllocPolicy& _FAPolicy);
		~cPRFifo();

		const char* name() { return "fifo"; };

		void resolvePageFault(sProc* proc, uint32_t page);

		void finishedQuanta(sProc* proc);

		void finishedIO(sProc* proc, sPTE* page);

		void clearPages(int numPages);

		void unpinFrame(uint32_t frame);
};

#endif // PR_FIFO_H_INCLUDED
