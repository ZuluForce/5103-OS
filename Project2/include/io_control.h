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

enum eIOType {
	IO_OUT,
	IO_IN,
	IO_IN_WAIT,
};

struct sIOContext {
	uint32_t pid;
	sPTE* page;

	int time;

	/** Release this waiting I/O when this one finishes */
	sIOContext* release;
};

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
		queue<sPTE*> finishedPTE;

		void removeWait(uint32_t);

		sIOContext* getContext();
		void returnContext(sIOContext* ctx);

		void dumpIOError(sProc*);

		int* VC;

	public:
		cIOControl(int numProcs, int* VC);
		~cIOControl();

		queue<uint64_t>& getFinishedQueue() { return finished; };
		queue<sPTE*>& getFinishedPTEQueue() { return finishedPTE; };
		void tick();
		void tick(int times);

		/** Schedule an I/O event */
		sIOContext* scheduleIO(sProc* proc, uint32_t page, eIOType iotype, sIOContext* release = NULL);

		void clearIOQueue(uint32_t pid);
		//void addPageOut(uint32_t pid, sPTE* page, int time);
		//void addPageIn(uint32_t pid, sPTE* page, int time);
};

#endif // IO_CONTROL_H_INCLUDED
