#ifndef LOTTERY_H_INCLUDED
#define LOTTERY_H_INCLUDED

/** @file */

#include <assert.h>
#include <pthread.h>
#include <time.h>
#include "scheduler/scheduler.h"
#include "utility/id.h"

#define DEF_BLOCK_VEC_SIZE 4

using namespace std;

/** Lottery Scheduler */
class cLottery: public cScheduler {
private:
	vector<ProcessInfo*> readyVector;
	vector<ProcessInfo*> blockedVector;

	queue<pidType> traceUnblocked;

	int totalReady;
	int totalBlocked;
	int totalTickets;

	ProcessInfo* runningProc;

	pthread_mutex_t blockedLock;
	pthread_cond_t allBlocked;

	cIDManager blockedID;
	cIDManager readyID;

	FILE* logStream;
	cProcessLogger* procLogger;

public:
	cLottery();
	~cLottery();

	void initProcScheduleInfo(ProcessInfo*);
	void addProcess(ProcessInfo*);
	void setBlocked(ProcessInfo*);
	void unblockProcess(ProcessInfo*);
	void removeProcess(ProcessInfo*);

	ProcessInfo* getNextToRun();

	pidType numProcesses();

	void addLogger(FILE* _logStream);
	void addProcLogger(cProcessLogger* _procLogger);

	void printUnblocked();

};

/** Struct containing process info specific for Lottery scheduling */
struct lotteryInfo {
	unsigned int readyIndex;	/**< Index position in ready vector */
	unsigned int blockedIndex;	/**< Index position in blocked vector */
};

/* Doxygen knows to include the documentation for the methods in the
 * abstract class. Don't think I need to include it here.
 */

/** @fn void cLottery::addLogger(FILE* _logStream)
 *  Assign the file pointer to the logStream data field.
 */

/** @fn void cLottery::addProcLogger(cProcessLogger*)
 *  Assign the cProcessLogger pointer to the procLogger data field.
 */

/** @fn void cLottery::printUnblocked()
 *  Removes each PID from the traceUnblocked queue and prints it to the trace logger.
 */



#endif /* LOTTERY_H_ */
