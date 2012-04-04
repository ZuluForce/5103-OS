#include <Kernel.h>

static const String PROGRAM_NAME = "cp";

// The size of the buffer to be used when reading files.
static const int BUF_SIZE = 4096;

// The file mode to use when creating the output file.
// ??? perhaps this should be the same mode as the input file
static const short OUTPUT_MODE = 0700;


/* cp command */
int main(int argc, char **argv) 
// throws Exception
{
  // initialize the file system simulator kernel
  Kernel::initialize();

  // make sure we got the correct number of parameters
  if(argc != 3)
    {
      fprintf(stderr,"%s%s%s%s\n", PROGRAM_NAME , ": usage: ./" ,
	      PROGRAM_NAME , " input-file output-file");
      Kernel::Exit(1);
    }

  // give the parameters more meaningful names
  String in_name = argv[1];
  String out_name = argv[2];

  // open the input file
  int in_fd = Kernel::open(in_name , Kernel::O_RDONLY);
  if(in_fd < 0)
    {
      Kernel::perror(PROGRAM_NAME);
      fprintf(stderr,"%s%s%s\n", PROGRAM_NAME, ": unable to open input file",
	      in_name);
      Kernel::Exit(2);
    }

  // open the output file
  int out_fd = Kernel::creat(out_name , OUTPUT_MODE);
  if(out_fd < 0)
    {
      Kernel::perror(PROGRAM_NAME);
      fprintf(stderr,"%s%s%s\n", PROGRAM_NAME, ": unable to open output file ",
	      argv[2]);
      Kernel::Exit(3);
    }

  // read and write buffers full of data while we can
  int rd_count;
  byte* buffer = new byte[BUF_SIZE];
  while(true)
    {
      // read a buffer full from the input file
      rd_count = Kernel::read(in_fd , buffer , BUF_SIZE);

      // if error or nothing read, quit the loop
      if(rd_count <= 0)
        break;

      // write whatever we read to the output file
      int wr_count = Kernel::write(out_fd , buffer , rd_count);

      // if error or nothing written, give message and exit
      if(wr_count <= 0)
	{
	  Kernel::perror(PROGRAM_NAME);
	  fprintf(stderr, "%s%s\n", PROGRAM_NAME ,
		  ": error during write to output file");
	  Kernel::Exit(4);
	}
    }

  // close the files
  Kernel::close(in_fd);
  Kernel::close(out_fd);

  // check to see if the final read was successful; exit accordingly
  if(rd_count == 0)
    Kernel::Exit(0);
  else
    {
      Kernel::perror(PROGRAM_NAME);
      fprintf(stderr,"%s%s\n", PROGRAM_NAME,
	      ": error during read from input file");
      Kernel::Exit(5);
    }
}

