#include "Kernel.h"

static String PROGRAM_NAME = "df";

int main(int argc, char** argv) {
	Kernel::initialize();

	int status = 0;
	status = Kernel::filesysStatus();
	if ( status < 0 ) {
		Kernel::perror(PROGRAM_NAME);
		return -1;
	}

	Kernel::Exit(0);
}
