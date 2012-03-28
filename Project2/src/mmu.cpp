#include "mmu.h"

extern INIReader* settings;
extern FILE* logStream;

cMMU::cMMU() {
	tlbSize = EXTRACTP(uint16_t, MMU, tlb_size);
	off_bits = EXTRACTP(int,Global,offset_bits);

	string addr_type = EXTRACTP(string,MMU,addr_type);
	if ( addr_type.compare("hex") == 0 )
		hexAddr = true;
	else if ( addr_type.compare("decimal") == 0 )
		hexAddr = false;

	if ( tlbSize == 0 ) {
		cerr << "Warning, MMU started with tlb size 0" << endl;
	}

	ptbr = NULL;

	TLB = (sTLBE*) malloc( sizeof(sTLBE) * tlbSize );
	memset(TLB, '\0', tlbSize * sizeof(sTLBE));
	//flushTLB();

	replaceIndex = 0;

	pageSize = (1 << off_bits);

	/* ---- Print Init Info to Screen ---- */
	cout << "MMU initialized with settings:" << endl;
	cout << "\tTLB Size: " << tlbSize << endl;
	cout << "\tPage Size: " << (1 << off_bits) << endl;
	cout << "\tAddress Type: " << (hexAddr ? "hex" : "decimal") << endl;

	fprintf(logStream, "MMU initialized with settings: \n");
	fprintf(logStream, "TLB Size: %d\n", tlbSize);
	fprintf(logStream, "Page Size: %d\n", pageSize);
	fprintf(logStream, "Address Type: %s\n\n", (hexAddr ? "hex" : "decimal"));

	/* ----------------------------------- */

	return;
}

cMMU::~cMMU() {
	free(TLB);
}

void cMMU::addVC(int* VC) {
	this->VC = VC;
}

void cMMU::flushTLB(bool sync) {
	/* Flush it */
	for ( int i = 0; i < tlbSize; ++i) {
		if ( sync && TLB[i].valid && (TLB[i].dirty || TLB[i].ref)) {
			assert(ptbr != NULL); //Remove this before handing in.
			cout << "Syncing TLB entry " << i << " back to page table (page:" << TLB[i].VPN << ")" << endl;
			ptbr[TLB[i].VPN].flags[FI_DIRTY] = TLB[i].dirty;
			ptbr[TLB[i].VPN].flags[FI_REF] = TLB[i].ref;
			ptbr[TLB[i].VPN].timestamp = TLB[i].timestamp;
		}

		TLB[i].valid = false;
		TLB[i].dirty = false;
		TLB[i].ref = false;
		TLB[i].timestamp = 0;
	}

	replaceIndex = 0;

	return;
}

void cMMU::syncTLB() {
	assert( ptbr != NULL);

	for ( int i = 0; i < tlbSize; ++i) {
		if ( TLB[i].valid && (TLB[i].dirty || TLB[i].ref) ) {
			cout << "Syncing TLB entry " << i << " to the page table (page:" << TLB[i].VPN << ")" << endl;
			ptbr[TLB[i].VPN].flags[FI_DIRTY] = TLB[i].dirty;
			ptbr[TLB[i].VPN].flags[FI_REF] = TLB[i].ref;
			ptbr[TLB[i].VPN].timestamp = TLB[i].timestamp;

			//TLB[i].dirty = false;
			//TLB[i].ref = false;
		}
	}

	return;
}

void cMMU::addTLB(uint32_t VPN, uint32_t frame, bool isWrite) {
	assert(TLB != NULL);
	assert(ptbr != NULL);

	sTLBE* replaceEntry = TLB + replaceIndex;
	/* Sync the old entry with the page table */
	if ( replaceEntry->valid ) {
		ptbr[replaceEntry->VPN].flags[FI_DIRTY] = replaceEntry->dirty;
		//ptbr[replaceEntry.frame].flags[FI_REF] = ...;
	}

	//TLB[replaceIndex].valid = true;
	//TLB[replaceIndex].VPN = VPN;
	//TLB[replaceIndex].frame = frame;

	//TLB[replaceIndex].dirty = isWrite ? true : false;

	replaceEntry->valid = true;
	replaceEntry->VPN = VPN;
	replaceEntry->frame = frame;
	replaceEntry->dirty = isWrite ? true : false;
	replaceEntry->ref = true;
	replaceEntry->timestamp = *VC;

	replaceIndex = ++replaceIndex % tlbSize;

	return;
}

uint32_t cMMU::getAddr(string& sVA, bool isWrite) {
	/* Make sure the stream is empty */
	ssAddr.str(std::string());
	ssAddr.clear(); //This is necessary to reset eof bit in stream

	if ( hexAddr )
		ssAddr << std::hex << sVA;
	else
		ssAddr << sVA;

	uint32_t VA = 0;
	ssAddr >> VA;

	//cout << "VA = " << VA << endl;

	//uint32_t VPN = VA >> off_bits;
	uint32_t VPN = VA / pageSize;
	uint32_t offset = VA % pageSize;

	cout << "Request for VPN: " << VPN << endl;
	cout << "Offset: " << offset << endl;

	fprintf(logStream, "MMU: VPN=%d  Offset=%d\n", VPN, offset);

	/* Check TLB/PT */
	/* The writeup says to do these in "parallel" so I don't know if that
	 * means we should literally write this threaded or just pretend for
	 * process accounting purposes that it was done in parallel.
	 */

	uint32_t PA = 0; //Physical address

	for (int i = 0; i < tlbSize; ++i) {
		if (TLB[i].VPN == VPN && TLB[i].valid) {
			if ( !ptbr[VPN].flags[FI_PRESENT] ) {
				/* While this sort of defeats the purpose of the
				 * tlb if we only have one process we don't flush
				 * the tlb however if a page is removed we don't
				 * want its tlb entry to be valid anymore (if it is there).
				 *
				 * Rather than creating a more complicated mechanism to
				 * propagate this information back we just check it here.
				 */
				TLB[i].valid = false;

				faultPage = VPN;
				mmu_status = MMU_PF;
				return 0;
			}

			uint32_t FN = TLB[i].frame;

			PA = (FN * pageSize) + offset;
			cout << "Physical Address: " << PA << endl;

			mmu_status = MMU_THIT;
			cout << "***TLB_HIT***" << endl;
			fprintf(logStream, "MMU: *TLB HIT* Frame=%d PA=%d\n", FN, PA);

			TLB[i].dirty = isWrite ? true : false;
			TLB[i].ref = true;
			TLB[i].timestamp = *VC;

			++tlb_hits;
			return PA;
		}
	}

	++tlb_misses;
	/* Do a page walk. Not much of a walk when it is 1-level */
	if ( ptbr[VPN].flags[0] ) {
		mmu_status = MMU_TMISS;

		PA = (ptbr[VPN].frame * pageSize) + offset;

		cout << "***TLB_MISS/Page Walk***" << endl;
		fprintf(logStream, "MMU: *TLB MISS* Found Frame=%d  PA=%d\n", ptbr[VPN].frame, PA);

		/* Save in TLB */
		addTLB(VPN, ptbr[VPN].frame, isWrite);

		return PA;
	}

	faultPage = VPN;
	mmu_status = MMU_PF;

	return 0;
}
