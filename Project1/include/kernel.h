#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

#include <vector>

using namespace std;

class Kernel {
	private:
		int clockTick;

		//Devices:
		//vector<BlockDevice*> B_Devices;
		//vector<CharDevice*> C_Devices;
		//ClockDevice clockInterrupt;

	public:
		Kernel();
		~Kernel();


		/* Methods we will likely need:
		 * loadProgram
		 * ...
		 */
};

#endif // KERNEL_H_INCLUDED
