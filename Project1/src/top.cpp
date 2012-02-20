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

int waitUpdate() {
    int wd;
    wd = inotify_add_watch(inotify_fd, ".", IN_CREATE);
    if (wd == -1){
        perror("Could not add watch");
        return -1;
    }
    int length;
    char buffer[EVENT_BUF_SIZE];
    int fileCreated = 0;
    while(!fileCreated){
        length = read(inotify_fd, buffer, EVENT_BUF_SIZE);
        if (length < 0){
            perror("Problem waiting for proc file to be created");
            return -1;
        }
        int i = 0;
        while (i < length){
            struct inotify_event *event = (struct inotify_event *) &buffer[0];
            if (event->mask & IN_CREATE){
                if (strcmp(event->name, procLogFile) == 0){
                    fileCreated = 1;
                    printf("Process log file now exits.\n");
                    break;
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }


    if (inotify_rm_watch(inotify_fd, wd) < 0){
        perror("Could not remove the watch");
        return -1;
    }
	return 0;
}


int open_inotify_fd() {
	int fd;
    fd = inotify_init ();

    if (fd < 0){
        perror ("inotify_init failed () = ");
    }
    return fd;
}

int setUpSocket(){
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("Error opening socket for writing");
        return -1;
    }

    /* Name of socket where we receive responses from the OS */
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, recvSockName);

    /* Binding is necessary so a response can be returned by the OS */
    unlink(recvSockName); // In case there is a socket left in the directory
    if (bind(sockfd, (struct sockaddr *) &local, sizeof(local))){
        perror("Error binding socket");
        return -1;
    }

    /* The name of the unix socket where we will request process names */
    dest.sun_family = AF_UNIX;
    strcpy(dest.sun_path, procNameReq);
    return 0;
}

int getProcessName(unsigned int num, char *buf){

    // Request the process name
    if (sendto(sockfd, &num, sizeof(num), 0, (struct sockaddr*) &dest, sizeof(dest)) == -1){
        perror("Could not send a request to the OS for the process name");
        return -1;
    }

    // Wait for the return value
    if (recv(sockfd, buf, FILENAME_MAX, 0) == -1){
        perror("Could not receive the process name from the OS");
        return -1;
    }

    return 0;

}

int main(int argc, char** argv) {
    if (argc != 2){
        printf("Usage: %s <proc.log directory>", argv[0]);
        exit(-1);
    }
    if ( chdir(argv[1]) == -1 ) {
        perror("Failed to change directories");
        exit(-1);
    }
    strncpy(procLogFileDir, argv[1], 256);
    printf("Switched to directory: %s\n", argv[1]);


    inotify_fd = open_inotify_fd();
    if (inotify_fd < 0){
        exit(-1);
    }

	struct stat fileinfo;
	if ( stat(procLogFile, &fileinfo) == -1 ) {
		printf("Process log file doesn't exist. Waiting for it to be created.\n");
		 if (waitUpdate() != 0){
		     exit(-1);
		 }
	}

	if (setUpSocket() != 0){
	    exit(-1);
	}

	while(1){

        boost::format fmter("%1% %|40t|%2% %|52t|%3% %|64t|%4% %|76t|%5% %|88t|%6%\n");

        fmter % "NAME" % "PID" % "MEMORY" % "CPUSTART" % "CPUTIME" % "STATE";


        cout << fmter;
        //printf("PID  MEMORY  CPUSTART  CPUTIME  STATE\n");
        stat(procLogFile, &fileinfo);
        int fileSize = fileinfo.st_size;
        int numLines = fileSize / MAX_LINE_LENGTH;
        unsigned int pid, memory, cpustart, cputime, state;


        char *buf = (char*) malloc( FILENAME_MAX);
        char *temp = (char*) malloc( MAX_LINE_LENGTH);
        // Read from the file
        ifstream file;
        file.open(procLogFile);
        if (file.good()){
            for (int i = 0; i < numLines; i++){
                file.seekg(i * MAX_LINE_LENGTH);
                file.getline(temp, MAX_LINE_LENGTH);
                if (strncmp(temp, " ", 1) == 0){
                    continue;
                }
                file.seekg(i * MAX_LINE_LENGTH);
                file >> pid >> memory >> cpustart >> cputime >> state;
                if (getProcessName(pid, buf) != 0){
                    strcpy(buf, "Unknown process name");
                }
                fmter % buf % pid % memory % cpustart % cputime % getStatString((eProcState) state);
                cout << fmter;
                //printf("%d %d %d %d %s\n", pid, memory, cpustart, cputime, getStatString((eProcState) state));
            }
        }
        file.close();








        int wf;
        wf = inotify_add_watch(inotify_fd, procLogFile, IN_MODIFY);

        int length;
        char buffer[EVENT_BUF_SIZE];
        int logFileModified = 0;
        while(!logFileModified){
            printf("Wating for process log file to be modified\n");
            length = read(inotify_fd, buffer, EVENT_BUF_SIZE);
            if (length < 0){
                perror("Problem waiting for proc file to be modified");
                return -1;
            }
            int i = 0;
            while (i < length){
                struct inotify_event *event = (struct inotify_event *) &buffer[0];
                if (event->mask & IN_MODIFY){
                    printf("%s file was modified.\n", procLogFile);
                    logFileModified = 1;
                    break;
                }

                 i += EVENT_SIZE + event->len;
            }
        }

         printf("\n\n\n\n");

	}

	return 0;
}
