#include "mmu.h"

extern INIReader* settings;

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

	return;
}

uint32_t cMMU::getAddr(string& sVA, bool write) {
	ssAddr.str("");

	if ( hexAddr )
		ssAddr << std::hex << sVA;
	else
		ssAddr << sVA;

	uint32_t VA;
	ssAddr >> VA;

	uint32_t VPN = VA >> off_bits;

	cout << "Request for VPN: " << VPN << endl;

	return MMU_THIT;
}
