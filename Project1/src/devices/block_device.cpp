#include "devices/block_device.h"

cBlockDevice::cBlockDevice(int usec) {
	pthread_mutex_init(&deviceLock, NULL);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = BLOCKSIG;
	sev.sigev_value.sival_ptr = &timerid;

	if ( timer_create(QD_CLOCKID, &sev, &timerid) == -1 )
		throw ((string) "Failed to create timer: " + strerror(errno));

	iTime.it_value.tv_nsec = usec * 1000;
	iTime.it_value.tv_sec = 0;
	iTime.it_interval.tv_nsec = 0;
	iTime.it_interval.tv_sec = 0;

	return;
}

cBlockDevice::~cBlockDevice() {

	return;
}

void cBlockDevice::setDefaultTime(int usec) {
	iTime.it_value.tv_nsec = usec * 1000;
	iTime.it_value.tv_sec = 0;
}

void cBlockDevice::scheduleDevice(ProcessInfo* proc) {
	assert(proc != NULL);

	pthread_mutex_lock(&deviceLock);

	waitQueue.push(proc);

	if ( queueLength() == 1) {
		/* Schedule Interrupt */
		if ( timer_settime(timerid, 0, &iTime, NULL) == -1 )
			throw ((string) "Failed to start BlockDevice timer: " + strerror(errno));
	}

	pthread_mutex_unlock(&deviceLock);

	return;
}

ProcessInfo* cBlockDevice::timerFinished() {
	pthread_mutex_lock(&deviceLock);

	ProcessInfo* finishedProc = waitQueue.front();
	waitQueue.pop();

	if ( queueLength() > 0) {
		/* Schedule next interrupt */
		if ( timer_settime(timerid, 0, &iTime, NULL) == -1 )
			throw ((string) "Failed to start BlockDevice timer: " + strerror(errno));
	}

	pthread_mutex_unlock(&deviceLock);

	return finishedProc;
}

int cBlockDevice::queueLength() {
	return waitQueue.size();
}
