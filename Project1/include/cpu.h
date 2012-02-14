#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

/** \file */ //So Doxygen documents global types

#include <cstdio>
#include <cstdlib>
#include <assert.h>
#include <inttypes.h>

#include "utility/logger.h"

#define MAX_PARAMS 2		/**< Max number of execution parameters for any Opcode */
#define MAX_PARAM_SIZE 256 	/**< Maximum size in bytes for an execution parameter. Creates exception if exceeded. */

/** \enum ePSW
 *	Enumeration of Program Status Word Flags
 *
 *	The program status word is a bit vector and this
 *	enumeration defines the meaning of particular bits.
 *	This is used in the interpretation of execution status.
 */
enum ePSW {
	PS_EXCEPTION = 0x1, 				/**< Executing process has created an exception */
	PS_TERMINATE = PS_EXCEPTION << 1, 	/**< Executing process has finished. */
	PS_SYSCALL = PS_TERMINATE << 1,		/**< Executing process has made a system call */
	PS_FINISHED = PS_SYSCALL << 1		/**< Executing process finished an instruction. No problems */
};

typedef unsigned int pidType;

class cCPU {
	private:
		/* 0 = User   1 = Kernel */
		bool KMode;

		/* max PC should be the size of the VAS */
		unsigned int PC, maxPC;
		int VC;
		uint16_t PSW; /**< Program Status Word */

        /* Text of executing process */
        char *execText; /**< Text data for currently executing process */

        /* Splits the current line, pointed to by PC
         * into individual tokens and passes back how many
         * there are
         */
		//If any added operations have more than 2 params change this
		char tokenBuffer[2][MAX_PARAM_SIZE]; 	/**< Holds the tokenized execution parameters */
		char Opcode; 							/**< Holds the current Opcode */
        int tokenizeLine();

		/* Logging */
		FILE* traceStream;
	public:
		pidType pid; //Only necessary for printint trace file

		cCPU();
		~cCPU();

		void initTraceLog();

		/* -------- Used by the kernel -------- */
		/** Set the program text
		 *
		 * Point the cpu to the text data for the running
		 * process. This text is indexed using the program
		 * counter (PC).
		 *
		 *	@param text
		 *		Program text pointer. assert( text != NULL)
		 */
        void setText(char* text);

		/** Set the cpu back into user mode
		 *
		 *	This is used by the kernel after the kernel has finished
		 *	servicing a process' kernel mode request (syscall).
		 */
        void setUserMode();

		/** Get/Set the program counter
		 *
		 *	Get the current value for the program counter
		 *	and then set its value to the given parameter. This
		 *	is useful for swapping out process values.
		 */
        unsigned int getSetPC(unsigned int newPC);

        /** Get/Set the VC
         *
         *	Get the current value for VC and set its value to
         *	the given paramter. This is useful for swapping out
         *	process values.
         */
        int getSetVC(int newVC);

		/** Get/Set the PSW
		 *
		 *	Get the current value for the PSW and set its value
		 *	to the given parameter.
		 */
        uint16_t getSetPSW(uint16_t newPSW);

        /** Get the Program Status Word
         *
         * Returns the program status word which is a unsigned
         * 16-bit integer type with flags from #ePSW set. These
         * are used by the kernel to make action decisions.
         */
        uint16_t getPSW();

        /** Set a new value for the PSW
         *
         *	Used by the kernel to reset the PSW after a system
         *	call. Any process execution which returns to the
         *	kernel but does not terminate the process should
         *	reset the PSW so subsequent exceptions/terminations
         *	are not lost by stray PSW values.
         */
        void setPSW(uint16_t newPSW);

        /** Get execution parameters from the cpu
         *
         *	Fetch the given execution paramter from the cpu's internal
         *	buffer. When an instruction is encountered that has parameters
         *	associated with it, the cpu tokenizes them and places it in an
         *	internal buffer. This function is mainly used by the kernel in
         *	handling system calls.
         *
         *	@param num
         *		Must be less than MAX_PARAMS (currently 2)
         *
         *	@return
		 *		Returns a char* which points to a string of at most MAX_PARAM_SIZE - 1 bytes.
         */
        char* getParam(int num);

        /** Get the current Opcode
         *
         * Get the current Opcode in the cpu. This is used by the kernel to
         *	determine which system call as being made. Used in conjunciton with
         *	cCPU::getParam the kernel can process system calls.
         */
        char getOpcode();

		/** Start execution
		 *
		 *	Once all appropriate process data is entered by the kernel this
		 *	function is called to start execution. Any time control needs to
		 *	be returned to the kernel this function will return with the
		 *	appropriate PSW flags set for the kernel to act on.
		 */
        void run();
        /* ------------------------------------ */
};

/** \class cCPU
 *	A class for emulating a simple cpu.
 *
 *	This class emulates the internals of a very simple cpu with
 *	two main registers, PC and VC. In addition, it has other state
 *	for handling system calls and program exceptions.
 */

#endif // CPU_H_INCLUDED
