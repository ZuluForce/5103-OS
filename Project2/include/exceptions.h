#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

/** @file */

/** Distinguish between exceptions in a generic class
 *
 *	Since we didn't really have time to expand the
 *	exception system for this project this never really
 *	expanded.
 */
enum eExType {
	PR_NO_FRAMES_AVAIL, /**< No frames are available. Thrown by PRModule */
};

/** Generic exception class
 *
 *	These exceptions are caught in main and their
 *	error messages are printed out.
 */

class cException {
	private:
		string errMsg;

		bool fatal;

	public:
		void setErrorStr(const string& msg) { errMsg = msg; };

		/** Is this a fatal error?
		 *
		 *	After this is set you must press up, up,
		 *	down, down, left, right, left, right, B, A.
		 *	Hopefully then the program will fix itself. No
		 *	really, you should end the program.
		 */
		void setFatality(bool f) { fatal = f; };
		string& getErrorStr() { return errMsg; };
		bool isFatal() { return fatal; };
};

/** A class handling exceptions in the VMM Core */
class cVMMExc: public cException {
	private:
		string dumpInfo;
		eExType type;

	public:
		void setDump(const string& dump) { dumpInfo = dump; };
		string& getDump() { return dumpInfo; };

		void setType(eExType _type) { type = _type; };
};

/** Class handling exceptions in the I/O system */
class cIOExc: public cException {
	private:
		string IO_Data_Trace;

	public:
		void setTrace(const string& trace) { IO_Data_Trace = trace; };
		string& getTrace() { return IO_Data_Trace; };
};

/** Exception specific to the PR module*/
class cPRExc: public cException {
	private:
		string name;

	public:
		void setName(const string& _name) { name = _name; };
};

#endif // EXCEPTIONS_H_INCLUDED
