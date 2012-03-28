#ifndef PR_FIFO_H_INCLUDED
#define PR_FIFO_H_INCLUDED

#include <queue>
#include <sstream>
#include "vmm_core.h"

using namespace std;

/** FIFO PR Policy */
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

		ePRStatus resolvePageFault(sProc* proc, uint32_t page);

		void finishedQuanta(sProc* proc);

		void finishedIO(sProc* proc, sPTE* page);

		bool clearPages(int numPages);

		void unpinFrame(uint32_t frame);

		/** Return a frame to the system.
		 *
		 *	The VMM core calls this when a process termintates
		 *	to free up any of its used frames. This call could
		 *	be made directly to the frame allocator but using
		 *	the PR module as a middle man gives it a chance to
		 *	update as needed.
		 */
		void returnFrame(uint32_t frame);
};

#endif // PR_FIFO_H_INCLUDED
