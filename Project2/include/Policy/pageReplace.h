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
		virtual void getPage(sProc* proc, uint32_t page) = 0;

		virtual void unpinFrame(uint32_t frame) = 0;
};

#endif // PAGEREPLACE_H_INCLUDED
