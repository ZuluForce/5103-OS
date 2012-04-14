#include <Kernel.h>

static const String PROGRAM_NAME = "ln";

int main(int argc, char **argv) {
	Kernel::initialize();

	if ( argc < 3 ) {
		fprintf(stderr, "%s%s%s%s\n", PROGRAM_NAME, ": usage: ./",
				PROGRAM_NAME, " target file new-link");

		Kernel::Exit(1);
	}

	int status = 0;

	if ( argc >= 4 && !strcmp(argv[3], "-s") ) {
		status = Kernel::symlink(argv[1],argv[2]);
	} else {
		status = Kernel::link(argv[1],argv[2]);
	}

	if ( status < 0 ) {
		Kernel::perror(PROGRAM_NAME);
		Kernel::Exit(-1);
	}

	Kernel::Exit(0);
}
