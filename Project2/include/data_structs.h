#ifndef DATA_STRUCTS_H_INCLUDED
#define DATA_STRUCTS_H_INCLUDED

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <inttypes.h>

using namespace std;

enum eFlagIndex {
	FI_PRESENT,
	FI_DIRTY,
	FI_REF,
};

struct sPTE {
	uint32_t frame;

	/** Page Table Entry (PTE) VM flags
	 *
	 *	flags[0]: Present/absent
	 *	flags[1]: Dirty
	 *	flags[2]: Referenced
	 */
	bool flags[3];

	uint8_t time;
};

struct sProc {
	unsigned int pid;
	uint16_t cswitches;
	int pageFaults;
	int clockTime;

	istringstream* data;
	int PC;
	int maxPC;

	/* Used to redo a single instr */
	bool restart;
	string rline;

	sPTE* PTptr;

	void* scheduleData;
};

/* ---- Control Structs for parsing options ---- */
struct sOpOverride {
	string section;
	string option;
	string newValue;
};

struct sCmdOptions {
	vector<string> settingFiles;

	vector<string> traceFiles;

	vector<sOpOverride*> overrides;
};

#endif // DATA_STRUCTS_H_INCLUDED
