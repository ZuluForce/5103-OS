#include "Policy/frameAlloc.h"

cFixedAlloc::cFixedAlloc() {
	/* ---- Set Defaults ---- */
	settings->addDefault("PM-Alloc-Fixed","alloc-size","20");

	allocSize = EXTRACTP(int,PM-Alloc-Fixed,alloc-size);

	uint32_t numFrames = EXTRACTP(uint32_t, Global, total_frames);

	dynamic_bitset<> frames(numFrames);
	frames.reset();

	dynamic_bitset<> pinned(numFrames);
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
	frames.set(frame, false);

	return;
}

bool cFixedAlloc::checkAvailable(uint32_t frame) {
	return frames.test(frame);
}

bool cFixedAlloc::pin(uint32_t frame) {
	if ( pinned.test(frame) )
		return false;

	pinned.set(frame);
	return true;
}

bool cFixedAlloc::unpin(uint32_t frame) {
	if ( !pinned.test(frame) )
		return false;

	pinned.flip(frame);
	return true;
}
