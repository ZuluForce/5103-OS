#ifndef DATA_STRUCTS_H_INCLUDED
#define DATA_STRUCTS_H_INCLUDED

#include <vector>
#include <string>
#include <inttypes.h>

using namespace std;

struct sPTE {
	uint32_t frame;

	/** Page Table Entry (PTE) VM flags
	 *
	 *	flags[0]: Present/absent
	 *	flags[1]: Dirty
	 *	flags[2]: Referenced
	 *	flags[3]: ---
	 */
	char flags[4];
};

struct sProc {
	uint16_t cswitches;
	string data;

	sPTE* PTptr;
};

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
