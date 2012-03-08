#ifndef MMU_CPP_INCLUDED
#define MMU_CPP_INCLUDED

#include "iniReader.h"

class cMMU {
	private:
		uint32_t ptbr;
		uint32_t ptlr;

	public:
		cMMU(INIReader& settings);
		~cMMU();

		void setPTBR(uint32_t val);
		void setPTLR(uint32_t val);
};

#endif // MMU_CPP_INCLUDED
