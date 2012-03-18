#include "mmu.h"

extern INIReader* settings;
extern FILE* logStream;

cMMU::cMMU() {
	tlbSize = EXTRACTP(uint16_t, MMU, tlb_size);
	off_bits = EXTRACTP(int,Global,off_bits);
	string addr_type = EXTRACTP(string,MMU,addr_type);
	if ( addr_type.compare("hex") == 0 )
		hexAddr = true;
	else if ( addr_type.compare("decimal") == 0 )
		hexAddr = false;

	if ( tlbSize == 0 ) {
		cerr << "Warning, MMU started with tlb size 0" << endl;
	}

	TLB = (sTLBE*) malloc( sizeof(sTLBE) * tlbSize );
	flushTLB();

	replaceIndex = 0;

	pageSize = (1 << off_bits);

	/* ---- Print Init Info to Screen ---- */
	cout << "MMU initialized with settings:" << endl;
	cout << "\tTLB Size: " << tlbSize << endl;
	cout << "\tPage Size: " << (1 << off_bits) << endl;
	cout << "\tAddress Type: " << (hexAddr ? "hex" : "decimal") << endl;
	/* ----------------------------------- */

	return;
}

cMMU::~cMMU() {
	free(TLB);
}

void cMMU::flushTLB() {
	/* Flush it */
	for ( int i = 0; i < tlbSize; ++i) {
		TLB[i].valid = false;
	}

	replaceIndex = 0;

	return;
}

void cMMU::addTLB(uint32_t VPN, uint32_t frame) {
	TLB[replaceIndex].valid = true;
	TLB[replaceIndex].VPN = VPN;
	TLB[replaceIndex].frame = frame;

	replaceIndex = ++replaceIndex % tlbSize;

	return;
}

uint32_t cMMU::getAddr(string& sVA, bool write) {
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
			uint32_t FN = TLB[i].frame;

			PA = (FN * pageSize) + offset;
			cout << "Physical Address: " << PA << endl;

			mmu_status = MMU_THIT;
			cout << "***TLB_HIT***" << endl;
			fprintf(logStream, "MMU: *TLB HIT* PA=%d\n", PA);

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
		addTLB(VPN, ptbr[VPN].frame);

		return PA;
	}

	faultPage = VPN;
	mmu_status = MMU_PF;

	return 0;
}
