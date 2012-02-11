#include "init.h"

int main(int argc, char **argv) {
	cout << "Initializing emulated OS" << endl;

	/* Initialize the Kernel */
    cKernel kernel = cKernel();

	//kernel.boot();

	return 0;
}
