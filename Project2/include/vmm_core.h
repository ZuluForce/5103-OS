#ifndef VMM_CORE_H_INCLUDED
#define VMM_CORE_H_INCLUDED

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

class cVMM {
	private:
		/* ---- Virtual Memory Info ---- */
		uint32_t numFrames;
		uint32_t PS;
		uint32_t PT_Size;

		/* ----- Processes ----- */
		vector<sProc*>& procs;
		cRoundRobin scheduler;

		int currentProc;


		/* ----- Execution Machinery ----- */
		cCPU cpu;
		int VC; /**< Virtual Counter */


		/* ------ I/O Control ------ */
		cIOControl* ioCtrl;
		uint32_t pageInCount, pageOutCount;

		/* ------ Policy Modules ------ */
		cPRPolicy& PRModule;
		cCleanDaemon& cDaemon;

		void initProcesses();
		void cleanupProcess(sProc* proc);

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

enum eResOptions {
	/* G_[option]: Global option
	 * L_[option]: Local (per-process) option
	 */

	 G_CS,
	 G_PF,
	 G_ET,

	 L_CS,
	 L_PF,
	 L_ET,
};


#endif // VMM_CORE_H_INCLUDED
