#ifndef CLOCK_DEVICE_H_INCLUDED
#define CLOCK_DEVICE_H_INCLUDED

#include "devices/abstract_device.h"

class ClockDevice: public AbstractDevice {
	private:

	public:
		ClockDevice();
		~ClockDevice();

		void setTimer(int usec);
};

#endif // CLOCK_DEVICE_H_INCLUDED
