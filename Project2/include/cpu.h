#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "mmu.h"
#include "data_structs.h"
#include "iniReader.h"

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

		int quanta;

	public:
		cCPU(INIReader* settings);
		~cCPU();

		/** How many cycles should execute without interrupt?
		 *
		 *	Even though the actual round robin is implemented
		 *	in the VMM core I added this to avoid having to
		 *	return to the VMM every execution.
		 */
		void runTime(int _q) { quanta = _q; return;}

		void switchProc(sProc*);
};

#endif // CPU_H_INCLUDED
