#ifndef INIREADER_H_INCLUDED
#define INIREADER_H_INCLUDED

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

/* Forward Declarations */
struct KeyRecord;
class INIReader;

typedef map<string,KeyRecord> KeyMap;
typedef map<string,KeyMap> SectionMap;

typedef map<string,string> DefaultKeyMap;
typedef map<string,DefaultKeyMap> DefaultMap;

#define STR(x) #x
#define EXTRACT(type,sec,opt) settings.extractValue<type>(STR(sec),STR(opt))
#define EXTRACTP(type,sec,opt) settings->extractValue<type>(STR(sec),STR(opt))
#define EXTRACT_(obj,type,sec,opt) settings.extractValue<type>(STR(sec),STR(opt))
#define EXTRACTP_(obj,type,sec,opt) settings->extractValue<type>(STR(sec),STR(opt))

struct KeyRecord {
    protected:
        string value;
        string comment;

        int PosPtr;

    public:
        /* Initializers */
        KeyRecord(const string value);
        KeyRecord(const string, const string,int);

        /* Setters */
        void setValue(const string);
        void setComment(string);
        void setPosPtr(int);

        /* Getters */
        string& getValue();
        string& getComment();
        int getPosPtr();

        string operator() (const KeyRecord& record) {
        	return value;
        }
};

class INIReader {
    protected:
        fstream ini_file;

        int get_pointer;    //Stream pointer to current get line
        int put_pointer;    //Stream pointer to current put line
        int lineNumber;

        bool overwriteMode;

        string ini_name;
        string currLine;   //Current line being read/parsed
        string currSection;     //Current section being parsed
        string currKey;         //Current key being parsed
        string defaultSection;

        SectionMap section_map;
        SectionMap::iterator sec_it;
        KeyMap::iterator key_it;

        KeyMap* temp_map;

        DefaultMap default_map;
        DefaultMap::iterator def_sec_it;
        DefaultKeyMap::iterator def_key_it;

        DefaultKeyMap *def_temp_map;

        vector<string> noOverwriteSection;

        //Loads the sections and (key,value) pairs from the ini file
        void parse_ini();

        int addSection(char* line, bool modCurrSection = true);
        int addSection(string& line, bool modCurrSection = true, bool ignoreRules = false); //-1 = error with header   0 = Fine   1 = Already exists

        int addKey(string& line);
        int addKey(string& line, string& section);

        string& getKeyValue(const string& section, const string& key);
        string& getKeyComment(const string& section, const string& key);
        string& getDefault(const string& section, const string& key);
        static void strip_white_space(string& str, const string& TrimChars = " \t\n\r", int TrimDir = 0);

    public:
        INIReader(/*Uint8 flags = 0x0000*/);
        INIReader(string filename /*, Uint8 flags = 0x0000 */);
        bool load_ini(string filename, bool auto_parse = true, bool overwrite = true);
        bool loaded(); //True = there is a .ini loaded   False = none is loaded or there was an error loading one
        bool exists(const string& section, const string& key);

        /* Manipulators */
        void addDefault(const string& section, const string& key, const string& value);

        void addOverwriteException(const string& section);

        //Returns true if the value was overriden. False if it doesn't exist.
        bool overWriteOp(const string& section, const string& key, const string& value);
        template<class T> T extractValue(const string& section, const string& key);

        string extractComment(const string& section, const string& key);
};

template <class T>
T INIReader::extractValue(const string& section, const string& key) {
	stringstream stream;
    stream << getKeyValue(section, key);

    T out;
    stream >> out;

    return out;
}

/* Without the attribute the compiler thinks it is unused because it depends
 * on the templated function findInVector being used with string& types.
 */
__attribute__((unused)) static bool strEq(const string& s1, const string& s2) {
	if ( s1.compare(s2) == 0 ) return true;

	return false;
}

template<class T>
bool findInVector(vector<T>& vec, const T& item, bool (*comp_fn) (const T&,const T&)) {
	typename std::vector<T>::iterator it;

	for ( it = vec.begin(); it != vec.end(); ++it ) {
		if ( comp_fn(*it, item) ) return true;
	}

	return false;
}

#endif // INIREADER_H_INCLUDED
