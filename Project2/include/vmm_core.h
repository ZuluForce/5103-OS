#ifndef VMM_CORE_H_INCLUDED
#define VMM_CORE_H_INCLUDED

#include <vector>
#include <queue>
#include <string>
#include <inttypes.h>

#include "cpu.h"
#include "data_structs.h"
#include "iniReader.h"

using namespace std;

class cVMM {
	private:
		uint32_t numFrames;
		uint32_t PS;

		INIReader* settings;

		vector<sProc*>& procs;

		int currentProc;

		cCPU cpu;

	public:
		cVMM(INIReader* _settings, vector<sProc*>& _procs);
		~cVMM();

		int start();
};


#endif // VMM_CORE_H_INCLUDED
