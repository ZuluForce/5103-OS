#include "Policy/frameAlloc.h"

extern INIReader* settings;
extern FILE* logStream;

cFixedAlloc::cFixedAlloc() {
	/* ---- Set Defaults ---- */
	settings->addDefault("PM-Alloc-Fixed","alloc-size","20");

	allocSize = EXTRACTP(int,PM-Alloc-Fixed,alloc-size);

	numFrames = EXTRACTP(uint32_t, Global, total_frames);
	assert(numFrames > 0);

	frames = dynamic_bitset<>(numFrames);
	frames.reset();

	pinned = dynamic_bitset<>(numFrames);
	pinned.reset();
}

cFixedAlloc::~cFixedAlloc() {

	return;
}

void cFixedAlloc::regProcs(vector<sProc*>& procs) {
	return;
}

uint32_t cFixedAlloc::findFirstOf(bool check, dynamic_bitset<>& bit) {
	for ( int i = 0; i < bit.size(); ++i ) {
		if ( bit[i] == check )	return i;
	}

	return bit.size() + 1;
}

pair<bool,uint32_t> cFixedAlloc::getFrame() {
	//Find first free
	uint32_t firstFree = findFirstOf(false, frames);

	if ( firstFree == frames.size() + 1 )
		return make_pair(false, 0);

	frames.set(firstFree);

	return make_pair(true, firstFree);
}

pair<bool,uint32_t> cFixedAlloc::getFrame(sProc* proc) {
	return getFrame();
}

bool cFixedAlloc::getFrame(uint32_t frame) {
	//Find first free
	uint32_t firstFree = findFirstOf(false, frames);
	if ( firstFree == frames.size() + 1 )
		return false;

	frames.set(firstFree);

	return true;
}

void cFixedAlloc::returnFrame(uint32_t frame) {
	assert(frame < numFrames);

	frames.set(frame, false);

	return;
}

bool cFixedAlloc::checkAvailable(uint32_t frame) {
	if ( frame >= numFrames ) return false;

	return frames.test(frame);
}

bool cFixedAlloc::pin(uint32_t frame) {
	assert(frame < numFrames);

	if ( pinned.test(frame) )
		return false;

	fprintf(logStream, "FA Policy: Pinning frame %d\n", frame);
	pinned.set(frame);
	return true;
}

bool cFixedAlloc::unpin(uint32_t frame) {
	assert( frame < numFrames );

	if ( !pinned.test(frame) )
		return false;

	fprintf(logStream, "FA Policy: Unpinnig frame %d\n", frame);
	pinned.flip(frame);
	return true;
}
