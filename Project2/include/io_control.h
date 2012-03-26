#ifndef IO_CONTROL_H_INCLUDED
#define IO_CONTROL_H_INCLUDED

#include <cassert>
#include <vector>
#include <queue>
#include "iniReader.h"
#include "exceptions.h"
#include "data_structs.h"
#include "utility/logger.h"

using namespace std;

/** I/O request type */
enum eIOType {
	IO_OUT,		/**< Output */
	IO_IN,		/**< Input */
	IO_IN_WAIT,	/**< Input that needs to wait for some other I/O */
};

/** Struct for holding the context of an I/O operation. */
struct sIOContext {
	uint32_t pid;	/**< PID of process that this I/O is for */
	sPTE* page;		/**< Page that is being either moved in or out */

	int time;		/**< Time remaining on this I/O */

	/** Release this waiting I/O when this one finishes */
	sIOContext* release;
};

/** Class representing an I/O controller */
class cIOControl {
	private:
		int io_req_time, io_time;
		/** What is being paged out?
		 *
		 * 	This vector is indexed by pid and it holds
		 *	a queue of the pages of the respective process
		 *	which are being paged out. This is so that in
		 *	the case that a process has a page spilled by
		 *	some other process' page fault or the cleaning
		 *	daemon, it is prevented from recovering the page
		 *	until it is successfully written out.
		 */
		vector<queue<sIOContext*> > io_out;
		vector<queue<sIOContext*> >::iterator out_iter;

		/** What is the next page in?
		 *
		 *	This keeps track of the next page to complete its
		 *	I/O.
		 */
		queue<sIOContext*> io_in;


		/** I/O in requests waiting for an io_out to finish */
		vector<sIOContext*> io_in_wait;
		vector<sIOContext*>::iterator in_wait_iter;

		/** Cache of previously allocated contexts.
		 *
		 *	This is simply a performance boost to avoid
		 *	constantly allocating/deallocating context structs.
		 */
		queue<sIOContext*> context_cache;

		/** Queue that holds process ids that have completed and I/O
		 *
		 *	This is specifically used for completed page ins. After calling
		 *	tick, any completed processes are placed in here and the VMM core
		 *	can read it and unblock.
		 *
		 *	I made it a 64-bit data type so that I didn't need to mess with
		 *	allocating and managing some other structure. The first 32-bits
		 *	represent the pid of the process whose I/O completed. The last
		 *	32-bits is the page that was brought in for the process. From this
		 *	the core can get the frame and unpin it.
		 */
		queue<uint64_t> finished;

		/** Holds the PTE of finished I/O in's */
		queue<sPTE*> finishedPTE;

		void removeWait(uint32_t);

		sIOContext* getContext();
		void returnContext(sIOContext* ctx);

		void dumpIOError(sProc*);

		int* VC; /** Reference virtual counter in core */

	public:
		cIOControl(int numProcs, int* VC);
		~cIOControl();

		queue<uint64_t>& getFinishedQueue() { return finished; };
		queue<sPTE*>& getFinishedPTEQueue() { return finishedPTE; };
		void tick();
		void tick(int times);

		/** Schedule an I/O event */
		sIOContext* scheduleIO(sProc* proc, uint32_t page, eIOType iotype, sIOContext* release = NULL);
};

/** @fn cIOControl::removeWait(uint32_t)
 *	Remove an I/O context from waiting.
 *
 *	When an I/O out completes, if its release context property
 *	is not null then the controller will try to release the
 *	specified context from the waiting vector.
 */

/** @fn cIOControl::getContext()
 *	Get a context struct
 *
 *	If there is a context struct cached from previous use then it is returned.
 *	Otherwise a new one is allocated.
 */

/** @fn cIOControl::returnContext(sIOContext*)
 *	Return a context struct for future use.
 *
 *	The struct is not deallocated. It is put into a cache queue.
 */

/** @fn cIOControl::dumpIOError(sProc*)
 *	Generate and throw IO Error
 *
 *	This does the work of generating an IO excetion.It was originally
 *	intended to work broadly for many different scenarios but the
 *	necessity was not there. It works but it produces a fairly generic
 *	error.
 */

/** @fn cIOControl::getFinishedQueue()
 *	Get a reference to the finished queue.
 *
 *	This is called by the core to get a refernce to this specific
 *	queue so it can be used in processing finished I/O. This queue
 *	hold 64-bit integers representing the pid and page of for the
 *	completed I/O.
 */

/** @fn cIOControl::getFinishedPTEQueue()
 *	Get a reference to the finished PTE queue
 *
 *	This is called by the core to get a reference to this specific
 *	queue to be used in processing finished I/O. The purpose for
 *	this queue was that the VMM core needed the frame of the
 *	completed I/O so it could be unpinned. Given the PTE in this
 *	queue it makes page data in the finished queue redundant.
 */

/** @fn cIOControl::tick()
 *	Count 1 time unit
 *
 *	Thiscounts 1 virtual time unite and modifies the remaining time
 *	on pending I/O correspondingly.
 */

/** @fn cIOControl::tick(int)
 *
 *	This executes ::tick() N number of times. This is useful in
 *	recording virtual time with events that take chunks of time
 *	such as the context switching of a process.
 *
 *	@param int Number of ticks to execute.
 */

/** @fn cIOControl::scheduleIO(sProc* proc, uint32_t page, eIOType iotype, sIOContext* release = NULL)
 *	Schedule I/O
 *
 *	This is used to schedule some I/O. The type is specified by the eIOType parameter. If the type
 *	is eIOType::IO_IN_WAIT it will be put in a vector (indexed by pid) waiting to be released.
 *	If the type is eIOType::IO_OUT and the sIOContext* != NULL then after the I/O completes
 *	it will relese the I/O pointed to by this context.
 */

#endif // IO_CONTROL_H_INCLUDED
