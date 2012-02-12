#ifndef ABSTRACT_DEVICE_H_INC
#define ABSTRACT_DEVICE_H_INC

#include <time.h>
#include <signal.h>

class AbstractDevice {

	public:
		virtual void setTimer(int time) = 0;
		virtual void disarm() = 0;
};

#endif
