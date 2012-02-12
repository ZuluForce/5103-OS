#include "init.h"

int main(int argc, char **argv) {
	cout << "Initializing emulated OS" << endl;
	/* Parse command line arguments */

	/* Initialize the Kernel */
    cKernel kernel = cKernel();

	try {
		kernel.boot();
	} catch (const std::string& emsg) {
		cerr << "Error Caught: " << emsg << endl;
		exit(-1);
	} catch (const kernelError& kerr) {
		cerr << "Kernel Exception: " << kerr.message << endl;
		exit(-1);
	}

	return 0;
}
