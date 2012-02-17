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

	return;
}

int open_inotify_fd() {
	int fd;

  watched_items = 0;
  fd = inotify_init ();

  if (fd < 0)
    {
      perror ("inotify_init () = ");
    }
  return fd;
}

int main(int argc, char** argv) {
	if ( argc > 1 ) {
		if ( chdir(argv[2]) == -1 ) {
			perror("Failed to change directories");
			exit(-1);
		}
		printf("Switched to directory: %s\n", argv[2]);
	}

	inotify_fd = open_inotify_fd();

	stat fileinfo;
	if ( stat(procLogFile, &fileinfo) == -1 ) {
		printf("Process log file doesn't exist. Going into wait.\n");
		waitUpdate();
	}

	cout << boost::format("%1% %2% %3% %1%\n") % "This" % "is" % "a";
	return 0;
}
