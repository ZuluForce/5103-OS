#include "cpu.h"

extern INIReader* settings;
extern cVMM* VMMCore;
extern FILE* logStream;

cCPU::cCPU() {

	curProc = NULL;

	/* ---- Print Init Info to Screen ---- */
	instr_time = EXTRACTP(int, Timings, instr_time);
	cs_time = EXTRACTP(int, Timings, cs_time);
	/* --------------------------------- */

	cout << "CPU initialized with: " << endl;
	cout << "\tinstruction time = " << instr_time << " units" << endl;
	cout << "\tContext Switch time = " << cs_time << " units" << endl << endl;

	return;
}

cCPU::~cCPU() {

}

void cCPU::addVC(int* VC) {
	this->VC = VC;
}

void cCPU::incVC(int amnt) {
	*VC += amnt;

	VMMCore->tickController(amnt);

	fprintf(logStream, "-*-Virtual Counter: %d-*-\n", *VC);
}

void cCPU::switchProc(sProc* newProc) {

	/* This means there is currently a valid process in the cpu
	 * and it is being replaced by a new valid process */
	if (curProc && newProc && (curProc != newProc) ) {
		cout << "Context switching process " << curProc->pid << " for process " << newProc->pid << endl;
		fprintf(logStream, "Switching out Process %d for Process %d\n", curProc->pid, newProc->pid);
		++(curProc->cswitches);

		mmu.flushTLB();

		mmu.setPTBR(newProc->PTptr);

		curProc = newProc;

		incVC(cs_time);
	} else if ( !curProc && newProc) {
		/* This means nothing was on the cpu when newProc was
		 * requested to be put on. In this case don't add anything
		 * for a context switch.
		 */
		curProc = newProc;

		mmu.flushTLB();
		mmu.setPTBR(newProc->PTptr);
	}

	/* If neither of those ifs were true then the VMM core must
	 * be looping waiting for a process to become unblocked and
	 * therefore newProc == NULL
	 */

	return;
}

uint32_t cCPU::getFaultPage() {
	return mmu.getFaultPage();
}

uint8_t cCPU::run() {

	string line;
	if ( curProc->restart ) {
		line = curProc->rline;
		curProc->restart = false;
	} else if ( !std::getline(*curProc->data, line) ) {
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

	fprintf(logStream, "CPU: Opcode:%s  Virtual-Address:%s  PID:%d\n", opCode.c_str(), addr.c_str(), curProc->pid);
	incVC(instr_time);
	uint32_t VA;

	if ( opCode[0] == 'R' ) {
		VA = mmu.getAddr(addr, false);
	} else if ( opCode[0] == 'W' ) {
		VA = mmu.getAddr(addr, true);
	} else {
		return CPU_EX;
	}

	/* Check for page fault */
	eMMUstate status = mmu.checkStatus();
	if ( status == MMU_PF ) {
		/* Save instruction */
		curProc->restart = true;
		curProc->rline = line;

		return CPU_PF;
	}

	return CPU_OK;
}
