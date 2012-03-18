#ifndef FRAMEALLOC_H_INCLUDED
#define FRAMEALLOC_H_INCLUDED

#include <boost/dynamic_bitset.hpp>

#include "iniReader.h"
#include "data_structs.h"
#include "utility/logger.h"

using namespace std;
using namespace boost;

extern INIReader* settings;

/** An abstract class for allocating frames to processes.
 *
 *	The VMM core uses derived classes of this type to decide
 *	how many frames to give a process when it is created.
 */
class cFrameAllocPolicy {
	private:
		int numFrames;

	public:
		/** How many frames does the whole system have */
		void setNumFrames(int frames) { numFrames = frames; };

		/** Give the policy module information about processes
		 *
		 *	Some frame allocation policies may want information
		 *	regarding particular processes. This is called after
		 *	construction and it gives the policy a chance to
		 *	build any necessary datastructures.
		 */
		virtual void regProcs(vector<sProc*>& procs) = 0;

		virtual pair<bool,uint32_t> getFrame(sProc*) = 0;

		/** System is returning a page
		 *
		 *	This could happen on process termination or invocation
		 *	of the page cleaning daemon.
		 */
		virtual void returnFrame(uint32_t frame) = 0;

		virtual bool pin(uint32_t frame) = 0;
		virtual bool unpin(uint32_t frame) = 0;
};

class cFixedAlloc: public cFrameAllocPolicy {
	private:
		int allocSize;
		dynamic_bitset<> frames;
		dynamic_bitset<> pinned;

		uint32_t numFrames;

		uint32_t findFirstOf(bool check, dynamic_bitset<>& bits);

	public:
		cFixedAlloc();
		~cFixedAlloc();

		void regProcs(vector<sProc*>& procs);

		bool checkAvailable(uint32_t frame);

		pair<bool,uint32_t> getFrame();
		pair<bool,uint32_t> getFrame(sProc*);
		bool getFrame(uint32_t frame);

		void returnFrame(uint32_t frame);

		bool pin(uint32_t frame);
		bool unpin(uint32_t frame);
};

#endif // FRAMEALLOC_H_INCLUDED
