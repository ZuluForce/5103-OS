#ifndef PAGEREPLACE_H_INCLUDED
#define PAGEREPLACE_H_INCLUDED

#include "iniReader.h"
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
		 *	@return uint32_t The page that is now available for
		 */
		uint32_t getPage(sProc* proc);

		int pinPage(uint32_t page);

};

#endif // PAGEREPLACE_H_INCLUDED
