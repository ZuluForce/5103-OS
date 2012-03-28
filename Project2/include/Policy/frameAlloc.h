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
		uint32_t numFrames;

		virtual void printFrames() = 0;

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

		virtual uint32_t checkOpen(bool) = 0;

		virtual bool pin(uint32_t frame) = 0;
		virtual bool unpin(uint32_t frame) = 0;
};

/** A class implementing a simple fcfs style frame allocation
 *
 *	The namae "fixed alloc" is a bit of a misnomer because
 *	originally we though that each process would only be
 *	given a fixed number of frames. With the global demand
 *	paging policy this changed to simply being a class for
 *	managing which frames were open and whenever someone
 *	requests a page they are given one if it is available,
 *	regardless of how many they have gotten before.
 */
class cFixedAlloc: public cFrameAllocPolicy {
	private:
		int allocSize;
		dynamic_bitset<> frames;
		dynamic_bitset<> pinned;

		uint32_t numFrames;
		uint32_t openFrames;

		uint32_t findFirstOf(bool check, dynamic_bitset<>& bits);

		bool _printF;
		bool _printF_on_pin;
		void printFrames();
		FILE* printLoc;

	public:
		cFixedAlloc();
		~cFixedAlloc();

		void regProcs(vector<sProc*>& procs);

		bool checkAvailable(uint32_t frame, bool pinnedTaken = true);

		pair<bool,uint32_t> getFrame();
		pair<bool,uint32_t> getFrame(sProc*);
		bool getFrame(uint32_t frame);

		void returnFrame(uint32_t frame);

		uint32_t checkOpen(bool);

		bool pin(uint32_t frame);
		bool unpin(uint32_t frame);
};

/** @fn cFixedAlloc::pin(uint32_t)
 *	Pin a frame
 *
 *	A pinned frame cannot be given to anyone else.
 *	Usually this does not matter because the page
 *	is reserved first and therefore the frame allocator
 *	won't give it out again. There are scenarios where
 *	on a page fault they PR module may decide to replace
 *	a page which is still coming into memory. In this case
 *	the PR module would notice from this class that the
 *	frame is pinned so it can't spill it.
 *
 *	This scenario happens mostly in low memory situations.
 */

#endif // FRAMEALLOC_H_INCLUDED
