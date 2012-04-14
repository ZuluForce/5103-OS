#include <Kernel.h>

static const String PROGRAM_NAME = "find";

int printDirContents(int fd, String path){

    int status;
    int subFd;
    DirectoryEntry directoryEntry;
    Stat stat;
    String name;
    fprintf(stdout, "%s\n", path);

    while (1){
        // read an entry; quit loop if error or nothing read
        status = Kernel::readdir(fd, &directoryEntry);
        if(status <= 0)
            break;
        name = directoryEntry.getName();
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
        	continue;
        }
        StringBuffer newPathBuilder(path);
        newPathBuilder.append(name);
        String newPath = newPathBuilder.toString();

        status = Kernel::stat(newPath, &stat, true);
        if (status < 0){
        	Kernel::perror(PROGRAM_NAME);
            fprintf(stderr, "stat failed on %s\n", newPath);
            Kernel::Exit(2);
        }
        if ((stat.getMode() & Kernel::S_IFMT) == Kernel::S_IFDIR){
            subFd = Kernel::open(newPath, Kernel::O_RDONLY);
            if (subFd < 0){
                Kernel::perror(PROGRAM_NAME);
                fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
                   ": unable to open ", newPath, " for reading");
                Kernel::Exit(2);
            }
            newPathBuilder.append("/");
            printDirContents(subFd, newPathBuilder.toString());
        } else {
        	printf("%s\n", newPath);
        }

    }
    // check to see if our last read failed
    if(status < 0){
        Kernel::perror(PROGRAM_NAME);
        fprintf(stderr, "%s: unable to read directory entry\n", PROGRAM_NAME);
       exit(2);
    }

    // close the directory
    Kernel::close(fd);
    return 0;
}

/* find command */
int main(int argc, char **argv)
// throws Exception
{
  // initialize the file system simulator kernel
    Kernel::initialize();

    // print a helpful message if no command line arguments are given
	if(argc < 2)
	{
	  fprintf (stderr, "%s%s\n", PROGRAM_NAME, ": too few args");
	  Kernel::Exit(1);
	}

	// for each argument given on the command line
	for(int i = 1; i < argc; i ++){
	  // give the argument a better name
	  String name = argv[i];
	  int fd = Kernel::open(name, Kernel::O_RDONLY);
	  if(fd < 0) {
			Kernel::perror(PROGRAM_NAME);
			fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME,
			   ": unable to open ", "/", " for reading");
			Kernel::Exit(1);
	  }
	  int status = 0;
	  Stat stat;
	  status = Kernel::fstat(fd, &stat);
	  if (status < 0){
	          	Kernel::perror(PROGRAM_NAME);
	              fprintf(stderr, "stat failed on %s\n", name);
	              Kernel::Exit(2);
	          }
	  if ((stat.getMode() & Kernel::S_IFMT) == Kernel::S_IFDIR){
		  if ((strcmp(name, "/") != 0) && (name[strlen(name)-1] != '/')){
				  StringBuffer pathBuilder(name);
				  pathBuilder.append("/");
				  name = pathBuilder.toString();
		  }
		  printDirContents(fd, name);
	  } else{
		  printf("%s\n", name);
	  }

	}
    Kernel::Exit(0);

}

