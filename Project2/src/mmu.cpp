#include "mmu.h"

cMMU::cMMU(INIReader* settings) {
	/* Add default settings */
	settings->addDefault("MMU", "tlb_ize", "64");

	tlbSize = EXTRACTP(uint16_t, MMU, tlb_size);

	if ( tlbSize == 0 ) {
		cerr << "Warning, MMU started with tlb size 0" << endl;
	}

	TLB = (sTLBE*) malloc( sizeof(sTLBE) * tlbSize );
	flushTLB();

	replaceIndex = 0;

	/* ---- Print Init Info to Screen ---- */
	cout << "MMU initialized with settings:" << endl;
	cout << "\tTLB Size: " << tlbSize << endl;
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
