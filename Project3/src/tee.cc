#include <Kernel.h>
#include <unistd.h>

static const String PROGRAM_NAME = "tee";

static const int BUF_SIZE = 4096;

// The file mode to use when creating the output file.
static const short OUTPUT_MODE = 0700;

/* Copies standard input to standard output and to a file */
int main (int argc, char **argv)
// throws Exception
{
  // initialize the file system simulator kernel
  Kernel::initialize ();

  // print a helpful message if the number of arguments is not correct
  if (argc != 2)
    {
      fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME, ": usage: ./",
	       PROGRAM_NAME, " output-file\n");
      Kernel::Exit (1);
    }

  // give the command line argument a better name
  String name = argv[1];

  // create the output file
  int out_fd = Kernel::creat (name, OUTPUT_MODE);
  if (out_fd < 0)
    {
      Kernel::perror (PROGRAM_NAME);
      fprintf (stderr, "%s%s%s\n", PROGRAM_NAME,
	       ": unable to open output file ", name);
      Kernel::Exit (2);
    }

  // create a buffer for reading from standard input
  byte *buffer = new byte[BUF_SIZE];

  // while we can, read from standard input
  byte ch;
  int i = 0;
  bool done = false;
  // read a buffer full of data from standard input
  do
  {
  		ch=getchar();
  		if(ch == EOF)
  			done = true;
      	else
      		buffer[i++] = ch;
      	if( i == BUF_SIZE || done)
      	{
      		// write what we read to the output file; if error, exit
      		int wr_count = Kernel::write (out_fd, buffer, i);
      		if (wr_count <= 0)
			{
	  			Kernel::perror (PROGRAM_NAME);
	  			fprintf (stderr, "%s%s\n", PROGRAM_NAME,
		   		": error during write to output file");
	  			Kernel::Exit (3);
			}
			// write what we read to standard output
      		write (1, buffer, i);
      		i = 0;
      	}
	} while(!done);
  // close the output file
  Kernel::close (out_fd);

  // exit with success
  Kernel::Exit (0);
}
