#include "utility/process_logger.h"

cProcessLogger::cProcessLogger(const char *file) {
	assert(file != NULL);

	procLogStream = fopen(file, "w");

	if ( procLogStream == NULL ) {
		perror("Failed to open process log file");
		exit(-1);
	}

	procLogFD = fileno(procLogStream);

	lineSize = MAX_LINE_LENGTH;

	return;
}

cProcessLogger::~cProcessLogger() {

	return;
}

void cProcessLogger::addProcess(ProcessInfo* proc) {
	pidType newID = lineIDs.getLowID();
	printf("Added process to log file on line %d\n", newID);

	proc->procFileLine = newID;

	writeProcessInfo(proc);
}

void cProcessLogger::writeProcessInfo(ProcessInfo* proc) {
	assert(proc != NULL);

	/* Seek to the correct file location */
	if ( lseek(procLogFD, proc->procFileLine * lineSize, SEEK_SET) == -1) {
		perror("Failed to seek within process file");
		return;
	}

	int size;

	/* Format: pid memory status*/
	size = snprintf(outputBuffer, MAX_LINE_LENGTH, outputFormat,
					proc->pid, proc->memory, proc->totalCPU, proc->state);

	assert(size < MAX_LINE_LENGTH);

	//Pad it to the correct length
	for (int i = 0; i < MAX_LINE_LENGTH - size; ++i) {
		outputBuffer[size + i] = ' ';
	}

	outputBuffer[MAX_LINE_LENGTH - 1] = '\n';

	write(procLogFD, outputBuffer, MAX_LINE_LENGTH);

	return;
}
