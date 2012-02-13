#include "cpu.h"

cCPU::cCPU() {
	PC = maxPC = VC = 0;
	PSW = 0;

	execText = NULL;
	Opcode = '\0';

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
			//Space encountered. Could be between parameters or trailing after the last one
			tokenBuffer[count][bufferIndex] = '\0'; //Terminate string
			count = bufferIndex > 0 ? count + 1 : count; //Increment the counter if anything was read in
			bufferIndex = 0;
			c = execText[++PC]; //Skip to next character
			continue;
		} else if ( c == '\n' ) {
			//End of the line
			tokenBuffer[count][bufferIndex] = '\0';
			count = bufferIndex > 0 ? count + 1 : count;
			++PC; //Bring it around to the next line
			break;
		} else if ( c == '\0' ) {
			//End of the program
			tokenBuffer[count][bufferIndex] = '\0';
			count = bufferIndex > 0 ? count + 1 : count;
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

void cCPU::setUserMode() {
	KMode = false;
}

unsigned int cCPU::getSetPC(unsigned int newPC) {
	unsigned int oldPC = PC;
	PC = newPC;
	return oldPC;
}

int cCPU::getSetVC(int newVC) {
	int oldVC = VC;
	VC = newVC;
	return oldVC;
}

uint16_t cCPU::getSetPSW(uint16_t newPSW) {
	uint16_t oldPSW = PSW;
	PSW = newPSW;
	return oldPSW;
}

uint16_t cCPU::getPSW() {
	return PSW;
}

void cCPU::setPSW(uint16_t newPSW) {
	PSW = newPSW;
}

char* cCPU::getParam(int num) {
	if ( num < MAX_PARAMS )
		return tokenBuffer[num];

	return NULL;
}

char cCPU::getOpcode() {
	return Opcode;
}

void cCPU::run() {
	assert(execText != NULL);
	//assert(PC <= maxPC);'
	int arg; //Used for storing opcode parameters

	while (true) {
		/* Get the Opcode */
		Opcode = execText[PC];
		printf("PC = %d	Opcode: %c\n", PC, Opcode);

		switch (Opcode) {
			case 'S':
				/* 'S x': Store x to VC */
				printf("Opcode: S\n");
				++PC; //For the tokenizer to work
				if ( (arg = tokenizeLine()) != 1 ) {
					/* Invalid arguments. Notify the OS. */
					printf("Invalid number of arguments for S operation: %d\n", arg);
					for ( int i = 0; i < arg; ++i)
						printf("Arg %d = %s\n", arg, tokenBuffer[i]);

					PSW |= PS_EXCEPTION;
					return;
				}

				arg = atoi(tokenBuffer[0]);
				printf("Storing %d to VC\n", arg);
				VC = arg;

				break;

			case 'A':
				/* 'A x': Add x to VC */
				printf("Opcode: A\n");
				++PC;
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
					PSW |= PS_EXCEPTION;
					return;
				}

				arg = atoi(tokenBuffer[0]);
				printf("Adding %d to VC\n", arg);
				VC += arg;

				break;

			case 'D':
				/* 'D x': Decrement x from VC */
				++PC;
				printf("Opcode: D\n");
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
					PSW |= PS_EXCEPTION;
					return;
				}

				arg = atoi(tokenBuffer[0]);
				printf("Decrementing %d from VC\n", arg);
				VC -= arg;

				break;

			case 'C':
				/* Call the OS to start new process: 'C <priority> <filename>'*/
				printf("Opcode: C\n");
				++PC;
				if ( tokenizeLine() != 2 ) {
					/* Invalid arguments. Notify the OS. */
					PSW |= PS_EXCEPTION;
					return;
				}

				PSW |= PS_SYSCALL;
				KMode = true; //Enter kernel mode

				return;

			case 'I':
				/* IO syscall to device class: 'I <dev-class> (B/C) */
				printf("Opcode: I\n");
				++PC;
				if ( tokenizeLine() != 1) {
					PSW |= PS_EXCEPTION;
					return;
				}

				PSW |= PS_SYSCALL;
				KMode = true; //Enter kernel mode

				return;

			case 'P':
				/* Priveleged instruction */
				printf("Opcode: P\n");
				if ( !KMode ) {
					/* Exception!, notify the OS */
					printf("Process tried privleged operation in user mode\n");
					PSW |= PS_EXCEPTION;

					return;
				}
				/* Else it is a NoOp */
				continue;

			case 'E':
				/* Terminate the process. Notify the OS */
				printf("Opcode: E\n");

				PSW |= PS_TERMINATE;

				return;

			default:
				/* Invalid Instruction, Notify the OS */
				//#ifdef DEBUG
				printf("Invalid instruction %c\n", Opcode);
				//#endif
				PSW |= PS_EXCEPTION;

				return;
		}


	}
}
