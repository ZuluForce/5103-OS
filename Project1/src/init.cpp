#include "init.h"

int main(int argc, char **argv) {
	cout << "Initializing emulated OS" << endl;

	/* Initialize the CPU */
    cCPU cpu = cCPU();

	/* Initialize the Kernel */
    cKernel kernel = cKernel(cpu);

	/* Load first program to run */

	while (true)
		pause();

	return 0;
}
