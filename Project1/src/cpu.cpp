#include "cpu.h"

cCPU::cCPU() {

	return;
}

cCPU::~cCPU() {

	return;
}

unsigned int cCPU::getSetPC(unsigned int newPC) {
	unsigned int oldPC = PC;
	PC = newPC;
	return oldPC;
}

int cCPU::getSetVC(int newVC) {
	int oldVC = VC;
	VC = newVC;
	return newVC;
}

void cCPU::run() {
	char *instr;
	char Opcode;

	while (true) {
		/* Get the Opcode */
		Opcode = execText[PC];

		switch (Opcode) {
			case 'S':
				/* 'S x': Store x to VC */

				break;

			case 'A':
				/* 'A x': Add x to VC */

				break;

			case 'D':
				/* 'D x': Decrement x from VC */
				break;

			case 'C':
				/* Call the OS to start new process: 'C <priority> <filename>'*/
				break;

			case 'I':
				/* IO syscall to device class: 'I <dev-class> (B/C) */

				break;

			case 'P':
				/* Priveleged instruction */
				if ( !KMode ) {
					/* Exception!, notify the OS */
					;
				}
				/* Else it is a NoOp */
				break;

			case 'E':
				/* Terminate the process. Notify the OS */

				break;

			default:
				/* Invalid Instruction, Notify the OS */

				break;
		}


	}
}
