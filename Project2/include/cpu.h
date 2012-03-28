#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

/** @file */

#include "mmu.h"
#include "data_structs.h"
#include "iniReader.h"
#include "utility/logger.h"

/** Status of the execution quantum termination */
enum eCPUState {
	CPU_OK = 0x1,				/**< CPU finished executing instruction ok */
	CPU_PF = CPU_OK << 1,		/**< CPU/MMU incured a page fault */
	CPU_TERM = CPU_PF << 1,		/**< Process execution termination */
	CPU_EX = CPU_TERM << 1,		/**< Process exception */
	CPU_CIRC_PF = CPU_EX << 1,	/**< Potential circular page fault detected by cpu */
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

		int instr_time, cs_time, quanta;
		int* VC;
		void incVC(int amnt);	/**< Increment the VC and print its value to trace file */

	public:
		cCPU();
		~cCPU();

		void addVC(int* VC);

		void switchProc(sProc*);

		uint32_t getFaultPage();
		uint8_t run();
};

/** @fn cCPU::addVC(int*)
 *	Give the cpu a reference to the core's VC
 *
 *	The VMM core calls this to provide a reference to the VC so the
 *	cpu can update it appropriately. This makes it easier than having
 *	some back and forth time management between the two systems.
 */

/** @fn cCPU::switchProc(sProc*)
 *	Context switch for this process.
 *
 *	There are a couple scenarios to consider here:
 *	\li parameter = NULL - Do Nothing
 *
 *	\li curProc=NULL
 *		Switches process
 *		Flushes TLB (Not really necessary but make sure)
 *		Set PTBR in MMU
 *
 *	\li curProc!=NULL
 *		Increment CS count for curProc
 *		Flush TLB
 *		Set PTBR for new proc
 *		Increment the VC by the cs_switch time (default 5)
 */

/** @fn cCPU::getFaultPage()
 *	Get the page number that faulted.
 *
 *	On a page fault the VMM core will queury here to find
 *	out which page needs to be brough in. This function
 *	simply calls the mmu for this information.
 */

/**	@fn cCPU::run()
 *	Start executing the current process
 *
 *	Once a process has been switched onto the cpu this is
 *	called to start execution. The process executes for the
 *	quanta specified in the .ini file/s. If the process page
 *	faults before this quanta it returns with a special staus
 *	code.
 */

#include "vmm_core.h"

#endif // CPU_H_INCLUDED
