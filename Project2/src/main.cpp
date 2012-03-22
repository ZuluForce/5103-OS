#include "main.h"

using namespace std;

static cIDManager* pidGen;

FILE* logStream;

enum eParseState {
	PS_NONE = 0,
	PS_OVER = 0x1,
	PS_IFILE = PS_OVER << 1, /**< Currently parsing list of ini files */
	PS_TFILE = PS_IFILE << 1	 /**< Currently parsing list of trace files */
};

/** Matches command line strings for setting overrides
 *
 *	Rather than importing the boost regex library and
 *	having to depend on it being compiled on the test
 *	machine I decided to roll a simple check for this
 *	one command.
 *
 *	@return int 0: Okay  -1: No Match  < -1: Error
 */
int matchOverride(char* line, map<string, string>& match) {
	match["section"] = "";
	match["option"] = "";
	match["value"] = "";

	int matches = 0;
	char* origLine = line;

	while ( *line != '\0' && *line != '\n' ) {
		if ( *line == ':' ) {
			if ( matches >= 2 || (line - origLine) == 0) {
				/* This is an error because even though it
				 * wasn't a match, no filename can have ':'
				 * and therefore this can be for any other option
				 */
				return -2;
			}

			/* So the string constructor only takes
			 * this substring */
			*line = '\0';
			switch( matches ) {
				case 0:
					match["section"] = origLine;
					break;

				case 1:
					match["option"] = origLine;
					break;

				default:
					return -2;
			}

			origLine = line + 1;
			++matches;
		}

		++line;
	}

	/* We have matched the first 2, check if we scanned past a
	 * third. section:option:*value*
	 */
	if ( matches == 2) {
		if ( ( line - origLine ) == 0 )
			return -2; //Error!

		match["value"] = origLine;
		return 0;
	}

	if ( matches == 1 || matches > 2 ) {
		return -2; //Error!
	}

	return -1; //Means we found no ':' so it was just a no match
}

/**	Parses the command line for options.
 *
 *	Do not increment argc before passing it here.
 */
void parseCmdLine(int argc, char** argv, sCmdOptions* ops) {
	/* Parsing state */
	eParseState state = PS_NONE;

	/* Map for filling information
	 * about overridden options. Solely
	 * used for parsing.
	 */
	map<string,string> overmap;
	int error;

	sOpOverride* tempOver;

	for ( int i = 1; i < argc; ++i ) {
		error = matchOverride(argv[i], overmap);

		if ( error == -2 ) {
			cerr << "Error in parsing command line parameters" << endl;
			cerr << "Argument " << i - 1 << " is invalid" << endl;
			exit(-1);
		}

		if ( !error ) {
			cout << "Found setting override:" << endl;
			cout << "\tSection: " << overmap["section"] << endl;
			cout << "\tOption: " << overmap["option"] << endl;
			cout << "\tNew Value: " << overmap["value"] << endl;

			tempOver = new sOpOverride;
			tempOver->section = overmap["section"];
			tempOver->option = overmap["option"];
			tempOver->newValue = overmap["value"];

			ops->overrides.push_back(tempOver);

			state = PS_NONE;
			continue;
		}

		if ( strcmp(argv[i], "-t") == 0) {
			state = PS_TFILE;

			continue;
		}

		if ( strcmp(argv[i], "-s") == 0) {
			state = PS_IFILE;

			continue;
		}

		switch ( state ) {
			case PS_TFILE:
				cout << "Adding trace file: " << argv[i] << endl;
				ops->traceFiles.push_back(argv[i]);
				break;

			case PS_IFILE:
				cout << "Adding setting file: " << argv[i] << endl;
				ops->settingFiles.push_back(argv[i]);
				break;

			default:
				cerr << "Invalid parse state parseCmdLine." << endl;
				exit(-1);
				break;
		}

	}
}

sProc* loadProc(string& filename) {
	sProc* newProc = new sProc;
	newProc->pid = pidGen->getID();
	newProc->pageFaults = 0;
	newProc->tlbhit = newProc->tlbmiss = 0;
	newProc->clockTime = 0;
	newProc->cswitches = 0;
	newProc->restart = false;

    /* Read process contents into memory */
	struct stat fileinfo;
    if ( stat(filename.c_str(), &fileinfo) < 0 ) {
    	/* File likely doesn't exist */
    	fprintf(stderr, "Program trace %s does not exist \n", filename.c_str());

    	return NULL;
    }

    int file = open( filename.c_str(), S_IRUSR);

    char* buf = (char*) malloc( fileinfo.st_size );
    if ( read(file, buf, fileinfo.st_size) < fileinfo.st_size ) {
    	perror("Error reading program into memory");
    	return NULL;
    }

	newProc->data = new istringstream(buf);

	newProc->PC = 0;
	newProc->maxPC = fileinfo.st_size;

	close(file);
	free(buf);

	return newProc;
}

