#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

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

/**< A class handling exceptions in the VMM Core */
class cVMMExc: public cException {
	private:
		string dumpInfo;

	public:
		void setDump(const string& dump) { dumpInfo = dump; };
		string& getDump() { return dumpInfo; };
};

class cIOExc: public cException {
	private:
		string IO_Data_Trace;

	public:
		void setTrace(const string& trace) { IO_Data_Trace = trace; };
		string& getTrace() { return IO_Data_Trace; };
};

#endif // EXCEPTIONS_H_INCLUDED
