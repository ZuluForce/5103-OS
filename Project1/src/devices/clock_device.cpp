#include "devices/clock_device.h"

ClockDevice::ClockDevice() {
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = CLOCKSIG;
	sev.sigev_value.sival_ptr = &timerid;

	if ( timer_create(CLOCKID, &sev, &timerid) == -1 )
		throw ((string) "Failed to create timer: " + strerror(errno));

	iDisarm.it_value.tv_nsec = 0;
	iDisarm.it_value.tv_sec = 0;
	iDisarm.it_interval.tv_nsec = 0;
	iDisarm.it_interval.tv_sec = 0;

	return;
}


ClockDevice::~ClockDevice() {

	return;
}


void ClockDevice::setTimer(int usec) {
	/* Setup an interrupt timer */
	iTime.it_value.tv_nsec = usec * 1000;
	iTime.it_interval.tv_nsec = usec * 1000;

	if ( timer_settime(timerid, 0, &iTime, NULL) == -1 )
		throw ((string) "Failed to start timer: " + strerror(errno));

	return;
}

void ClockDevice::disarm() {
	if ( timer_settime(timerid, 0, &iDisarm, NULL) == -1 )
		throw ((string) "Failed to disarm timer: " + strerror(errno));
}

int ClockDevice::getTime() {
	if ( timer_gettime(timerid, &iProbe) == -1 )
		return -1;

	return iProbe.it_value.tv_nsec / 1000;
}
