#include "Policy/pr_fifo.h"

extern cVMM* VMMCore;
extern FILE* logStream;


cPRFifo::cPRFifo(cFrameAllocPolicy& _FAPolicy)
: cPRPolicy(_FAPolicy), FAPolicy(_FAPolicy) {

	return;
}

cPRFifo::~cPRFifo() {

	return;
}

void cPRFifo::getPage(sProc* proc, uint32_t page) {
	//Check for any open frames
	pair<bool,uint32_t> freeFrame = FAPolicy.getFrame(proc);

	if ( freeFrame.first ) {
		fprintf(logStream, "Found Free Frame (%d) for page\n", freeFrame.second);

		/* Update the page table entry */
		proc->PTptr[page].frame = freeFrame.second;
		proc->PTptr[page].flags[FI_PRESENT] = true; //Set present bit

		pageHist.push(&(proc->PTptr[page]));
		pageOwners.push(proc->pid);

		//schedule process for io_in
		VMMCore->pageIn(proc, page, IO_IN);
		FAPolicy.pin(freeFrame.second);

		return;
	}

	if ( pageHist.size() == 0 ) {
		cerr << "An error has occured!!" << endl;
		cerr << "There are no open frames and none to spill. ";
		cerr << "Was the VMM initialized with 0 global frames?" << endl;
	}

	//Get the page to replace and its owner's pid
	sPTE* rPage = pageHist.front();
	unsigned int ownerid = pageOwners.front();

	pageHist.pop();
	pageOwners.pop();

	if ( rPage == NULL ) {
		cVMMExc ex;
		ex.setErrorStr("PR_FIFO: Page extracted from fifo queue is NULL");
		ex.setDump("--");

		throw ((cVMMExc) ex);
	}

	proc->PTptr[page].frame = rPage->frame;
	FAPolicy.pin(rPage->frame);

	//Check if the page is dirty
	if ( rPage->flags[FI_DIRTY] ) {
		fprintf(logStream, "Spilling frame %d contianing a dirty page belonging to Process %d\n", rPage->frame, ownerid);
		sProc* owner = VMMCore->getProcess(ownerid);

		/* Spill old page */
		rPage->flags[FI_DIRTY] = false;
		rPage->flags[FI_PRESENT] = false;

		/* Schedule I/O */
		sIOContext* ctx = VMMCore->pageOut(owner, rPage->frame);

		/* Schedule the desired page to be read in after the
		 * frame's current page is spilled. */
		VMMCore->pageIn(proc, page, IO_IN_WAIT, ctx);

		return;
	}

	fprintf(logStream, "Found non-dirty page frame %d\n", rPage->frame);
	VMMCore->pageIn(proc,page,IO_IN);
	return;
}


void cPRFifo::unpinFrame(uint32_t frame) {
	FAPolicy.unpin(frame);
}
