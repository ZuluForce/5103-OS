#ifndef MMU_CPP_INCLUDED
#define MMU_CPP_INCLUDED

#include <inttypes.h>

#include "data_structs.h"
#include "iniReader.h"

/**< Struct representing a single entry in the TLB */
struct sTLBE {
	uint32_t VPN;
	uint32_t frame;

	/** Is this a valid translation?
	 *
	 *	If false, the MMU will ignore any cache
	 *	hits on this entry. This is used in flushing
	 *	the tlb.
	 */
	bool valid;
};

class cMMU {
	private:
		sTLBE* TLB;
		uint16_t tlbSize;

		/** For excavating old tlb entries we are simply
		 *	treating it as a ring buffer. This is the index
		 *	where new entries are being placed */
		uint32_t replaceIndex;

		/* -- Page Table Info --*/
		sPTE* ptbr;
		uint32_t ptlr;

		bool initialized;

	public:
		cMMU(INIReader* settings);
		~cMMU();

		void setPTBR(sPTE* _ptbr) { ptbr = _ptbr; };
		void setPTLR(uint32_t val) { ptlr = val; };

		void flushTLB();
};

#endif // MMU_CPP_INCLUDED
