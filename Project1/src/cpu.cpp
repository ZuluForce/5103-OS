#include "cpu.h"

cCPU::cCPU() {

	return;
}

cCPU::~cCPU() {

	return;
}

int cCPU::tokenizeLine() {
	char c = execText[PC];

	//Strip all whitespace
	while ( c == ' ' ) {
		c = execText[++PC];
	}

	int bufferIndex = 0, count = 0;

	while ( count < MAX_PARAMS && bufferIndex < MAX_PARAM_SIZE ) {
		if ( c == ' ' ) {
			//This only happens in poorly formated lines ie. 'C <priority> <filename>   '
			count = bufferIndex > 0 ? count + 1 : count;
			tokenBuffer[count][bufferIndex] = '\0';
			bufferIndex = 0;
			c = execText[++PC];
			continue;
		} else if ( c == '\n' ) {
			//End of the line
			count = bufferIndex > 0 ? count + 1 : count;
			tokenBuffer[count][bufferIndex] = '\0';
			++PC; //Bring it around to the next line
			break;
		} else if ( c == '\0' ) {
			//End of the program
			count = bufferIndex > 0 ? count + 1 : count;
			tokenBuffer[count][bufferIndex] = '\0';
			break;
		}
		tokenBuffer[count][bufferIndex++] = c;

		c = execText[++PC];
	}

	return count;
}

void cCPU::setText(char *text) {
	assert( text != NULL );
	execText = text;

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

uint16_t cCPU::getPSW() {
	return PSW;
}

char* cCPU::getParam(int num) {
	if ( num < MAX_PARAMS )
		return tokenBuffer[num];

	return NULL;
}

void cCPU::run() {
	int arg; //Used for storing opcode parameters

	while (true) {
		/* Get the Opcode */
		Opcode = execText[PC];

		switch (Opcode) {
			case 'S':
				/* 'S x': Store x to VC */
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
				}

				arg = atoi(tokenBuffer[0]);
				printf("Storing %d to VC\n", arg);
				VC = arg;

				break;

			case 'A':
				/* 'A x': Add x to VC */
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
				}

				arg = atoi(tokenBuffer[0]);
				printf("Adding %d to VC\n", arg);
				VC += arg;

				break;

			case 'D':
				/* 'D x': Decrement x from VC */
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
				}

				arg = atoi(tokenBuffer[0]);
				printf("Decrementing %d from VC\n", arg);
				VC -= arg;

				break;

			case 'C':
				/* Call the OS to start new process: 'C <priority> <filename>'*/
				if ( tokenizeLine() != 2 ) {
					/* Invalid arguments. Notify the OS. */
				}

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
				continue;
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
