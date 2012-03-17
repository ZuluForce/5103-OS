#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "mmu.h"
#include "data_structs.h"
#include "iniReader.h"

enum eCPUState {
	CPU_OK = 0x1,			/**< CPU finished executing instruction ok */
	CPU_PF = CPU_OK << 1,	/**< CPU/MMU incured a page fault */
	CPU_TERM = CPU_PF << 1,	/**< Process execution termination */
	CPU_EX = CPU_TERM << 1,	/**< Process exception */
};


/** Very Simple CPU
 *
 *	This cpu is only used to abstract the
 *	actual reading/execution of the programs
 *	and the use of the MMU. This cpu is not meant
 *	to mimic the functionality of the cpu from the
 *	first project.
 */
class cCPU {
	private:
		sProc* curProc;

		cMMU mmu;

		string opCode;
		string addr;

	public:
		cCPU();
		~cCPU();

		void switchProc(sProc*);

		uint8_t run();
};

#endif // CPU_H_INCLUDED
