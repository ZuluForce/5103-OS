#include "utility/logger.h"

static FILE* logStream;

static bool logInitialized;

FILE* initLog(const char* filename) {
	assert(filename != NULL);
	printf("Filename: %s\n", filename);

	logStream = fopen(filename, "w");

	if ( logStream == NULL ) {
		perror("Failed to open log file");

		return NULL;
	}


	logInitialized = true;

	return logStream;
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
