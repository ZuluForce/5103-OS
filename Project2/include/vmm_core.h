#ifndef VMM_CORE_H_INCLUDED
#define VMM_CORE_H_INCLUDED

/** @file */

#include <ctime>
#include <vector>
#include <queue>
#include <bitset>
#include <string>
#include <string.h>
#include <inttypes.h>
#include <sstream>
#include <boost/dynamic_bitset.hpp>

#include "cpu.h"
#include "data_structs.h"
#include "iniReader.h"
#include "exceptions.h"
#include "round_robin.h"
#include "io_control.h"
#include "Policy/allPR.h"
#include "utility/strConv.h"
#include "Policy/frameAlloc.h"
#include "Policy/cleaningDaemon.h"

using namespace std;
using namespace boost;

extern INIReader* settings;

/** Virtual Memory Manager core class */
class cVMM {
	private:
		/* ---- Virtual Memory Info ---- */
		uint32_t numFrames;	/**< Global memory frames */
		uint32_t PS;		/**< Page size in bytes */
		uint32_t PT_Size;	/**< Page table size (# of entries) */

		/* ----- Processes ----- */
		vector<sProc*>& procs;	/**< All processes. */
		cRoundRobin scheduler;	/**< Process scheduler */

		int currentProc;		/**< PID of current executing process */


		/* ----- Execution Machinery ----- */
		cCPU cpu;	/**< Executing CPU */
		int VC;		/**< Virtual Counter */


		/* ------ I/O Control ------ */
		cIOControl* ioCtrl; /**< I/O Controller */
		uint32_t pageInCount, pageOutCount;

		/* ------ Policy Modules ------ */
		cPRPolicy& PRModule;	/**< Page Replacement Module */
		cCleanDaemon& cDaemon;	/**< Cleaning Daemon */

		void initProcesses();
		void cleanupProcess(sProc* proc);
		void clearCircChecks();

		void printResults();

	public:
		cVMM(vector<sProc*>& _procs, cPRPolicy& _PRM, cCleanDaemon& _cDaemon);
		~cVMM();

		/* ------ I/O Control ------ */
		sIOContext* pageOut(sProc*,uint32_t,sIOContext* ctx = NULL);
		sIOContext* pageIn(sProc*, uint32_t, eIOType,sIOContext* ctx = NULL);
		void tickController(int times);

		sProc* getProcess(unsigned int id);
		int start();
};

/** @fn cVMM::cVMM(vector<sProc*>&, cPRPolicy&, cCleanDaemon&)
 *	Virtual Memory Manager Constructor
 *
 *	Initializes the Virtual Memory Manager class which is the core
 *	of the project.
 *
 *	@param vector<sProc*>& A vector contining the processes loaded in main.cpp.
 *	The page tables and other data are initialized during VMM initialization.
 *
 *	@param cPRPolicy& A reference to a derived class of cPRPolicy (fifo, pure lru, lru apporx).
 *
 *	@param cCleanDaemon& A reference to the cleaning daemon. This is passed in because it
 *	requires a reference to the frame allocation policy which is only available in main.cpp
 */

/** @fn cVMM::~cVMM()
 *	Virtual Memory Manager Destructor
 *
 *	Cleans up the VMM.
 */

/** @fn cVMM::start()
 *	Start running the processes.
 *
 *	After the FMM has been initialized with some processes this
 *	is called to start execution.
 */

/** @fn cVMM::getProcess(unsigned int id)
 *	Used to get a reference to the particular process struct.
 *
 *	In some places like the PR module or the io_controller there is
 *	sometimes a need to get a reference to the process when all you
 *	have is a pid.
 */

/** @fn cVMM::pageOut(sProc*,uint32_t,sIOContext*)
 *	Wrapper for the I/O controller (output scheduling)
 *
 *	This is a wrapper for the I/O controller. Calling this
 *	abstracts the actual I/O controller from those who will
 *	use it as well as providing a point for the VMM to record
 *	paging statistics.
 *
 *	@see cIOControl::scheduleIO
 */

/** @fn cVMM::pageIn(sProc*, uint32_t, eIOType,sIOContext* ctx = NULL)
 *	Wrapper for I/O controller (input scheduling)
 *
 *	Another wrapper for the I/O controller.
 *
 *	@see cIOControl::scheduleIO
 */

/** @fn cVMM::tickController(int)
 *	Tick the I/O controller n number of times.
 *
 *	This is used for timekeeping. For example, when wwe do a context
 *	switch on the cpu it takes 5 units of time but we still need to
 *	keep track of the passage of time within other devices which
 *	virtually should be running in parallel.
 *
 *	@see cIOcontrol::tick
 */

/** @fn cVMM::initProcesses()
 *	Initialize process data specific to the VMM
 *
 *	The main purpose of this function is to initialize the
 *	page table for all processes.
 *
 *	@see sProc
 */

/** @fn cVMM::cleanupProcess(sProc*)
 *	Cleanup process data and state
 *
 *	This function returns all occupied frames back to the system.
 *	After this it frees the memory used for the page table.
 *
 *	All other data is left in tact and it is not removed from the
 *	process vector because upon termination we need to gather
 *	statistics for each process for the results trace.
 *
 *	@see cPRPolicy::returnFrame
 */

/** @fn cVMM::clearCircChecks()
 *	Set the circular fault check flag to false for each proc
 *
 *	This addition ot the cpu is more of a safety net but as
 *	long as it is being used it needs to work correctly. This
 *	funtion is mainly called after the cleaning daemon clears
 *	pages so that the cpu doesn't think there is a circular
 *	fault when it was really just the cleaning daemon.
 */

/** @fn cVMM::printResults()
 *	Gather and print results to log file
 *
 *	After all execution is finished, this function gathers
 *	per-process and global data and prints it to a log file.
 *	This log can be read normally or used by the python script
 *	to build graphs (assuming a .conf is present).
 */

#endif // VMM_CORE_H_INCLUDED
