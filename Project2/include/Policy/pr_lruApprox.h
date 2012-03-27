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

		/** Get a struct for holding both the PTE and Owner.
        *   If the pteowner_cache has an entry, re-use it. Otherwise
        *   malloc a new one. This helps with heap performance.
        */
		sPTEOwner* getPTEOwner();

		/** Add a PTEOwner to the pteowner_cache to use later.
		*/
		void returnPTEOwner(sPTEOwner* pteOwner);

	public:
		cPRLruApprox(cFrameAllocPolicy& _FAPolicy);
		~cPRLruApprox();

		const char* name() { return "lru_approx"; };

        /** If no frames are free, spill the frame that has the lowest
        *   shifted time. This aging algorithm simulates LRU.
        */
		ePRStatus resolvePageFault(sProc* proc, uint32_t page);

        /** Update the approx time field in software.
        */
		void finishedQuanta(sProc* proc);

        /** I/O is finished so add the PTE to the pageHist so we
        *   know what frames are in use.
        */
		void finishedIO(sProc* proc, sPTE* page);

        /** Clears pages by sorting the pageHist by the time field (increasing).
        *   Then removes numPages from the front of pageHist.
        */
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

/** @fn cPRLruApprox::updateTime()
*   Updates the time field by first shifting the time field to the right and then
*   adding a 1 in the far left position if the ref bit is set.
*   This function is called on a page fault or after a quanta is finished.
*/



#endif // PR_LRUAPPROX_H_INCLUDED
