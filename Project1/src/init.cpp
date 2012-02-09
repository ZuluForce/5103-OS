#include "init.h"

int main(int argc, char **argv) {
	cout << "Initializing emulated OS" << endl;

	/* Initialize the CPU */
    cCPU cpu();

	/* Initialize the Kernel */
    cKernel kernel();

	/* Load first program to run */

	return 0;
}
