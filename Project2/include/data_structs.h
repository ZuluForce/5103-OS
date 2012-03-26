#ifndef DATA_STRUCTS_H_INCLUDED
#define DATA_STRUCTS_H_INCLUDED

/** @file */

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <inttypes.h>

using namespace std;

/** Enum used for indexing PTE flags */
enum eFlagIndex {
	FI_PRESENT, /**< Access PTE present bit */
	FI_DIRTY,	/**< Access PTE dirty bit */
	FI_REF,		/**< Access PTE reference bit */
};

/** A struct representing a single page table entry */
struct sPTE {
	uint32_t frame; /**< Frame that this PTE maps to */

    /** This gets set in hardware by the MMU */
	int timestamp;

	/** Page Table Entry (PTE) VM flags
	 *
	 *	flags[0]: Present/absent
	 *	flags[1]: Dirty
	 *	flags[2]: Referenced
	 */
	bool flags[3];

    /** This is set in software by the pr_lruApprox after each quanta
     * or on a page fault.
     */
	uint8_t time;
};

struct sPTEOwner {
	uint32_t pid;
	sPTE* page;
};

/** Struct representing a process */
struct sProc {
	unsigned int pid;				/**< Process PID */
	uint16_t cswitches;				/**< # of Context Switches */
	int pageFaults, tlbhit, tlbmiss;
	int clockTime;					/**< Time spent executing */

	istringstream* data;			/**< Text data of process */
	int PC;							/**< Program Counter */
	int maxPC;						/**< Max PC. Based on size of process file */

	bool restart;					/**< Used to flag an instruction restart */
	int repeatedFaults;				/**< Number of repeated page faults */
	string rline;					/**< Process instruction for the restart */

	sPTE* PTptr;					/**< Pointer to process' page table */

	void* scheduleData;				/**< Scheduler specific data. Always round robin for this project */
};

/* ---- Control Structs for parsing options ---- */
/** Struct for holding a settings overrides from command line */
struct sOpOverride {
	string section;
	string option;
	string newValue;
};

/** All the settings files, traces and setting overrides from command line */
struct sCmdOptions {
	vector<string> settingFiles;

	vector<string> traceFiles;

	vector<sOpOverride*> overrides;
};

#endif // DATA_STRUCTS_H_INCLUDED
