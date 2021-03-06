#ifndef PROCESS_LOGGER_H_INCLUDED
#define PROCESS_LOGGER_H_INCLUDED

/** @file */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "process.h"
#include "utility/id.h"

using namespace std;

#define MAX_LINE_LENGTH 45 /**< Max line length for a process entry in the log file */

/* Format: pid memory cpustart cputime state */
static const char procNameReq[] = "proc.log.req";
static const char outputFormat[] = "%u %d %d %d %d";
static const char requestError[] = "INVALID_ID";

/** @var static const char procNameReq[]
 *	Name of unix socket file for processes like top to
 *	request process names.
 *
 *	Since process trace names are variable, their names
 *	are stored in a string objects and then served upon
 *	request to this socket. While filenames do have a max
 *	size, it is more efficient to do it this way when you
 *	consider that most trace file names will not be near
 *	the max.
 */

/** @var static const char outputFormat[]
 *	Format for process info in the log file
 *
 *	Scanf is used to print the information in this
 *	format to a buffer. Then this buffer is padded
 *	to fill #MAX_LINE_LENGTH and then it is written
 *	out to the appropriate line in the file.
 */

/** @var static const char requestError[]
 *	Return value when a invalid process ID was requested.
 *
 *	If another process requests an invalid ID on the request
 *	socket, this is the corresponding message.
 */

/** Class specifically for logging process state information
 *
 *	In order for monitoring programs such as top to function
 *	the kernel and its associated modules must export process
 *	information. This class logs information for each process
 *	to a file named by its pid. This is inspired by the unix
 *	/proc filesystem (although memory mapped files aren't being used).
 *	This allows the kernel to easily update only those processes
 *	which have changed.
 */
class cProcessLogger {
	private:
		cIDManager lineIDs;

		string nameFile;

		int procLogFD;
		FILE* procLogStream;
		int lineSize;

		void addToVector(FILE*);

		char outputBuffer[MAX_LINE_LENGTH];
		char emptyBuffer[MAX_LINE_LENGTH];

		int previousID;

		/* Socket for requesting process name */
		int listenSock;
		pthread_t nameReqListener;

		vector<string> procNames;
		/* ---------------------------------- */

		pthread_mutex_t logWriteLock;


	public:
		cProcessLogger(const char *file);
		~cProcessLogger();

		void addProcess(ProcessInfo*, const char*);
		void rmProcess(ProcessInfo*);

		void writeProcessInfo(ProcessInfo*);

		friend void* nameSockFn(void*);
};

enum pivotType {
	pivotMiddle,
	pivotRandom
};

/* Sort Functions Return Values:
 * -1 : Desired order
 * 0  : Equal
 * 1  : Reverse order (will be switched)
 */
template <typename T>
void QuickPartition(std::vector<T> items, int (*sort_fn) (T,T),
					int left, int right, int pivot = pivotMiddle) {
	T temp;
	T pivotValue = items[pivot];

	/* Move pivot to the end of the section */
	temp = items[right];
	items[right] = pivotValue;
	items[pivot] = temp;

	int placeIndex = left;
	for (int i = left; i < right; ++i) {
		if (sort_fn(items[i], pivotValue) == 1) {
			/* Swap the values */
			temp = items[i];
			items[i] = items[placeIndex];
			items[placeIndex] = temp;

			++placeIndex;
		}
	}

	/* Put pivot into correct location */
	items[right] = items[placeIndex];
	items[placeIndex] = pivotValue;

	return;
}

/* Quicksort Implementation for std::vectors
 * Uses in-place partitioning for better cache performance
 */
template <typename T>
void QuicksortVector(std::vector<T> items, int (*sort_fn) (T,T), pivotType pType,
						int start, int end) {
	if ( (end - start + 1) <= 1 )
		return;
	int pivot;

	switch ( pType ) {
		case pivotMiddle:
			pivot = (start + end) / 2;
			break;

		case pivotRandom:
			pivot = start + (rand() % (start - end));
			break;

		default:
			return;
	}

	QuickPartition<T>(items, sort_fn, start, end, pivot);
	QuicksortVector<T>(items, sort_fn, pType, start, pivot);
	QuicksortVector<T>(items, sort_fn, pType, pivot + 1, end);
	return;
}


#endif // PROCESS_LOGGER_H_INCLUDED
