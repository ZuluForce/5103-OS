#ifndef PR_LRUAPPROX_H_INCLUDED
#define PR_LRUAPPROX_H_INCLUDED

#include <list>
#include <queue>
#include <sstream>
#include "vmm_core.h"

using namespace std;

class cPRLruApprox: public cPRPolicy {
	private:
		//history of the pages
		list<sPTEOwner*> pageHist;
		queue<sPTEOwner*> pteowner_cache;

		cFrameAllocPolicy& FAPolicy;

		uint32_t PTSize;

		void updateTime();
		sPTEOwner* getPTEOwner();

	public:
		cPRLruApprox(cFrameAllocPolicy& _FAPolicy);
		~cPRLruApprox();

		const char* name() { return "lru_approx"; };

		void resolvePageFault(sProc* proc, uint32_t page);

		void finishedQuanta(sProc* proc);

		void finishedIO(sProc* proc, sPTE* page);

		void clearPages(int numPages);

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

#endif // PR_LRUAPPROX_H_INCLUDED
