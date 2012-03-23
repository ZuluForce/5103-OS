#ifndef MMU_CPP_INCLUDED
#define MMU_CPP_INCLUDED

#include <cassert>
#include <string.h>
#include <inttypes.h>

#include "data_structs.h"
#include "iniReader.h"


enum eMMUstate {
	MMU_THIT,	/**< TLB Hit */
	MMU_TMISS,	/**< TLB Miss */
	MMU_PF		/**< Page Fault */
};

/**< Struct representing a single entry in the TLB */
struct sTLBE {
	uint32_t VPN;
	uint32_t frame;

	int timestamp;

	/** Is this a valid translation?
	 *
	 *	If false, the MMU will ignore any cache
	 *	hits on this entry. This is also used in
	 *	flushing the tlb.
	 */
	bool valid;
	bool dirty;
	bool ref;
};

class cMMU {
	private:
		sTLBE* TLB;
		uint16_t tlbSize;
		uint32_t pageSize;
		uint32_t off_bits;
		bool hexAddr;

		stringstream ssAddr;

		/** For replacing old tlb entries we are simply
		 *	treating it as a ring buffer. This is the index
		 *	where new entries are being placed */
		uint32_t replaceIndex;

		int* VC;

		/* -- Page Table Info --*/
		sPTE* ptbr;

		/* These will be collected during cleanup to
		 * provide the appropriate statistics.
		 */
		int tlb_hits;
		int tlb_misses;

		eMMUstate mmu_status;

		uint32_t faultPage;

		void addTLB(uint32_t, uint32_t, bool);

	public:
		cMMU();
		~cMMU();

		void setPTBR(sPTE* _ptbr) { ptbr = _ptbr; };

		void addVC(int* VC);

		void flushTLB(bool sync = true);
		void syncTLB();

		eMMUstate checkStatus() { return mmu_status; };
		uint32_t getFaultPage() { return faultPage; };
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
		uint32_t getAddr(string& sVA, bool isWrite);
};

#endif // MMU_CPP_INCLUDED
