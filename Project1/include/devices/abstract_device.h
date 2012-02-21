#ifndef ABSTRACT_DEVICE_H_INC
#define ABSTRACT_DEVICE_H_INC

#include <time.h>
#include <signal.h>

/** An abstract device class
 *
 *	This was initially gonig to ge the abstract class
 *	for all devices such as the clock, character and block
 *	devices but it turned out this was not the desired
 *	interface for everything. This is why the
 *	cAbsQueuedDevice class was made. This class remains
 *	here because it is used by the clock device however
 *	it is no longer necessary.
 */
class AbstractDevice {

	public:
		virtual void setTimer(int time) = 0;
		virtual void disarm() = 0;
};

/** @fn virtual void AbstractDevice::setTimer(int time) = 0
 *	Set the length of the timer for this device
 *
 *	@param int time It is up to the implementation what the
 *	scale for this time is. It will likely be milliseconds
 *	or more.
 */

/** @fn virtual void AbstractDevice::disarm() = 0
 *	Disarm the timer set from a previous call to #setTimer
 */
#endif
