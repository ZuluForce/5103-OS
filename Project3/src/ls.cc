#include <Kernel.h>
static String PROGRAM_NAME = "ls";

/* ls command */

static void print(String name, Stat *stat)
{
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


int main(int argc, char **argv)
// throws Exception
{
  // initialize the file system simulator kernel
  Kernel::initialize();
  // for each path-name given
  for(int i = 1; i < argc; i ++) {
		String name = argv[i];
		int status = 0;

		// stat the name to get information about the file or directory
		Stat *stat = new Stat();
		status = Kernel::stat(name, stat);
		if(status < 0) {
			Kernel::perror(PROGRAM_NAME);
			Kernel::Exit(1);
		}
		// mask the file type from the mode
		short type = (short)(stat->getMode() & Kernel::S_IFMT);

		// if name is a regular file, print the info
		if(type == Kernel::S_IFREG) {
			print(name, stat);
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

	  // while we can read, print the information on each entry
	  while(true)
	    {
	      // read an entry; quit loop if error or nothing read
	      status = Kernel::readdir(fd, directoryEntry);
	      if(status <= 0)
			break;

	      // get the name from the entry
	      String entryName = directoryEntry->getName();

	      // call stat() to get info about the file
	      char buf[500];
	      strcpy (buf, name);
	      strcat (buf, "/");
	      strcat (buf, entryName);
	      status = Kernel::stat(buf, stat);
	      if(status < 0)
		{
		  Kernel::perror(PROGRAM_NAME);
		  Kernel::Exit(1);
		}

	      // print the entry information
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



