/* mkdir program */

#include <Kernel.h>

static const String PROGRAM_NAME = "mkdir";

int main(int argc, char ** args) //throws Exception
  {
    // initialize the file system simulator kernel
    Kernel::initialize();

    // print a helpful message if no command line arguments are given
    if(argc < 2)
    {
      fprintf (stderr, "%s%s\n", PROGRAM_NAME, ": too few args");
      Kernel::Exit(1);
    }

    // create a buffer for writing directory entries
    byte *directoryEntryBuffer = new byte[DirectoryEntry::DIRECTORY_ENTRY_SIZE];

    // for each argument given on the command line
    for(int i = 1; i < argc; i ++)
    {
      // given the argument a better name
      String name = args[i];
      int status = 0;

      // call creat() to create the file
      int newDir = Kernel::creat(name , Kernel::S_IFDIR);
      if(newDir < 0)
      {
        Kernel::perror(PROGRAM_NAME);
	fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME, ": ",  name , "\"");
        Kernel::Exit(2);
      }
      // get file info for "."
      Stat *selfStat = new Stat();
      status = Kernel::fstat(newDir , selfStat);
      if(status < 0)
      {
        Kernel::perror(PROGRAM_NAME);
        Kernel::Exit(3);
      }
      // add entry for "."
      DirectoryEntry *self = new DirectoryEntry(selfStat->getIno() , ".");
      self->write(directoryEntryBuffer , 0);
      status = Kernel::write
	(newDir, directoryEntryBuffer ,  DirectoryEntry::DIRECTORY_ENTRY_SIZE);
      if(status < 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  Kernel::Exit(4);
	}
      // get file info for ".."
      Stat *parentStat = new Stat();
      char buf[100]; // remove magic #
      strcpy (buf, Kernel::getDeepestDir(name,true));

      status = Kernel::stat(buf, parentStat);
      // add entry for ".."
      DirectoryEntry *parent = new DirectoryEntry
	(parentStat->getIno() , "..");

      parent->write(directoryEntryBuffer , 0);
      status = Kernel::write
	(newDir, directoryEntryBuffer, DirectoryEntry::DIRECTORY_ENTRY_SIZE);
      if(status < 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  Kernel::Exit(5);
      }

      // call close() to close the file
      status = Kernel::close(newDir);
      if(status < 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  Kernel::Exit(6);
	}
    }

    // exit with success if we process all the arguments
    Kernel::Exit(0);
  }



