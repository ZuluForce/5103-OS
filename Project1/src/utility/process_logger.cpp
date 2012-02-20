#include "utility/process_logger.h"

//For the threads to access internal state
static cProcessLogger* log_ptr;

void* nameSockFn(void* args) {
	log_ptr->listenSock = socket(AF_UNIX, SOCK_DGRAM, 0);

	struct sockaddr_un local, remote;

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, procNameReq);

	unlink(procNameReq); //Incase there is a socket left in the directory
	if ( bind(log_ptr->listenSock, (struct sockaddr*) &local, sizeof(struct sockaddr_un)) ) {
		perror("Error binding socket");
		exit(-1);
	}

	pidType reqID;
	const char* pName;
	size_t nameLen;
	size_t sSize = sizeof(remote);

	while ( true ) {
		if ( recvfrom(log_ptr->listenSock, &reqID, sizeof(pidType), 0,
						(struct sockaddr*) &remote, (socklen_t*) &sSize) < 0) {
			if ( errno == EINTR )
				continue;
			perror("Error reading from socket");
			exit(-1);
		}

		if ( reqID > log_ptr->procNames.size() ) {
			/* Return Error Messages */
			printf("Invalid request for precess name (pid %d)\n", reqID);
			sendto(log_ptr->listenSock, requestError, strlen(requestError), 0,
					(struct sockaddr*) &remote, sizeof(struct sockaddr_un));

			continue;
		}

		pName = log_ptr->procNames.at(reqID).c_str();
		nameLen = log_ptr->procNames.at(reqID).length() + 1;

		sendto(log_ptr->listenSock, pName, nameLen, 0, (struct sockaddr*) &remote, sizeof(struct sockaddr_un));
	}

	return NULL;
}


cProcessLogger::cProcessLogger(const char *file) {
	assert(file != NULL);

	/* Create socket for top to request trace names */
	log_ptr = this;
	pthread_create(&nameReqListener, NULL, nameSockFn, this);

	/* Open trace log file */
	procLogStream = fopen(file, "w");

	if ( procLogStream == NULL ) {
		perror("Failed to open process log file");
		exit(-1);
	}

	procLogFD = fileno(procLogStream);

	lineSize = MAX_LINE_LENGTH;

	previousID = 0;

	/* Could have used memset */
	for ( int i = 0; i < MAX_LINE_LENGTH; ++i ) {
		emptyBuffer[i] = ' ';
	}

	/* Initialize the vector for storing process names */
	procNames.resize( 4 );

	pthread_mutex_init(&logWriteLock, NULL);

	return;
}

cProcessLogger::~cProcessLogger() {
	pthread_cancel(nameReqListener);
	pthread_join(nameReqListener, NULL);
	close(listenSock);

	procNames.clear();

	fclose(procLogStream);
	//remove(nameFile.substr())

	return;
}

void cProcessLogger::addProcess(ProcessInfo* proc, const char* name) {
	assert( name != NULL );

	pidType newID = lineIDs.getLowID();

	proc->procFileLine = newID;

	if ( newID >= procNames.size() )
		procNames.resize( procNames.size() * 2 );

	procNames.at(newID) = name;

	writeProcessInfo(proc);

	previousID = newID;
}

void cProcessLogger::rmProcess(ProcessInfo* proc) {
	pthread_mutex_lock(&logWriteLock);

	if ( lseek(procLogFD, proc->procFileLine * lineSize, SEEK_SET) == -1 ) {
		perror("lseek cProcessLogger::rmProcess");
		pthread_mutex_unlock(&logWriteLock);
		return;
	}

	int written;
	written = write(procLogFD, emptyBuffer, MAX_LINE_LENGTH - 1);
	if ( written != MAX_LINE_LENGTH - 1) fprintf(stderr, "Process line not properly cleared\n");

	lineIDs.returnID(proc->procFileLine);

	procNames.at(proc->procFileLine).clear();

	pthread_mutex_unlock(&logWriteLock);
	return;
}


void cProcessLogger::writeProcessInfo(ProcessInfo* proc) {
	assert(proc != NULL);

	pthread_mutex_lock(&logWriteLock);
	/* Seek to the correct file location */
	if ( lseek(procLogFD, proc->procFileLine * lineSize, SEEK_SET) == -1) {
		perror("Failed to seek within process file");
		pthread_mutex_unlock(&logWriteLock);
		return;
	}

	int size;

	size = snprintf(outputBuffer, MAX_LINE_LENGTH, outputFormat,
					proc->pid, proc->memory, proc->startCPU, proc->totalCPU, proc->state);

	assert(size < MAX_LINE_LENGTH);

	//Pad it to the correct length
	for (int i = 0; i < MAX_LINE_LENGTH - size; ++i) {
		outputBuffer[size + i] = ' ';
	}

	outputBuffer[MAX_LINE_LENGTH - 1] = '\n';

	if ( write(procLogFD, outputBuffer, MAX_LINE_LENGTH) < MAX_LINE_LENGTH)
		perror("Failed writing entry to process table. Non fatal error.");

	pthread_mutex_unlock(&logWriteLock);
	return;
}
