#include <Kernel.h>
#include <unistd.h>

static const String PROGRAM_NAME = "cat";

/* The size of the buffer to be used for reading from the 
file.  A buffer of this size is filled before writing
to the output file */
static const int BUF_SIZE = 4096;

int main(int argc,char **argv) 
// throws Exception
{
  // initialize the file system simulator kernel
  Kernel::initialize();

  // display a helpful message if no arguments are given
  if(argc == 1)
    {
      fprintf (stderr,"%s%s%s%s\n",PROGRAM_NAME,": usage: " ,PROGRAM_NAME,
	       " input-file ...");
      Kernel::Exit(1);
    }

    // for each filename specified
  for(int i = 1; i < argc; i ++)
    {
      String name = argv[i];

      // open the file for reading
      int in_fd = Kernel::open(name ,Kernel::O_RDONLY);
      if(in_fd < 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  fprintf(stderr,"%s%s%s\n",PROGRAM_NAME ,": unable to open input file ",
		  name);
	  Kernel::Exit(2);
	}

      // create a buffer for reading data
      byte *buffer = new byte[BUF_SIZE];

      // read data while we can
      int rd_count;
      while(true)
	{
	  // read a buffer full of data
	  rd_count = Kernel::read(in_fd ,buffer ,BUF_SIZE);

	  // if we encounter an error or get to the end,quit the loop
	  if(rd_count <= 0)
	    break;

	  // write whatever we read to standard output
	  write (1,buffer,rd_count);
	}

      // close the input file
      Kernel::close(in_fd);

      // exit with failure if we encounter an error
      if(rd_count < 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  fprintf (stderr,"%s%s\n",PROGRAM_NAME,
		   ": error during read from input file");
	  Kernel::Exit(3);
	}
    }

  // exit with success if we read all the files without error
  Kernel::Exit(0);
}

