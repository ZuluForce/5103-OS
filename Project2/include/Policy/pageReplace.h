#ifndef PAGEREPLACE_H_INCLUDED
#define PAGEREPLACE_H_INCLUDED

#include "iniReader.h"
#include "utility/logger.h"
#include "Policy/frameAlloc.h"

class cPRPolicy {
	private:

	public:
		cPRPolicy();
		cPRPolicy(cFrameAllocPolicy&);

		/** Name of PR module
		 *
		 *	This is only used for logging purposes so
		 *	that the PR module can be easily identified.
		 */
		virtual const char* name() = 0;

		/** Get a page frame for this process
		 *
		 *	If a free one is available from the
		 *	frame allocator it will be used. Otherwise,
		 *	a page will have to be spilled.
		 *
		 *	@param sProc* A pointer to the process that the
		 *	frame is being requested for.
		 *
		 *	@param uint32_t page The page for the given process
		 *	that needs to be fetched.
		 */
		virtual void resolvePageFault(sProc* proc, uint32_t page) = 0;

		virtual void finishedQuanta(sProc*) = 0;

		virtual void finishedIO(sProc*, sPTE*) = 0;

		virtual void clearPages(int numPages) = 0;

		virtual void unpinFrame(uint32_t frame) = 0;
};

/** @fn cPRPolicy::finishedQuanta(sProc*)
 *	This is called after an execution quanta finishes. This gives the
 *	PR module a chance to update its internal data-structures using
 *	the reference bits or any other data available from the process
 *	struct.
 */

/** @fn cPRPolicy::finishedIO(sProc*, sPTE*)
 *
 *	This is called after an I/O (input) completes. This is another
 *	notification point for the PR module to update its data structs.
 */

/** @fn cPRPolicy::clearPages(vector<sPTE*>&)
 *
 *	This is currently only called by the cleaning daemon to notify the
 *	PR module that it has cleaned out this vector of pages and it should
 *	update its structs. The PR module need not call the FA module to release
 *	the frames that these pages occupied because the calling party (cleaning daemon)
 *	will do that.
 *
 *	@param vector<sPTE*>& A vector of page table entries that have been
 *	cleared from main memory.
 */


/** @fn cPRPolicy::unpinFrame(uint32_t frame)
 *	This is typically called after ::finishedIO so that the PR module
 *	unpins the frame in the frame allocation module.
 */

#endif // PAGEREPLACE_H_INCLUDED
