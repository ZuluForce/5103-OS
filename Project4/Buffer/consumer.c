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
static int flags = O_RDONLY;

#define BUF_SIZE 512

typedef enum {false,true} bool;

int device = -1;

void sigint(){
	printf("Caught a signal, closing device %d.\n", device);
	if (device >= 0){
		close(device);
	}
	exit(0);
}

int main(int argc, char **argv) {
	signal(SIGINT, sigint);
	device = open(deviceName,flags);
	if (device < 0) {
		perror("Failed to open device");
		return device;
	}
	
	char readBuffer[BUF_SIZE];
	char producerID[3];
	producerID[2] = '\0';
	
	memset((void*)readBuffer, 0, BUF_SIZE);
	
	fprintf(stdout, "Consumer setup with buffer of %d...starting to read\n",BUF_SIZE);
	
	int error;
	while (true) {
		error = 0;
		
		do {
			error = read(device, readBuffer, BUF_SIZE);
			if (error < 0) {
				fprintf(stderr, "Error reading form scull device (%d)\n", error);
				return error;
			} else if (error == 0) {
				printf("Consumer: Device empty with no producers. Retry in 4 seconds\n");
				sleep(4);
			} else {
				fprintf(stderr, "Consumer: Read %d bytes\n", error);

				memcpy(producerID,readBuffer,2);
				fprintf(stdout, "consumer: Message from producer (%d)-%s\n",
						atoi(producerID), readBuffer+2);
			}
		} while (error == 0);
	}
	
	return 0;
}
