#ifndef PR_FIFO_H_INCLUDED
#define PR_FIFO_H_INCLUDED

#include <queue>
#include "Policy/pageReplace.h"

using namespace std;

class cPRFifo: public cPRPolicy {
	private:
		//FIFO history of the pages
		queue<uint32_t> pageHist;

		cFrameAllocPolicy& FAPolicy;

	public:
		cPRFifo(cFrameAllocPolicy& _FAPolicy);
		~cPRFifo();

		const char* name() { return "fifo"; };
};

#endif // PR_FIFO_H_INCLUDED
