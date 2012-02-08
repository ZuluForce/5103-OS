#ifndef BLOCK_DEVICE_H_INCLUDED
#define BLOCK_DEVICE_H_INCLUDED

#include "devices/abstract_device.h"

class BlockDevice: public AbstractDevice {

	public:
		BlockDevice();
		~BlockDevice();

		void setTimer(int usec);
};

#endif // BLOCK_DEVICE_H_INCLUDED
