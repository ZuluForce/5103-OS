#include "mmu.h"

cMMU::cMMU(INIReader& settings) {
	/* Add default settings */
	settings.addDefault("MMU", "tlb_size", "64");

	return;
}

cMMU::~cMMU() {

}
