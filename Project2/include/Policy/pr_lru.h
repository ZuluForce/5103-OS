#ifndef PR_LRU_H_INCLUDED
#define PR_LRU_H_INCLUDED

#include <list>
#include <queue>
#include <sstream>
#include "vmm_core.h"
#include <limits.h>

using namespace std;

/** Pure LRU PR Policy */
class cPRLru: public cPRPolicy {
	private:
		//history of the pages
		list<sPTEOwner*> pageHist;
		queue<sPTEOwner*> pteowner_cache;

		cFrameAllocPolicy& FAPolicy;

		uint32_t PTSize;

		/** Get a struct for holding both the PTE and Owner.
        *   If the pteowner_cache has an entry, re-use it. Otherwise
        *   malloc a new one. This helps with heap performance.
        */
		sPTEOwner* getPTEOwner();

		/** Add a PTEOwner to the pteowner_cache to use later.
		*/
		void returnPTEOwner(sPTEOwner* pteOwner);

	public:
		cPRLru(cFrameAllocPolicy& _FAPolicy);
		~cPRLru();

		const char* name() { return "lru"; };

        /** If no frames are free, spill the frame
        *   that has the lowest timestamp, or was "least recently
        *   used."
        */
		ePRStatus resolvePageFault(sProc* proc, uint32_t page);

        /** Since all the times are set in software,
        *   this method does nothing.
        */
		void finishedQuanta(sProc* proc);

        /** Add the PTE to the pageHist to keep track of
        *   which frames have been given out.
        */
		void finishedIO(sProc* proc, sPTE* page);

        /** Sorts pageHist by timestamp (increasing order) then
        *   removes numPages from the front of the list.
        */
		bool clearPages(int numPages);

		void unpinFrame(uint32_t frame);

        /** Used for debugging purposes.
        *   Prints all of the timestamps in the pageHist list.
        */
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
