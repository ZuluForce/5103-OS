#include "Policy/pr_fifo.h"


cPRFifo::cPRFifo(cFrameAllocPolicy& _FAPolicy)
: cPRPolicy(_FAPolicy), FAPolicy(_FAPolicy) {

	return;
}

cPRFifo::~cPRFifo() {

	return;
}


