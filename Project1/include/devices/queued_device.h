#ifndef QUEUED_DEVICE_H_INCLUDED
#define QUEUED_DEVICE_H_INCLUDED

/** @file */

#include <time.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include "process.h"

#define QD_CLOCKID CLOCK_REALTIME
#define USEC_IN_SEC 1000000

class cAbsQueuedDevice {
	public:
		virtual void setDefaultTime(int usec) = 0;
		virtual void scheduleDevice(ProcessInfo*) = 0;

		virtual ProcessInfo* timerFinished() = 0;

		virtual int queueLength() = 0;
};

/** @fn virtual void setDefaultTime(int usec) = 0
 *	Set the default timer length for this device
 *
 *	Subsequent calls to schedule the device for a
 *	process should use this default time. This is
 *	convenience function. A class defined default
 *	can be defined statically or in the constructor.
 *
 *	@param int usec default microsecond length for
 *	device timer.
 */

/** @fn virtual void scheduleDevice(ProcessInfo*) = 0
 *	Schedule a process for a device interrupt.
 *
 *	If there are no other waiting processes then schedule
 *	the interrupt. If there is a pending interrupt then
 *	queue the process.
 *
 *	@param ProcessInfo* Process that wants to block on
 *	the device. This must be saved and returned to the
 *	kernel when its interrupt has been received.
 *
 *	@warning This must be synchronized with ::timerFinished
 *	because signal handlers will cause these methods to be
 *	called asynchronously.
 */

/** @fn virtual procPair timerFinished() = 0
 *	Called when a device timer has gone off
 *
 *	The signal handler in the kernel will call
 *	this method when a device's timer has
 *	completed. The method should then return
 *	the waiting process and then schedule an
 *	interrupt for the next device queued, if any.
 *
 *	@return ProcessInfo* Process that was waiting
 *	for the device I/O to complete. It will now be
 *	unblocked by the kernel/scheduler.
 *
 *	@warning Must be synchronized with ::scheduleDevice. Failing
 *	to do so could leave a process blocked indefinitely.
 */
#endif // QUEUED_DEVICE_H_INCLUDED
