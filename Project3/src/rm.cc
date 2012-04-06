#include "Kernel.h"

static const String PROGRAM_NAME = "rm";

int main(int argc, char** argv) {
	Kernel::initialize();

	if ( argc < 2 ) {
		fprintf(stderr, "%s%s%s\n", PROGRAM_NAME, ": usage: ./",
				PROGRAM_NAME, " file to remove");

		Kernel::Exit(1);
	}

	int status = 0;
	status = Kernel::unlink(argv[1]);

	if ( status < 0 ) {
		Kernel::perror(PROGRAM_NAME);
		Kernel::Exit(status);
	}

	return 0;
}
