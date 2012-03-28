#include "Policy/pr_fifo.h"

extern INIReader* settings;
extern cVMM* VMMCore;
extern FILE* logStream;

void cPRLru::printTimestamps(){
    sPTEOwner *curPTEOwner;
    list<sPTEOwner*>::iterator it;
    it = pageHist.begin();
    while (it != pageHist.end()){
        curPTEOwner = *it;
        fprintf(logStream, "Process %d: Has a PTE with frame %d, timeStamp: %d\n", curPTEOwner->pid, curPTEOwner->page->frame, curPTEOwner->page->timestamp);
        ++it;
    }

}

/** Compare the timestamp field in both structs and return true if the
*   first struct should come before the second one.
*/
bool compare_timestamps (sPTEOwner *first, sPTEOwner *second){
    if (first->page == NULL){
        cVMMExc ex;
        ex.setErrorStr("PR_LRU: Found a null page during daemon cleaning");
        ex.setDump("--");
        throw((cVMMExc) ex);
    }
    if (first->page->timestamp <= second->page->timestamp){
        return true;
    }
    return false;
}

cPRLru::cPRLru(cFrameAllocPolicy& _FAPolicy)
: cPRPolicy(_FAPolicy), FAPolicy(_FAPolicy) {

	PTSize = EXTRACTP(int, Global, page_bits);
	PTSize = 1 << PTSize;

	return;
}

cPRLru::~cPRLru() {

	return;
}

sPTEOwner* cPRLru::getPTEOwner() {
	sPTEOwner* newPTEOwner;
	if ( pteowner_cache.size() ) {
		newPTEOwner = pteowner_cache.front();
		pteowner_cache.pop();
	} else {
		newPTEOwner = (sPTEOwner*) malloc( sizeof(sPTEOwner) );
	}

	return newPTEOwner;
}

void cPRLru::returnPTEOwner(sPTEOwner* pteOwner){
    assert(pteOwner!= NULL);
    pteOwner->pid = 0;
    pteOwner->page = 0;
    pteowner_cache.push(pteOwner);
}

