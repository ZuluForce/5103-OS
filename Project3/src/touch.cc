#include <Kernel.h>
#include <unistd.h>

/* This is the nearly the same as 'tee' except it
 * does not read from std input. It creates a file
 * with the given name of size 0 bytes.
 */

static const String PROGRAM_NAME = "touch";

// The file mode to use when creating the output file.
static const short OUTPUT_MODE = 0700;

/* Copies standard input to standard output and to a file */
int main (int argc, char **argv) {
	// initialize the file system simulator kernel
	Kernel::initialize ();

	// print a helpful message if the number of arguments is not correct
	if (argc != 2) {
		fprintf (stderr, "%s%s%s%s\n", PROGRAM_NAME, ": usage: ./",
	   		PROGRAM_NAME, " output-file\n");
		Kernel::Exit (1);
	}

	// give the command line argument a better name
	String name = argv[1];

	// create the output file
	int out_fd = Kernel::creat (name, OUTPUT_MODE);
	if (out_fd < 0) {
		Kernel::perror (PROGRAM_NAME);
		fprintf (stderr, "%s%s%s\n", PROGRAM_NAME,
	   		": unable to open output file ", name);
		Kernel::Exit (2);
	}

	// close the output file
	Kernel::close (out_fd);

	// exit with success
	Kernel::Exit (0);
}
