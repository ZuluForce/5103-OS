#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

class cCPU {
	private:
		/* 0 = User   1 = Kernel */
		bool KMode;

		/* max PC should be the size of the VAS */
		unsigned int PC, maxPC;
		int VC;

        /* Text of executing process */
        char *execText;

	public:
		cCPU();
		~cCPU();


        void setText(char*);

        unsigned int getSetPC(unsigned int);
        int getSetVC(int);

        void run();
		/* Functions to add here:
		 * setPC, getPC
		 * ...
		 */
};

#endif // CPU_H_INCLUDED
