#include "Policy/pr_fifo.h"

extern INIReader* settings;
extern cVMM* VMMCore;
extern FILE* logStream;

cPRFifo::cPRFifo(cFrameAllocPolicy& _FAPolicy)
: cPRPolicy(_FAPolicy), FAPolicy(_FAPolicy) {

	PTSize = EXTRACTP(int, Global, page_bits);
	PTSize = 1 << PTSize;

	return;
}

cPRFifo::~cPRFifo() {

	return;
}

void cPRFifo::resolvePageFault(sProc* proc, uint32_t page) {

	//Check for any open frames
	pair<bool,uint32_t> freeFrame = FAPolicy.getFrame(proc);

	if ( freeFrame.first ) {
		cout << "PR_FIFO: Found free frame (" << freeFrame.second << ") for page" << endl;
		fprintf(logStream, "PR_FIFO: Found Free Frame (%d) for page\n", freeFrame.second);

		/* Update the page table entry */
		proc->PTptr[page].frame = freeFrame.second;
		proc->PTptr[page].flags[FI_PRESENT] = true; //Set present bit

		//pageHist.push(&(proc->PTptr[page]));
		//pageOwners.push(proc->pid);

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
	sPTE* rPage;
	unsigned int ownerid;
	while ( true ) {
		rPage = pageHist.front();
		ownerid = pageOwners.front();

		if ( rPage == NULL ) {
			cVMMExc ex;
			ex.setErrorStr("PR_FIFO: Extracted NULL PTE from Fifo queue");
			ex.setDump("--");
			throw((cVMMExc) ex);
		}
		/* If a process was termintated its page isn't removed
		 * from the queue for efficiency reasons. */
		if ( !(rPage->flags[FI_PRESENT]) ) {
			cout << "Owner: " << ownerid << " Frame: " << rPage->frame << endl;
			pageHist.pop();
			pageOwners.pop();
			continue;
		}

		break;
	}

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

	//It's moving out no matter what
	rPage->flags[FI_PRESENT] = false;

	//Check if the page is dirty
	if ( rPage->flags[FI_DIRTY] ) {
		cout << "Spilling frame " << rPage->frame << " containing a dirty page belonging to Process " << ownerid << endl;
		fprintf(logStream, "PR_FIFO: Spilling frame %d containing a dirty page belonging to Process %d\n", rPage->frame, ownerid);
		sProc* owner = VMMCore->getProcess(ownerid);

		/* Spill old page */
		rPage->flags[FI_DIRTY] = false;

		/* Schedule the desired page to be read in after the
		 * frame's current page is spilled. */
		sIOContext* ctx = VMMCore->pageIn(proc, page, IO_IN_WAIT);

		/* Schedule I/O */
		VMMCore->pageOut(owner, rPage->frame, ctx);

		return;
	}

	fprintf(logStream, "PR_FIFO: Found non-dirty page frame %d\n", rPage->frame);
	VMMCore->pageIn(proc,page,IO_IN);
	return;
}

void cPRFifo::finishedIO(sProc* proc, sPTE* page) {
	assert(proc != NULL);
	assert(page != NULL);

	pageHist.push( page );
	pageOwners.push( proc->pid );

	FAPolicy.unpin(page->frame);
	//FAPolicy.getFrame(page->frame);

	return;
}


void cPRFifo::finishedQuanta(sProc* proc) {
	assert(proc != NULL);
	/* For LRU you would want to do something with
	 * the reference bit here */

	return;
}

void cPRFifo::clearPages(int numPages) {
	assert(numPages >= 0);

	if ( numPages > pageHist.size() ) {
		cVMMExc ex;
		stringstream stream;
		stream << "Tried to clear more pages than are occupying frame" << endl;
		stream << "Check that the threshold for the daemon is less than the total frame count" << endl;
		stream << "and that the cleanup amount is less than total memory" << endl;
		ex.setErrorStr(stream.str());
		ex.setFatality(false);

		stream.str(std::string());
		stream.clear();

		stream << "Occupied Frames = " << pageHist.size() << " numPages (requested cleared) = " << numPages << endl;
		ex.setDump(stream.str());
		throw((cVMMExc) ex);
	}

	sPTE* rmPage;
	unsigned int owner;

	int removed = 0;
	/* Clear pages in a fifo order. Just take from the queue */
	while ( removed < numPages ) {
		rmPage = pageHist.front();
		owner = pageOwners.front();
		pageHist.pop();
		pageOwners.pop();

		/* Modify the PTE accordingly */
		rmPage->flags[FI_PRESENT] = false;
		rmPage->flags[FI_REF] = false; //Fifo doesn't care about this anyways

		fprintf(logStream, "Removing process %d's page from frame %d\n", owner, rmPage->frame);
		/* Scheudle I/O for the page if dirty */
		if ( rmPage->flags[FI_DIRTY] ) {
			fprintf(logStream, "\tPage is dirty. Scheduling page out.\n");
			VMMCore->pageOut(VMMCore->getProcess(owner), rmPage->frame);
			rmPage->flags[FI_DIRTY] = false;
		}

		FAPolicy.returnFrame(rmPage->frame);

		++removed;
	}
	if ( numPages > 0)
		fprintf(logStream, "###-------- Finished Clearing %d Frames --------###\n\n", numPages);

	return;
}

void cPRFifo::unpinFrame(uint32_t frame) {
	FAPolicy.unpin(frame);
}

void cPRFifo::returnFrame(uint32_t frame) {
	FAPolicy.returnFrame(frame);
}
