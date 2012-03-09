#include "cpu.h"

cCPU::cCPU(INIReader* settings)
: mmu(settings) {
	/* Set up any default settings */
	settings->addDefault("CPU","exec_quanta", "1");

	/* Extract any settings */
	quanta = EXTRACTP(int, CPU, exec_quanta);

	curProc = NULL;

	/* ---- Print Init Info to Screen ---- */
	cout << "CPU initialized with following parameters:" << endl;
	cout << "\tExecution Quanta: " << quanta << endl;
	/* --------------------------------- */

	return;
}

cCPU::~cCPU() {

}

void cCPU::switchProc(sProc* newProc) {
	mmu.flushTLB();

	mmu.setPTBR(newProc->PTptr);

	curProc = newProc;
}
