#include <Kernel.h>
static String PROGRAM_NAME = "ls";

/* ls command */

static void print(String name, Stat *stat) {
	// a buffer to fill with a line of output
	StringBuffer *s = new StringBuffer("");

	// a temporary string
	String t = new char[100];

	// append the inode number in a field of 5
	sprintf (t, "%d", stat->getIno());
	for(int i = 0; i < 5 - strlen(t); i ++)
		s->append(' ');
	s->append(t);
	s->append(' ');

	// append the size in a field of 10
	sprintf(t, "%d", stat->getSize());
	for(int i = 0; i < 10 - strlen(t); i ++)
		s->append(' ');
	s->append(t);
	s->append(' ');

	// append the name
	s->append(name);

	// print the buffer
	fprintf(stdout, "%s\n", s->toString());
}

static void printSym(String name, Stat *stat) {
	StringBuffer *s = new StringBuffer("");
	String t = new char[200];

	short nodenum = stat->getIno();

	int size = stat->getSize();
	byte *path = new byte[size];
	memset((void*) path, '\0', size);

	//Open an FD to read path in symlink
	FileDescriptor *fd = new FileDescriptor(Kernel::ROOT_FILE_SYSTEM, nodenum,
											Kernel::O_RDONLY);

	int _fd = Kernel::open(fd);
	Kernel::read(_fd, path, size);

	//fprintf(stdout, "%d %d %s -> %s\n", nodenum, size, (char*) name, (char*) path);
	sprintf(t, "%d", nodenum);
	for(int i = 0; i < 5 - strlen(t); i ++)
		s->append(' ');
	s->append(t);
	s->append(' ');

	// append the size in a field of 10
	sprintf(t, "%d", size);
	for(int i = 0; i < 10 - strlen(t); i ++)
		s->append(' ');
	s->append(t);
	s->append(' ');

	name += *name == '/' ? 1 : 0;
	sprintf(t, "%s -> %s", (char*) name, (char*) path);
	s->append(t);

	fprintf(stdout, "%s\n", s->toString());

	delete[] t;
	delete[] path;
}


int main(int argc, char **argv) {
	// initialize the file system simulator kernel
	Kernel::initialize();

	//Default setting is to not dereference symlinks
	bool noderef = true;
	for (int i = 1; i < argc; ++i)
		if (!strcmp(argv[i], "-r")) {
			noderef = false;
			break;
		}

  // for each path-name given
  for(int i = 1; i < argc; i ++) {
		String name = argv[i];
		int status = 0;

		if (!strcmp(name, "-r")) {
			continue;
		}

		// stat the name to get information about the file or directory
		Stat *stat = new Stat();
		status = Kernel::stat(name, stat, noderef);
		if(status < 0) {
			Kernel::perror(PROGRAM_NAME);
			Kernel::Exit(1);
		}
		// mask the file type from the mode
		short type = (short)(stat->getMode() & Kernel::S_IFMT);

		/*
		fprintf(stdout, "inode mode: %d\n", stat->getMode());
		fprintf(stdout, "inode mode(masked): %d\n", type);
		fprintf(stdout, "S_IFREG = %d\n", Kernel::S_IFREG);
		fprintf(stdout, "S_IFSYM = %d\n", Kernel::S_IFSYM);
		fprintf(stdout, "S_IFDIR = %d\n", Kernel::S_IFDIR);
		*/

		// if name is a regular file, print the info
		if(type == Kernel::S_IFREG) {
			print(name, stat);
			Kernel::Exit(0);
		}
		else if (type == Kernel::S_IFSYM) {
			printSym(name, stat);
			Kernel::Exit(0);
		}
		// if name is a directory open it and read the contents
		else if(type == Kernel::S_IFDIR) {
			// open the directory
			int fd = Kernel::open(name, Kernel::O_RDONLY);
			if(fd < 0) {
				Kernel::perror(PROGRAM_NAME);
				fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
				   ": unable to open ", name, " for reading");
				Kernel::Exit(1);
		}

	  // print a heading for this directory
	  fprintf(stdout,"%s%s\n", name, ":");

	  // create a directory entry structure to hold data as we read
	  DirectoryEntry *directoryEntry = new DirectoryEntry();
	  int count = 0;

		char buf[500];
		// while we can read, print the information on each entry
		while(true) {
			memset((void*) buf, '\0', 500);
			// read an entry; quit loop if error or nothing read
			status = Kernel::readdir(fd, directoryEntry);
			if(status <= 0)
				break;

			// get the name from the entry
			String entryName = directoryEntry->getName();
			//fprintf(stdout, "DirectoryEntry name: %s\n", (char*) entryName);

			// call stat() to get info about the file
			//char buf[500];
			strcpy (buf, name);
			strcat (buf, "/");
			strcat (buf, entryName);
			status = Kernel::stat(buf, stat, noderef);
			if(status < 0) {
				Kernel::perror(PROGRAM_NAME);
				Kernel::Exit(1);
			}

			if ( (stat->getMode() & Kernel::S_IFMT) == Kernel::S_IFSYM )
				printSym(entryName, stat);
			else
				print(entryName, stat);

			count ++;
		}

	  // check to see if our last read failed
	  if(status < 0)
        {
          Kernel::perror("main");
	  fprintf(stderr, "main: unable to read directory entry from /\n");
          Kernel::Exit(2);
        }

	  // close the directory
	  Kernel::close(fd);

        // print a footing for this directory
	  fprintf(stdout, "%s%d\n", "total files: ", count);
	}
    }

  // exit with success if we process all the arguments
  Kernel::Exit(0);
}



