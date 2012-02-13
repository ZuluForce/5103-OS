#include "utility/logger.h"

int initLog(const char* filename) {
	assert(filename != NULL);

	logStream = fopen(filename, "w");

	if ( logStream == NULL ) {
		perror("Failed to open log file");

		return -1;
	}


	logInitialized = true;

	return 0;
}

void closeLog() {
	assert(logInitialized);

	logInitialized = false;

	fclose(logStream);
}

FILE* getStream() {
	assert(logInitialized);

	return logStream;
}
