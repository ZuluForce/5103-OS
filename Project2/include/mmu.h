#ifndef MMU_CPP_INCLUDED
#define MMU_CPP_INCLUDED

/** @file */
#include <cassert>
#include <string.h>
#include <inttypes.h>

#include "data_structs.h"
#include "iniReader.h"

/** State of the MMU after translation (or attempted) */
enum eMMUstate {
	MMU_THIT,	/**< TLB Hit */
	MMU_TMISS,	/**< TLB Miss */
	MMU_PF		/**< Page Fault */
};

/** Struct representing a single entry in the TLB */
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

/** Class representing an MMU in the CPU */
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

/** @fn cMMU::addTLB(uint32_t,uint32_t,bool)
 *	Add an entry to the TLB
 */

/** @fn cMMU::setPTBR(sPTE*)
 *	Set the Page Table Base Register
 *
 *	This needs to be reset on context switch
 */

/** @fn cMMU::addVC(int*)
 *	Add a refernece to the main VCC for timekeeping purposes
 */

/** @fn cMMU::flushTLB(bool)
 *	Flush the tlb and optionally sync values back to page table
 *
 *	On context switch this is called to clear all the entries by
 *	simply setting their valid bit to false. If the sync flag is
 *	set and a page is dirty/referenced then this information is
 *	written back to the main page table.
 *
 *	This syncing is usually not necessary since the cpu calls sync
 *	on a page fault or quanta terminnation so that the table is
 *	up-to-date for the PR module to make decisions.
 */

/** @fn cMMU::syncTLB()
 *	Sync TLB entries without flushing them
 *
 *	This is the same as ::fulshTLB with the exception of not flushing/
 *	invalidating the table entries. This is useful when an execution
 *	quanta finishes so you want to sync the entries but not flush them
 *	in-case it is the only process left.
 */

/** @fn cMMU::checkStatus()
 *	What is the MMU status after the last translation
 *
 *	Since the getAddr function returns the translated address. With the
 *	exception of doing some bit packing we can't return the status in the
 *	same variable.
 */

/** @fn cMMU::getFaultPage()
 *	What was the VPN that caused the fault
 *
 *	The cpu calls this to send back to the VMM core so it can handle it
 *	accordingly.
 */

#endif // MMU_CPP_INCLUDED
