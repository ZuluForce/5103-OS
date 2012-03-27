#ifndef PR_LRU_H_INCLUDED
#define PR_LRU_H_INCLUDED

#include <list>
#include <queue>
#include <sstream>
#include "vmm_core.h"
#include <limits.h>

using namespace std;

class cPRLru: public cPRPolicy {
	private:
		//history of the pages
		list<sPTEOwner*> pageHist;
		queue<sPTEOwner*> pteowner_cache;

		cFrameAllocPolicy& FAPolicy;

		uint32_t PTSize;
		sPTEOwner* getPTEOwner();
		void returnPTEOwner(sPTEOwner* pteOwner);

	public:
		cPRLru(cFrameAllocPolicy& _FAPolicy);
		~cPRLru();

		const char* name() { return "lru"; };

		void resolvePageFault(sProc* proc, uint32_t page);

		void finishedQuanta(sProc* proc);

		void finishedIO(sProc* proc, sPTE* page);

		void clearPages(int numPages);

		void unpinFrame(uint32_t frame);

		void printTimestamps();

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

#endif // PR_LRU_H_INCLUDED
