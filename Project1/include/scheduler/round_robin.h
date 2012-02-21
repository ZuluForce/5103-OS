#ifndef ROUND_ROBIN_H_INCLUDED
#define ROUND_ROBIN_H_INCLUDED

/** @file */

#include <assert.h>

#include "scheduler/scheduler.h"

#define DEF_BLOCK_VEC_SIZE 4
#define QUANTUM 4 // in clock ticks

using namespace std;

/** Round Robin Scheduler */
class cRoundRobin: public cScheduler {
	private:
        /* Internal Datastructures */
        queue<ProcessInfo*> readyQueue;
        vector<ProcessInfo*> blockedVector;

        /* This is just for remembering what processes
		 * have been unblocked so the next call to
		 * getNextToRun can print them to the trace file.
		 * This is because getNextToRun is called
		 * synchronously with other trace output.
		 */
		queue<pidType> traceUnblocked;

        int totalBlocked;

        int clockTicksUsed;

        ProcessInfo* runningProc;

        /* When all processes are blocked the scheduler waits
		 * on this condition variable
		 */
        pthread_mutex_t 	blockedLock;
		pthread_cond_t 		allBlocked;

        cIDManager blockedID;

        /* Logging */
		FILE* logStream;
		cProcessLogger* procLogger;

	public:
		cRoundRobin();
		~cRoundRobin();

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

/** Struct containing process info specific for Round-Robin scheduling.
    Has a blockedIndex which is the index into the blockedVector if the process
    becomes blocked.
    */
struct roundRobinInfo {
	unsigned int blockedIndex;
};

/** @fn void cRoundRobin::initProcScheduleInfo(ProcessInfo*)
*   Initializes a new roundRobinInfo struct.
*/

/** @fn void cRoundRobin::addProcess(ProcessInfo*)
*   Adds a process to the back of the readyQueue
*/

/** @fn void cRoundRobin::setBlocked(ProcessInfo*)
*   Adds a process to the blocked vector using an id created with the ID Manager.
*   Sets runningProc to NULL and the state of the process to blocked.
*   Sets clockTicksUsed to 0 as a new process will be picked to run.
*   Increment the count of blocked processes.
*/

/** @fn void cRoundRobin::unblockProcess(ProcessInfo*)
*   Sets the process to be ready again and adds it to the back of the queue.
*   Returns the blocked index id back to the ID Manager for future reuse.
*   Decrement the count of blocked processes.
*   Add the blocked process id to the traceUnblocked queue.
*/

/** @fn void cRoundRobin::removeProcess(ProcessInfo*)
*   Sets the process state to be terminated and resets clockTicksUsed to 0.
*   Frees the roundRobinInfo struct
*/

/** @fn ProcessInfo* cRoundRobin::getNextToRun()
*   If a proces is currently executing and clockTicksUsed is greater than the QUANTUM,
*   stop the current process from executing futher and add it to the back of the ready
*   queue. If clockTicksUsed is less than the QUANTUM, increment clockTicksUsed and return
*   the currently executing process to the kernel to keep running.
*   If a new process needs to be scheduled, grab the top of the ready queue
*   and increment clockTicksUsed.
*/

/** @fn pidType cRoundRobin::numProcesses()
*   Count up the number of processes in the readyQueue, proceses that are blocked, and
*   add 1 if a process is currently running.
*/

/** @fn void cRoundRobin::addLogger(FILE*)
*   Assign the file pointer to the logStream data field.
*/

/** @fn void cRoundRobin::addProcLogger(cProcessLogger*)
*   Assign the cProcessLogger pointer to the procLogger data field.
*/

/** @fn void cRoundRobin::printUnblocked()
*   Removes each PID from the traceUnblocked queue and prints it to the trace logger.
*/

#endif // ROUND_ROBIN_H_INCLUDED
