#ifndef VMM_CORE_H_INCLUDED
#define VMM_CORE_H_INCLUDED

#include <vector>
#include <string>
#include <inttypes.h>

#include "data_structs.h"
#include "iniReader.h"

using namespace std;

class cVMM {
	private:
		uint32_t max_frames;
		uint32_t PS;

		INIReader* settings;

		vector<sProc*>& procs;
	public:
		cVMM(INIReader* _settings, vector<sProc*>& _procs);
		~cVMM();
};


#endif // VMM_CORE_H_INCLUDED
