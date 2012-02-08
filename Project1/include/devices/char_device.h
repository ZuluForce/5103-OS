#ifndef CHAR_DEVICE_H_INCLUDED
#define CHAR_DEVICE_H_INCLUDED

#include "devices/abstract_device.h"

class CharDevice: public AbstractDevice {
	public:
		CharDevice();
		~CharDevice();

		void setTimer(int usec);
};

#endif // CHAR_DEVICE_H_INCLUDED
