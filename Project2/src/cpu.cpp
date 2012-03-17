#include "cpu.h"

extern INIReader* settings;

cCPU::cCPU() {

	curProc = NULL;

	/* ---- Print Init Info to Screen ---- */

	/* --------------------------------- */

	return;
}

cCPU::~cCPU() {

}

void cCPU::switchProc(sProc* newProc) {
	if (curProc && curProc != newProc )
		++(curProc->cswitches);

	mmu.flushTLB();

	mmu.setPTBR(newProc->PTptr);

	curProc = newProc;
}

uint8_t cCPU::run() {

	string line;
	if ( !std::getline(*curProc->data, line) ) {
		cout << "Process ran out of data" << endl;
		return CPU_TERM;
	}

	//If so it can't possibly be valid
	if ( line.length() < 3 ) {
		return CPU_EX;
	}

	opCode = line[0];
	addr = line.substr(2);

	cout << "opCode: " << opCode << endl;
	cout << "Address: " << addr << endl;

	//if ( opCode[0] == 'R' )
		//mmu.getAddr(addr.c_str(), false);
	//else if ( opCode[1] == 'W' )
	//	mmu.getAddr(addr.c_str(), true);
	//else
	//	return CPU_EX;

	return CPU_OK;
}
