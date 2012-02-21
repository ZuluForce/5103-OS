#include "cpu.h"

cCPU::cCPU() {
	clockTick = 0;

	PC = maxPC = VC = 0;
	PSW = 0;

	execText = NULL;
	Opcode = '\0';

	return;
}

cCPU::~cCPU() {

	return;
}

void cCPU::initTraceLog() {
	traceStream = getStream();
}

void cCPU::initClockPulse(pthread_mutex_t* _pulseLock, pthread_cond_t* _pulseCond) {
	pulseLock = _pulseLock;
	pulseCond = _pulseCond;
}

int cCPU::tokenizeLine() {
	if ( PC >= maxPC )
		return 0;

	char c = execText[PC];

	//Strip all whitespace
	while ( c == ' ' || c == '\t') {
		c = execText[++PC];
	}

	int bufferIndex = 0, count = 0;

	while ( count < MAX_PARAMS && bufferIndex < MAX_PARAM_SIZE ) {
		if ( c == ' ' || c == '\t') {
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

void cCPU::setMaxPC(unsigned int _maxPC) {
	this->maxPC = _maxPC;
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
	if ( PC >= (maxPC - 1)) {
		//Abnormal termination
		PSW |= PS_EXCEPTION;
		PSW |= PS_ABNORMAL;

		return;
	}

	int arg; //Used for storing opcode parameters

	while (true) {
		/* Get the Opcode */
		Opcode = execText[PC];
		printf("PC = %d	Opcode: %c  Pid = %u\n", PC, Opcode, pid);

		switch (Opcode) {
			case 'S':
				/* 'S x': Store x to VC */
				++PC; //For the tokenizer to work
				if ( (arg = tokenizeLine()) != 1 ) {
					/* Invalid arguments. Notify the OS. */
					fprintf(traceStream, "Invalid number of arguments for S operation: %d\n", arg);
					printf("Invalid number of arguments for S operation: %d\n", arg);
					for ( int i = 0; i < arg; ++i)
						printf("Arg %d = %s\n", arg, tokenBuffer[i]);

					PSW |= PS_EXCEPTION;
					return;
				}

				fprintf(traceStream, "Executing: %c %s for pid = %d\n", Opcode, tokenBuffer[0], pid);
				arg = atoi(tokenBuffer[0]);
				VC = arg;

				PSW |= PS_FINISHED;
				return;

			case 'A':
				/* 'A x': Add x to VC */
				++PC;
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
					fprintf(traceStream, "Invalid number of arguments for A operation: %d\n", arg);
					printf("Invalid number of arguments for A operation: %d\n", arg);

					PSW |= PS_EXCEPTION;
					return;
				}

				fprintf(traceStream, "Executing: %c %s for pid = %d\n", Opcode, tokenBuffer[0], pid);
				arg = atoi(tokenBuffer[0]);
				VC += arg;

				PSW |= PS_FINISHED;
				return;

			case 'D':
				/* 'D x': Decrement x from VC */
				++PC;
				if ( tokenizeLine() != 1 ) {
					/* Invalid arguments. Notify the OS. */
					fprintf(traceStream, "Invalid number of arguments for D operation: %d\n", arg);
					printf("Invalid number of arguments for D operation: %d\n", arg);

					PSW |= PS_EXCEPTION;
					return;
				}

				fprintf(traceStream, "Executing: %c %s for pid = %d\n", Opcode, tokenBuffer[0], pid);
				arg = atoi(tokenBuffer[0]);
				VC -= arg;

				PSW |= PS_FINISHED;
				return;

			case 'C':
				/* Call the OS to start new process: 'C <priority> <filename>'*/
				++PC;
				if ( tokenizeLine() != 2 ) {
					/* Invalid arguments. Notify the OS. */
					fprintf(traceStream, "Invalid number of arguments for C operation: %d\n", arg);
					printf("Invalid number of arguments for C operation: %d\n", arg);

					PSW |= PS_EXCEPTION;
					return;
				}

				fprintf(traceStream, "Executing: %c %s %s for pid = %d\n",
							Opcode, tokenBuffer[0], tokenBuffer[1], pid);
				PSW |= PS_SYSCALL;
				KMode = true; //Enter kernel mode

				return;

			case 'I':
				/* IO syscall to device class: 'I <dev-class> (B/C) */
				++PC;
				if ( tokenizeLine() != 1) {
					fprintf(traceStream, "Invalid number of arguments for I operation: %d\n", arg);
					printf("Invalid number of arguments for I operation: %d\n", arg);

					PSW |= PS_EXCEPTION;
					return;
				}

				fprintf(traceStream, "Executing: %c %s for pid = %d\n", Opcode, tokenBuffer[0], pid);
				PSW |= PS_SYSCALL;
				KMode = true; //Enter kernel mode

				return;

			case 'P':
				/* Privileged instruction */
				if ( !KMode ) {
					/* Exception!, notify the OS */
					printf("Process pid = %d tried privileged operation in user mode\n", pid);
					PSW |= PS_EXCEPTION;

					fprintf(traceStream, "Invalid privileged instruction in user mode pid = %d\n", pid);

					return;
				}
				/* Else it is a NoOp */

				fprintf(traceStream, "Executing: P for kernel\n");
				PSW |= PS_FINISHED;
				return;

			case 'E':
				/* Terminate the process. Notify the OS */
				fprintf(traceStream, "Executing: %c for pid = %d\n", Opcode, pid);

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

void cCPU::executePrivSet(int num, int& clockTick) {
	while ( num > 0 ) {
		pthread_cond_wait(pulseCond, pulseLock);

		if ( KMode ) {
			++clockTick;
			fprintf(traceStream, "\nClocktick: %d\n", clockTick);
			fprintf(traceStream, "Executing privileged instruction for kernel on behalf of pid = %d\n", pid);
			printf("\nClocktick: %d\n", clockTick);
			printf("Executing privileged instruction for kernel on behalf of pid = %d\n", pid);
			--num;
		} else {
			PSW |= PS_EXCEPTION;
			return;
		}

	}

	return;
}
