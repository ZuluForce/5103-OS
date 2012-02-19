#include "top.h"

static int inotify_fd;

const char* getStatString(eProcState state) {
	switch (state) {
		case ready:
			return "READY";

		case blocked:
			return "BLOCKED";

		case running:
			return "RUNNING";

		case terminated:
			return "TERMINATED";

		default:
			return "UNKNOWN";
	}
}

void waitUpdate() {
    int wd;
    wd = inotify_add_watch(inotify_fd, procLogFile, IN_CREATE);
    int length;
    char buffer[EVENT_BUF_SIZE];
    while(1){
        length = read(inotify_fd, buffer, EVENT_BUF_SIZE);
        struct inotify_event *event = (struct inotify_event *) &buffer[0];
        if (event->mask & IN_CREATE){
            printf("%s file was created.\n", procLogFile);
            break;
        }
    }


    inotify_rm_watch(inotify_fd, wd);
	return;
}


int open_inotify_fd() {
	int fd;
    fd = inotify_init ();

    if (fd < 0){
        perror ("inotify_init failed () = ");
    }
    return fd;
}

void getProcessName(int num){
    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Error opening socket: ");
        exit(-1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DEFAULT_PORT);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Error connecting to socket: ");
        exit(-1);
    }

}

int main(int argc, char** argv) {
	if ( argc > 1 ) {
	    printf("%s\n", argv[1]);
		if ( chdir(argv[1]) == -1 ) {
			perror("Failed to change directories");
			exit(-1);
		}
		printf("Switched to directory: %s\n", argv[1]);
	}

    inotify_fd = open_inotify_fd();

	struct stat fileinfo;
	if ( stat(procLogFile, &fileinfo) == -1 ) {
		printf("Process log file doesn't exist. Going into wait.\n");
		waitUpdate();
	}

	printf("PID  MEMORY  CPUSTART  CPUTIME  STATE\n");
    int fileSize = fileinfo.st_size;
    int numLines = fileSize / MAX_LINE_LENGTH;
    int pid, memory, cpustart, cputime, state;

    // Read from the file
    ifstream file;
    file.open(procLogFile);
    if (file.good()){
        for (int i = 0; i < numLines; i++){
            file.seekg(i * MAX_LINE_LENGTH);
            file >> pid >> memory >> cpustart >> cputime >> state;
            printf("%d %d %d %d %s\n", pid, memory, cpustart, cputime, getStatString((eProcState) state));
        }
    }

    printf("\n\n\n\n");






	int wf;
	wf = inotify_add_watch(inotify_fd, procLogFile, IN_MODIFY);



	cout << boost::format("%1% %2% %3% %1%\n") % "This" % "is" % "a";
	return 0;
}