ePRStatus cPRLru::resolvePageFault(sProc* proc, uint32_t page) {

	//Check for any open frames
	pair<bool,uint32_t> freeFrame = FAPolicy.getFrame(proc);

	if ( freeFrame.first ) {
		cout << "PR_LRU: Found free frame (" << freeFrame.second << ") for page" << endl;
		fprintf(logStream, "PR_LRU: Found Free Frame (%d) for page\n", freeFrame.second);

		/* Update the page table entry */
		proc->PTptr[page].frame = freeFrame.second;
		proc->PTptr[page].flags[FI_PRESENT] = true; //Set present bit

		//pageHist.push(&(proc->PTptr[page]));
		//pageOwners.push(proc->pid);

		//schedule process for io_in
		VMMCore->pageIn(proc, page, IO_IN);
		FAPolicy.pin(freeFrame.second);


		return PR_SERVICED;
	}

	if ( pageHist.size() == 0 ) {
		cerr << "An error has occured!!" << endl;
		cerr << "There are no open frames and none to spill. ";
		cerr << "Was the VMM initialized with 0 global frames?" << endl;

		return PR_NO_AVAIL;
	}

	//Get the page_owner to replace


	int min = INT_MAX;
	list<sPTEOwner*>::iterator it, min_it;
	min_it = pageHist.begin();
    sPTEOwner *curPTEOwner;
    sPTEOwner *minPTEOwner = pageHist.front();
    min_it = pageHist.begin();
    it = pageHist.begin();
    while (it != pageHist.end()){
        curPTEOwner = *it;

        if ( curPTEOwner->page == NULL ) {
			cVMMExc ex;
			ex.setErrorStr("PR_LRU: Extracted NULL PTE from list");
			ex.setDump("--");
			throw((cVMMExc) ex);
		}
        if (!(curPTEOwner->page->flags[FI_PRESENT])){
		    cout << "Owner: " << curPTEOwner->pid << " Frame: " << curPTEOwner->page->frame << endl;
		    pageHist.erase(it++);
		    returnPTEOwner(curPTEOwner);
		    continue;
		}

        if (curPTEOwner->page->timestamp < min){
            minPTEOwner = curPTEOwner;
            min = curPTEOwner->page->timestamp;
            min_it = it;
        }
        ++it;
    }

    // The entire list consisted of pages with non-present frames.
    if (!(minPTEOwner->page->flags[FI_PRESENT])){
        cVMMExc ex;
        ex.setErrorStr("PR_LRU: No valid frames for replacement in list");
        ex.setDump("--");
        throw((cVMMExc) ex);
    }

	pageHist.erase(min_it);
    sPTE *minPTE = minPTEOwner->page;
    unsigned int minOwner = minPTEOwner->pid;

	proc->PTptr[page].frame = minPTE->frame;
	FAPolicy.pin(minPTE->frame);

	//It's moving out no matter what
	minPTE->flags[FI_PRESENT] = false;

	//Check if the page is dirty
	if ( minPTE->flags[FI_DIRTY] ) {
		cout << "Spilling frame " << minPTE->frame << " containing a dirty page belonging to Process " << minOwner << endl;
		fprintf(logStream, "PR_LRU: Spilling frame %d containing a dirty page belonging to Process %d\n", minPTE->frame, minOwner);
		sProc* owner = VMMCore->getProcess(minOwner);

		/* Spill old page */
		minPTE->flags[FI_DIRTY] = false;

		/* Schedule the desired page to be read in after the
		 * frame's current page is spilled. */
		sIOContext* ctx = VMMCore->pageIn(proc, page, IO_IN_WAIT);

		/* Schedule I/O */
		VMMCore->pageOut(owner, minPTE->frame, ctx);

		return PR_SERVICED_IO;
	}

	fprintf(logStream, "PR_LRU: Found non-dirty page frame %d\n", minPTE->frame);
	VMMCore->pageIn(proc,page,IO_IN);
	return PR_SERVICED_IO;
}

void cPRLru::finishedIO(sProc* proc, sPTE* page) {
	assert(proc != NULL);
	assert(page != NULL);
    sPTEOwner *hist = getPTEOwner();
    hist->pid = proc->pid;
    hist->page = page;
	pageHist.push_back( hist );

	FAPolicy.unpin(page->frame);

	return;
}

void cPRLru::finishedQuanta(sProc* proc) {
	assert(proc != NULL);
	return;
}

bool cPRLru::clearPages(int numPages) {
	assert(numPages >= 0);

	if ( numPages == 0 ) return false;

	if ( numPages > pageHist.size() ) {
		cVMMExc ex;
		stringstream stream;
		stream << "Tried to clear more pages than are occupying frame" << endl;
		stream << "Check that the threshold for the daemon is greater than the total frame cout" << endl;
		ex.setErrorStr(stream.str());
		ex.setFatality(false);

		stream.str(std::string());
		stream.clear();

		stream << "Occupied Frames = " << pageHist.size() << " numPages (requested cleared) = " << numPages << endl;
		ex.setDump(stream.str());
		throw((cVMMExc) ex);
	}

	// Sort the list by time so minimum times are at the front
    	pageHist.sort(compare_timestamps);

	sPTE* rmPage;
	unsigned int owner;

	int removed = 0;
	/* Clear pages from the front of the sorted list. */
	while ( removed < numPages ) {
		rmPage = pageHist.front()->page;
		owner = pageHist.front()->pid;
		pageHist.pop_front();

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

	fprintf(logStream, "###-------- Finished Clearing %d Frames --------###\n\n", numPages);

	return true;
}

void cPRLru::unpinFrame(uint32_t frame) {
	FAPolicy.unpin(frame);
}

void cPRLru::returnFrame(uint32_t frame) {
	FAPolicy.returnFrame(frame);
}
