#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

static char deviceName[] = "/dev/scullBuffer0";
static int flags = O_WRONLY;

#define MAX_BUFFER 1024

typedef enum {false,true} bool;

int device;

void sigint(){
	printf("Caught a signal, closing device %d.\n", device);
	if (device >= 0){
		close(device);
	}
	exit(0);
}

int main(int argc, char **argv) {
	signal(SIGINT, sigint);
	device = open(deviceName, flags);
	if ( device < 0 ) {
		perror("Failed to open deivce");
		return -1;
	}
	
	srand(time(NULL));
	int producerID;

	//Either choose a random ID or use the specified one
	if ( argc > 1 )
		producerID = atoi(argv[1]);
	else
		producerID = rand() % 100;
	
	/* I made the minimum 4 so that we could at least
	 * write the producerID out as a string */
	int bufferSize = (rand() % MAX_BUFFER) + 4;
	char writeBuffer[bufferSize];
	
	fprintf(stdout, "Producer %d started with buffer size %d\n",
			producerID, bufferSize);
	
	//Pick some ascii character between a-Z to fill the buffer
	//65 is 'a' and there are 58 ascii characters between a-Z
	char fillChar = 65 + (producerID % 58);
	
	fprintf(stdout, "Producer %d using character %c to fill buffer\n",
			producerID, fillChar);
	memset((void*)writeBuffer, fillChar, bufferSize);
	
	//Write producerID into data
	sprintf(writeBuffer, "%d", producerID);
	
	/* I'm not sure if it is only in C++ but you can use
	 * ternary operators as lvalues so you could write the
	 * following as:
	 * producerID < 10 ? writeBuffer[1] : writeBuffer[2] = fillChar
	 *
	 * Just something neat I wanted to throw out.
	 * 
	 * Anywyas, as an actual comment this is just for getting rid of
	 * the null terminator after the sprintf.
	 */
	if (producerID < 10){
		writeBuffer[1] = fillChar;
	} else{
		writeBuffer[2] = fillChar;
	}
	writeBuffer[bufferSize-1] = '\0';
	
	//Start filling up that buffer
	int error;
	while (true) {
		error = 0;

		do {
			error = write(device, writeBuffer, bufferSize);
			
			if ( error < 0 ) {
				fprintf(stderr, "Producer %d encountered an error while writing (%d)\n",
						producerID, error);

				return error;
			} else if ( error == 0 ) {
				fprintf(stderr, "Producer %d: Device full with no consumers. Retry in 4 seconds\n",
						producerID);
				
				sleep(4);
			} else {
				fprintf(stdout, "Producer wrote %d bytes\n", error);
				fprintf(stdout, "Wrote: '%s'\n", writeBuffer);
				//Sleep anywhere from 100 ms to 1 second
				usleep((rand()%10) * 100000);
			}
		} while (error == 0);
	}
	
	return 0;
}