int main(int argc, char** argv) {
	if ( argc < 2 ) {
		fprintf(stderr, "Usage: %s [OPTIONS]\n", argv[0]);
		fprintf(stderr, "Opions:\n");
		fprintf(stderr, "\t-t [TRACE_FILE]+\n");
		fprintf(stderr, "\t-s [INI FILE]+\n");
		fprintf(stderr, "\t\"Override ini setting\" <section>:<option>:<value>\n");

		exit(-1);
	}

	sCmdOptions* options = new sCmdOptions;
	parseCmdLine(argc, argv, options);

	INIReader* reader = new INIReader();
	settings = reader;
	setDefaults(settings); //Add static defaults
	settings->addOverwriteException("Processes");

	pidGen = new cIDManager(0);

	vector<string>::iterator it;
	it = options->settingFiles.begin();

	/* Parse all the given settings files into the reader */
	for (; it != options->settingFiles.end(); ++it) {
		cout << "Loading settings file: " << *it << endl;
		reader->load_ini(*it);
	}

	vector<sProc*> processes;
	it = options->traceFiles.begin();

	/* Load all the process traces */
	for (; it != options->traceFiles.end(); ++it) {
		processes.push_back(loadProc(*it));

		if ( processes.back() == NULL )
			exit(-1);
	}


	stringstream settingsProc;
	//string settingsProc;
	string procName;

	/* Load processes specified in the settings files */
	for ( int i = 0; i < MAX_SETTINGS_PROCS; ++i) {
		settingsProc.str("");
		settingsProc << i;

		if( !reader->exists("Processes", settingsProc.str()) )
			break;

		//Get process 'i' from the settings file
		procName = reader->extractValue<string>("Processes", settingsProc.str());
		processes.push_back(loadProc(procName));

		if ( processes.back() == NULL ) {
			cout << "Loaded " << i << " processes from settings file/s" << endl;
			exit(-1);
		}
	}

	vector<sOpOverride*>::iterator ito;
	ito = options->overrides.begin();

	sOpOverride* to; //Temp override

	for (; ito != options->overrides.end(); ++ito) {
		to = *ito;
		if ( !reader->overWriteOp(to->section, to->option, to->newValue) ) {
			cerr << "Failed to override setting, likely doesn't exist: " << endl;
			cerr << "\tSection: " << to->section << endl;
			cerr << "\tOption: " << to->option << endl;
			cerr << "\tValue: " << to->newValue << endl;
		}

		delete to;
	}

	delete options;

	/* Must initialize the log after all settings files
	 * have been loaded. */
	initLog(EXTRACTP(string, Results, trace).c_str());
	logStream = getStream();

	/* Start up the VMM */
	try {
		cPRPolicy* pr_policy = NULL;
		cFrameAllocPolicy* fa_policy = NULL;
		cCleanDaemon* cDaemon = NULL;

		string FA_Type = EXTRACTP(string, Policy, FA);
		string PR_Type = EXTRACTP(string, Policy, PR);

		if ( FA_Type.compare("fixed") == 0 ) {
			fa_policy = new cFixedAlloc();
		} else {
			cerr << "Invalid Frame Allocator Type: " << FA_Type << endl;
			exit(-1);
		}

		assert(fa_policy != NULL);

		if ( PR_Type.compare("fifo") == 0 ) {
			pr_policy = new cPRFifo(*fa_policy);
		} else if ( PR_Type.compare("lru_approx") == 0 ) {
			pr_policy = new cPRLruApprox(*fa_policy);
		} else {
			cerr << "Invalid Page Replacement Type: " << PR_Type << endl;
			exit(-1);
		}

		assert(pr_policy != NULL);

		cDaemon = new cCleanDaemon(*fa_policy);
		cVMM* manager = new cVMM(processes, *pr_policy, *cDaemon);
		manager->start();
	} catch ( cVMMExc& error) {
		cout << "Caught VMM core error: " << endl;
		cout << "\tError Msg: " << error.getErrorStr() << endl;
		cout << "\tInfo Dump: " << error.getDump() << endl;

		closeLog();
		exit(-1);
	} catch ( cException& error ) {
		cout << "Caught Exception: " << endl;
		cout << "\tErr Msg: " << error.getErrorStr() << endl;

		closeLog();
		exit(-1);
	}

	closeLog();
	return 0;
}
