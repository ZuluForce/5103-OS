#include "io_control.h"

extern INIReader* settings;
extern FILE* logStream;

cIOControl::cIOControl(int numProcs, int* VC) {
	io_out.resize(numProcs);
	io_in_wait.resize(numProcs);

	io_req_time = EXTRACTP(int, Timings, ioreq_time);
	io_time = EXTRACTP(int, Timings, pf_time);

	for ( int i = 0; i < numProcs; ++i) {
		io_in_wait.at(i) = NULL;
	}

	this->VC = VC;

	return;
}

cIOControl::~cIOControl() {
	return;
}

void cIOControl::removeWait(uint32_t pid) {
	sIOContext* ctx;

	ctx = io_in_wait.at(pid);
	io_in_wait.at(pid) = NULL;

	if ( ctx == NULL ) {
		cerr << "Output finished and tried to release non-existent pending input" << endl;

		return;
	}

	ctx->time = io_time;
	if ( io_in.size() > 0 ) {
		ctx->time -= io_in.back()->time;
	}

	cout << "***Adding Process " << ctx->pid << " to io_in queue from io_in_wait***" << endl;
	fprintf(logStream, "IO Control: Adding Process %d to io_in queue from io_in_wait\n", ctx->pid);
	fprintf(logStream, "\tI/O will complete when VC = %d\n", *VC + io_time);

	io_in.push(ctx);

	return;
}

sIOContext* cIOControl::getContext() {
	sIOContext* newContext;
	if ( context_cache.size() ) {
		newContext = context_cache.front();
		context_cache.pop();
	} else {
		newContext = (sIOContext*) malloc( sizeof(sIOContext) );
	}

	return newContext;
}

void cIOControl::returnContext(sIOContext* ctx) {
	assert(ctx != NULL);

	ctx->release = NULL;
	ctx->page = 0;
	ctx->pid = 0;
	ctx->time = -1;

	context_cache.push(ctx);
	return;
}

void cIOControl::tick(int times) {
	for ( int i = 0; i < times; ++i)
		tick();

	return;
}

void cIOControl::tick() {
	/* Take care of in-progress outputs */
	out_iter = io_out.begin();
	sIOContext* ctx;
	for(; out_iter != io_out.end(); ++out_iter) {

		/* Remove all I/O's that have 0 time left */
		while ( !((*out_iter).empty()) ) {
			ctx = (*out_iter).front();
			--(ctx->time);

			if ( ctx->time <= 0 ) {
				/* I/O finished. Remove and take care of the release (if any). */
				fprintf(logStream, "IO Control: I/O (output) finished\n");
				(*out_iter).pop();

				if ( ctx->release != NULL ) {
					removeWait(ctx->release->pid);
				}

				returnContext(ctx);
				continue; /* Continue checking for completions */
			}

			break; /* Fist in queue isn't finished. */
		}
	}

	while ( !io_in.empty()) {
		ctx = io_in.front();

		if ( --(ctx->time) <= 0 ) {
			cout << "***Pushing Process " << ctx->pid << " onto finished I/O queue***" << endl;
			fprintf(logStream, "IO Control: I/O(input) finished for process %d\n", ctx->pid);

			/* Do the funky bitshift to pack the <pid, frame> info */
			uint64_t fdata = ctx->pid;
			fdata = fdata << 32;
			fdata += ctx->page->frame;

			finished.push(fdata);
			finishedPTE.push(ctx->page);
			io_in.pop();

			ctx->page->flags[FI_PRESENT] = true;
			ctx->page->flags[FI_DIRTY] = false;
			ctx->page->flags[FI_REF] = false;

			// Set the timestamp to be the current virtual counter as this frame will be used shortly.
			ctx->page->timestamp = *VC;

			returnContext(ctx);
			continue;
		}

		break;
	}

	return;
}

sIOContext* cIOControl::scheduleIO(sProc* proc, uint32_t page, eIOType iotype, sIOContext* release) {
	assert(proc != NULL);
	/* Usually this is called each loop in the main loop
	 * of the core but since io_requests take some time
	 * we need to count it here.
	 */
    for (int i; i < io_req_time; i++){
        *VC = *VC + 1;
        tick();
    }

	//*VC += io_req_time;
	fprintf(logStream, "-*-Virtual counter = %d-*-\n", *VC);

	sIOContext* newContext = getContext();
	newContext->pid = proc->pid;
	newContext->page = &(proc->PTptr[page]);

	switch ( iotype ) {
		case IO_OUT: {
			newContext->release = release;

			queue<sIOContext*>& queueOut = io_out.at(proc->pid);

			if ( !queueOut.empty() ) {
				/* This way we only need to decrement the
				 * time for the first context in the queue and
				 * when it finishes the next one will have the
				 * appropriate time remaining.
				 */
				sIOContext* lastIO = queueOut.back();
				newContext->time = io_time - lastIO->time;
			} else {
				newContext->time = io_time;
			}

			fprintf(logStream, "Scheduling I/O (output)\n");
			fprintf(logStream, "\tMoving process %d's page %d out\n", proc->pid, page);
			fprintf(logStream, "\tI/O will complete when VC = %d\n", *VC + io_time);

			queueOut.push(newContext);
			break;
		}

		case IO_IN: {
			if ( !io_in.empty() ) {
				sIOContext* lastIO = io_in.back();

				newContext->time = io_time - lastIO->time;
			} else {
				newContext->time = io_time;
			}

			/* Should typically be NULL */
			newContext->release = release;

			/* Timekeeping for process */
			//proc->clockTime += 11;
			//The real meaning of the clock time is the
			//time spent on the cpu doing real work so we
			//don't count this

			cout << "***Adding Process " << proc->pid << " to io_in queue***" << endl;
			fprintf(logStream, "Scheduling I/O (input) for process %d\n", proc->pid);
			fprintf(logStream, "\tFetching Process' Page %d\n", page);
			fprintf(logStream, "\tI/O will complete when VC = %d\n", *VC + io_time);

			io_in.push(newContext);
			break;
		}

		case IO_IN_WAIT: {
			newContext->time = 0;

			newContext->release = release;

			if ( io_in_wait.at(proc->pid) != NULL ) {
				dumpIOError(proc);
			}

			io_in_wait.at(proc->pid) = newContext;

			/* Timekeeping for process */
			//proc->clockTime += 11;
			/*
			if ( release != NULL )
				proc->clockTime += release->time;
			else
				proc->clockTime += io_time;
			*/

			cout << "Added I/O to wait queue for process " << proc->pid << endl;
			fprintf(logStream, "Added I/O (input) to wait queue for process %d\n", proc->pid);

			break;
		}
	}

	return newContext;
}

void cIOControl::dumpIOError(sProc* proc) {
	stringstream errStream;

	errStream << "More than 1 I/O (page in) request waiting for process " << proc->pid << endl;
	errStream << "Shouldn't happen with demand paging" << endl;

	cIOExc ex = cIOExc();
	ex.setErrorStr(errStream.str());

	errStream.str(std::string());
	errStream.clear();

	errStream << "Trace of I/O (page in) vector: " << endl;
	vector<sIOContext*>::iterator it;
	for (it = io_in_wait.begin(); it != io_in_wait.end(); ++it) {
		if ( *it != NULL ) {
			errStream << "Process(" << (*it)->pid << ")--Page: " << (*it)->page;
			errStream << "Time: " << (*it)->time << endl;
		}
	}

	ex.setTrace(errStream.str());
	throw ((cIOExc) ex);
}
