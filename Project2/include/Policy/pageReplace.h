#ifndef PAGEREPLACE_H_INCLUDED
#define PAGEREPLACE_H_INCLUDED

/** @file */

#include "iniReader.h"
#include "utility/logger.h"
#include "Policy/frameAlloc.h"

enum ePRStatus {
	PR_SERVICED,	/**< Page fault serviced */
	PR_SERVICED_IO,	/**< Page fault serviced with I/O required */
	PR_NO_AVAIL,	/**< Page fault not serviced. No frames available (all pinned) */
	PR_NO_ACTION,	/**< Used in returning from circular page fault handler. */
};
//PR_NO_AVAIL won't happen unless you have 1-2 global frames

/** Abstract Page Replacement Policy */
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
		virtual ePRStatus resolvePageFault(sProc* proc, uint32_t page) = 0;
		ePRStatus resolveCircularPF(sProc* proc, uint32_t page) { return PR_NO_ACTION; };
		virtual void finishedQuanta(sProc*) = 0;
		virtual void finishedIO(sProc*, sPTE*) = 0;
		virtual bool clearPages(int numPages) = 0;
		virtual void unpinFrame(uint32_t frame) = 0;
		virtual void returnFrame(uint32_t frame) = 0;
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

/** @fn cPRPolicy::clearPages(int)
 *	Remove this many pages from memory.
 *
 *	If the cleaning daemon decides that pages need to be cleaned it will
 *	determine how many from its policy and call the PR module to clean
 *	that many. This method keeps the PR policy separate from the decision
 *	on how many pages to clean and when.
 *
 *	@param int Number of pages to clear.
 */


/** @fn cPRPolicy::unpinFrame(uint32_t)
 *	This is typically called after ::finishedIO so that the PR module
 *	unpins the frame in the frame allocation module.
 */

/** @fn cPRPolicy::returnFrame(uint32_t)
 *	Return this frame to the system.
 *
 *	This is called when a process exits and its frames are being
 *	returned to the system. This is usually a wrapper to the
 *	FA policy return frame function but it gives the PR module
 *	a chance to update its datastructures if necessary.
 *
 *	Since we have made the assumption that this is only called on
 *	process exit, it is not necessary to do any I/O on the page.
 *
 *	@param uint32_t Number of the frame to clear/return.
 */
#endif // PAGEREPLACE_H_INCLUDED
