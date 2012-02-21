#include "top.h"

/**
Top works in the following way:
It reads a proc.log file to get the PID, memory, cpustart, cputime,
and state information for each process. Each process is assigned a particular line
in this file. Top then uses these line numbers to request the name of the process from
the OS via a Unix socket. The OS updates the proc.log file as processes executes and Top
knows when to read again from this file by waiting on an inotify modify event.
*/

static int inotify_fd;
static int sockfd;
vector<char*> processNames;

/** @fn const char* getStatString(eProcState state)
*   This function converts the eProcState enum into a string.
*/
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

/** @fn int waitUpdate()
*   Waits for the proc.log file to be created.
*/
int waitUpdate() {
    int wd;
    // Add a watch on the current directory.
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
        /* A while loop is needed as multiple events could be generated before a read. This looping
        ensures that we get all of the events out and check to see if one matches the event of the
        procLogFile being created*/
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

    // Remove the watch on the current directory
    if (inotify_rm_watch(inotify_fd, wd) < 0){
        perror("Could not remove the watch");
        return -1;
    }
	return 0;
}


/** @fn int open_inotify_fd()
*   Initialize a new inotify instance and returns the file descriptor associated with the
*   inotify event
*/
int open_inotify_fd() {
	int fd;
    fd = inotify_init ();

    if (fd < 0){
        perror ("inotify_init failed () = ");
    }
    return fd;
}

/** @fn int setUpSocket()
*   Sets up the socket for OS <-> top communication
*/
int setUpSocket(){
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
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

/** @fn int getProcessName(unsigned int num, char *buf)
*   Sends the fileline number of the process to the OS via a Unix socket then waits for the OS to send
*   it back the corresponding name.
*/
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

/** @fn void saveProcessName(int id, char* name)
*   Saves the name in the processNames vector at the id location.
*/
void saveProcessName(int id, char* name){
    char *savedStr = (char*) malloc(strlen(name) + 1);
    if (savedStr == NULL){
        perror("Malloc failed in saveProcessName");
        exit(-1);
    }
    strcpy(savedStr, name);
    processNames.at(id) = savedStr;
}

/** @fn void removeProcessName(int id)
*   Removes a name from the process name vector and frees up the dynamic memory.
*/
void removeProcessName(int id){
    char *savedStr = processNames.at(id);
    processNames.at(id) = NULL;
    free(savedStr);
}

/** @fn assureProcessNameVectorBigEnough(int size)
*   This makes sure that the vector is big enough to store all of the process names.
*/
void assureProcessNameVectorBigEnough(int size){
    if (size >= processNames.size()){
        processNames.resize(size, NULL); // Fill new entries with NULL pointers
    }
}

int main(int argc, char** argv) {
    if (argc != 2){
        printf("Usage: %s <proc.log directory>", argv[0]);
        exit(-1);
    }

    // Change directory to the command line argument
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

	processNames.resize(PROCESS_NAMES_VEC_SIZE);
	processNames.clear();

	int fileSize;
	int numLines;
	unsigned int pid, memory, cpustart, cputime, state;
	char *buf = (char*) malloc( FILENAME_MAX);
	if (buf == NULL){
	    perror("Malloc failed");
	    exit(-1);
	}
	char *temp = (char*) malloc( MAX_LINE_LENGTH);
	if (temp == NULL){
	    perror("Malloc failed");
	    exit(-1);
	}
	ifstream file;
    boost::format fmter("%1% %|30t|%2% %|42t|%3% %|54t|%4% %|66t|%5% %|78t|%6%\n");

    int wf;
    // Add a watch on proc.log to generate events when it is modified
    wf = inotify_add_watch(inotify_fd, procLogFile, IN_MODIFY);
    int length;
    char buffer[EVENT_BUF_SIZE];
    int logFileModified;

	while(1) {

        fmter % "NAME" % "PID" % "MEMORY" % "CPUSTART" % "CPUTIME" % "STATE";
        cout << fmter;

        // Get the file size
        stat(procLogFile, &fileinfo);
        fileSize = fileinfo.st_size;
        numLines = fileSize / MAX_LINE_LENGTH;
        assureProcessNameVectorBigEnough(numLines);

        // Read from the file
        file.open(procLogFile);
        if (file.good()){
            for (int i = 0; i < numLines; i++){
                file.seekg(i * MAX_LINE_LENGTH);
                file.getline(temp, MAX_LINE_LENGTH);
                // Check if the process has been removed from the OS
                if (strncmp(temp, " ", 1) == 0){
                    if(processNames.at(i) != NULL){
                        removeProcessName(i);
                    }
                    continue;
                }
                file.seekg(i * MAX_LINE_LENGTH);
                file >> pid >> memory >> cpustart >> cputime >> state;
                if (processNames.at(i) != NULL){
                    strcpy(buf, processNames.at(i));
                } else{
                    if (getProcessName(i, buf) != 0){
                        strcpy(buf, "Unknown process name");
                    } else{
                        // Save the process name for later so we don't have to go to the socket again
                        saveProcessName(i, buf);
                    }
                }

                // If the process name is too long to fit on the screen, truncate it and append "..."
                if ( strlen(buf) > 30 ){
                	strcpy(buf+26, "...");
                }

                fmter % buf % pid % memory % cpustart % cputime % getStatString((eProcState) state);
                cout << fmter;
            }
        }

        file.close();

        logFileModified = 0;
        while(!logFileModified){
            length = read(inotify_fd, buffer, EVENT_BUF_SIZE);
            if (length < 0){
                perror("Problem waiting for proc file to be modified");
                return -1;
            }
            int i = 0;
            /* A while loop is needed as multiple events could be generated before a read. This looping
            ensures that we get all of the events out and check to see if one matches the event of the
            procLogFile being modified.*/
            while (i < length){
                struct inotify_event *event = (struct inotify_event *) &buffer[0];
                if (event->mask & IN_MODIFY){
                    logFileModified = 1;
                    break;
                }

                 i += EVENT_SIZE + event->len;
            }
        }

        // Clear the screen in between file updates
		system("clear");

	}

	return 0;
}
