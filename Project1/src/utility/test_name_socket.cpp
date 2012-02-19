#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "utility/process_logger.h"


using namespace std;

static const char recvSockName[] = "proc.names.recv";

int main(int argc, char** argv) {
	if ( argc < 2 ) {
		printf("Usage: %s ID\n", argv[0]);
		return 0;
	}

	int sock;
	struct sockaddr_un local, dest;
	char *buf = (char*) malloc( FILENAME_MAX );

	/* This is the socket we will send data out
	 * from. */
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);

	/* Name of socket where we will recevie responses
	 * from the os */
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, recvSockName);

	/* Binding is necessary so a response can be
	 * returned by the os */
	unlink(recvSockName); //Incase there is a socket left in the directory
	if ( bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_un)) ) {
		perror("Error binding socket");
		exit(-1);
	}

	/* The name of the unix socket where we will
	 * request process names */
	dest.sun_family = AF_UNIX;
	strcpy(dest.sun_path, procNameReq);

	unsigned int reqID = atoi(argv[1]);

	//Request the process name
	sendto(sock, &reqID, sizeof(reqID), 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_un));

	//Wait for the return value
	recv(sock, buf, FILENAME_MAX, 0);

	printf("Received process name: %s\n", buf);
	return 0;
}
