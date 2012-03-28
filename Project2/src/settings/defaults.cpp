#include "settings/defaults.h"


void setDefaults(INIReader* settings) {
	/* ---- Globals ---- */
	SETDP(Global,page_bits,20);
	SETDP(Global,offset_bits,12);

	SETDP(Global,total_frames,100);
	SETDP(Global,max_proc_frames,20);
	SETDP(Global,min_free_frames,10);

	/* ------ Timings ------ */
	SETDP(Timings,instr_time,1);
	SETDP(Timings,cs_time,5);
	SETDP(Timings,pf_time,10);
	SETDP(Timings,ioreq_time,1);
	SETDP(Timings,quanta,4);

	/* ------ MMU ------ */
	SETDP(MMU,tlb_size,64);
	SETDP(MMU,addr_type,hex);

	/* ------ Policy ------ */
	SETDP(Policy,FA,fixed);
	SETDP(Policy,PR,fifo);
	SETDP(Policy,clean_min,10);
	SETDP(Policy,cleanup_amnt,20);
	SETDP(Policy,print_frames,0);
	SETDP(Policy,frame_log, frames.trace);
	SETDP(Policy,ignore_circular_fault_warn,0);

	/* ------ Results ------ */
	SETDP(Results,file,stats.log);
	SETDP(Results,title,default);
	SETDP(Results,trace,results.trace);

	// Modules that are not known at compile time such as
	// the frame allocation and page replacement policies
	// should add their defualt settings at object creation
}
