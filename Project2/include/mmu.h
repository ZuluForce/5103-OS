#ifndef MMU_CPP_INCLUDED
#define MMU_CPP_INCLUDED

#include <inttypes.h>

#include "data_structs.h"
#include "iniReader.h"

enum eMMUstate {
	MMU_THIT,				/**< TLB Hit */
	MMU_TMISS,	/**< TLB Miss */
	MMU_PF		/**< Page Fault */
};

/**< Struct representing a single entry in the TLB */
struct sTLBE {
	uint32_t VPN;
	uint32_t frame;

	/** Is this a valid translation?
	 *
	 *	If false, the MMU will ignore any cache
	 *	hits on this entry. This is used in flushing
	 *	the tlb.
	 */
	bool valid;
};

class cMMU {
	private:
		sTLBE* TLB;
		uint16_t tlbSize;
		uint32_t off_bits;
		bool hexAddr;

		stringstream ssAddr;

		/** For replacing old tlb entries we are simply
		 *	treating it as a ring buffer. This is the index
		 *	where new entries are being placed */
		uint32_t replaceIndex;

		/* -- Page Table Info --*/
		sPTE* ptbr;

		/* These will be collected during cleanup to
		 * provide the appropriate statistics.
		 */
		int tlb_hits;
		int tlb_misses;

		eMMUstate mmu_status;

	public:
		cMMU();
		~cMMU();

		void setPTBR(sPTE* _ptbr) { ptbr = _ptbr; };

		void flushTLB();

		/** Translate virtual to physical address
		 *
		 *	This method checks the tlb and page table
		 *	simultaneously and returns the appropriate
		 *	address. Whoever receives this should first
		 *	check the mmu status to make sure it is valid.
		 *
		 *	@param bool write If set, the mmu will mark this
		 *	page as dirty.
		 */
		uint32_t getAddr(string& sVA, bool write);
};

#endif // MMU_CPP_INCLUDED
