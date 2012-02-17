#include "utility/process_logger.h"

cProcessLogger::cProcessLogger(const char *file) {
	assert(file != NULL);

	procLogStream = fopen(file, "w");

	nameFile = file;
	nameFile += ".names";

	procNameStream = fopen(nameFile.c_str(), "w");

	if ( procLogStream == NULL ) {
		perror("Failed to open process log file");
		exit(-1);
	}

	procLogFD = fileno(procLogStream);
	procNameFD = fileno(procNameStream);

	lineSize = MAX_LINE_LENGTH;

	previousID = 0;

	for ( int i = 0; i < MAX_LINE_LENGTH; ++i ) {
		emptyBuffer[i] = ' ';
	}

	return;
}

cProcessLogger::~cProcessLogger() {
	fclose(procNameStream);
	//remove(nameFile.c_str());

	fclose(procLogStream);
	//remove(nameFile.substr())

	return;
}

void cProcessLogger::addProcess(ProcessInfo* proc, const char* name) {
	assert( name != NULL );

	pidType newID = lineIDs.getLowID();
	printf("Added process to log file on line %d\n", newID);

	proc->procFileLine = newID;

	writeProcessName(newID, name);
	writeProcessInfo(proc);

	previousID = newID;
}

void cProcessLogger::rmProcess(ProcessInfo* proc) {
	if ( lseek(procLogFD, proc->procFileLine * lineSize, SEEK_SET) == -1 ||
		lseek(procNameFD, proc->procFileLine * lineSize, SEEK_SET) == -1) {
		perror("lseek cProcessLogger::rmProcess");
		return;
	}

	int written;
	written = write(procLogFD, emptyBuffer, MAX_LINE_LENGTH - 1);
	if ( written != MAX_LINE_LENGTH - 1) fprintf(stderr, "Process line not properly cleared\n");
	written = write(procNameFD, emptyBuffer, MAX_NAME_LENGTH - 1);
	if ( written != MAX_NAME_LENGTH - 1) fprintf(stderr, "Process name line not properly cleared\n");

	fprintf(procLogStream, "\n");
	fprintf(procNameStream, "\n");

	lineIDs.returnID(proc->procFileLine);

	return;
}

void cProcessLogger::writeProcessName(int line, const char* name) {
	if ( lseek(procNameFD, line * MAX_NAME_LENGTH, SEEK_SET) == -1 ) {
		perror("writeProcessName failed to seek");
		return;
	}

	int size;

	size = snprintf(outputBuffer, MAX_LINE_LENGTH, "%s", name);

	if ( size >= MAX_NAME_LENGTH ) {
		fprintf(stderr, "Process name too long to write to log");
		return;
	}

	//Pad it to the correct length
	for (int i = 0; i < MAX_NAME_LENGTH - size; ++i) {
		outputBuffer[size + i] = ' ';
	}

	outputBuffer[MAX_NAME_LENGTH - 1] = '\n';

	if ( write(procNameFD, outputBuffer, MAX_NAME_LENGTH) < MAX_NAME_LENGTH)
		perror("Failed writing entry to process table. Non fatal error.");

	return;
}

void cProcessLogger::writeProcessInfo(ProcessInfo* proc) {
	assert(proc != NULL);

	/* Seek to the correct file location */
	if ( lseek(procLogFD, proc->procFileLine * lineSize, SEEK_SET) == -1) {
		perror("Failed to seek within process file");
		return;
	}

	int size;

	size = snprintf(outputBuffer, MAX_LINE_LENGTH, outputFormat,
					proc->pid, proc->memory, proc->totalCPU, proc->state);

	assert(size < MAX_LINE_LENGTH);

	//Pad it to the correct length
	for (int i = 0; i < MAX_LINE_LENGTH - size; ++i) {
		outputBuffer[size + i] = ' ';
	}

	outputBuffer[MAX_LINE_LENGTH - 1] = '\n';

	if ( write(procLogFD, outputBuffer, MAX_LINE_LENGTH) < MAX_LINE_LENGTH)
		perror("Failed writing entry to process table. Non fatal error.");

	return;
}
