#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

class CPU {
	private:
		/* 0 = User   1 = Kernel */
		bool mode;

		int PC;
		int VC;

	public:
		CPU();
		~CPU();

		/* Functions to add here:
		 * setPC, getPC
		 * ...
		 */
};

#endif // CPU_H_INCLUDED
