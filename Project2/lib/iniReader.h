#ifndef INIREADER_H_INCLUDED
#define INIREADER_H_INCLUDED

#include <stdlib.h>
#include <string>
#include <map>
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

/* Option masks for INIReader */
#define MASK(x) (1 << x)        //Puts the bit in the correct location for the mask
#define INI_ALL = 0xFFFFFFFF;

typedef enum {
    INI_STD = 0,
    INI_LOWER,          //All strings are converted to lower case
    INI_IGNORE_COMM,    //Does not save any comments, can save small time if there are a lot
    INI_IGNORE_SEC      //Ignores section headers, puts everything into the same category

} INI_OPTIONS;

typedef enum {
    STD_MASK = MASK(INI_STD),
    LOWER_MASK =  MASK(INI_LOWER),
    IGNORE_COMM_MASK = MASK(INI_IGNORE_COMM),
    IGNORE_SEC_MASK = MASK(INI_IGNORE_SEC),
    IGNORE_ALL_MASK = MASK(INI_IGNORE_COMM) |
                        MASK(INI_IGNORE_SEC)
}OPTION_MASKS;

struct KeyRecord {
    private:
        string value;
        string comment;

        int PosPtr;

    public:
        /* Initializers */
        KeyRecord(string value);
        KeyRecord(string,string,int);

        /* Setters */
        void setValue(string);
        void setComment(string);
        void setPosPtr(int);

        /* Getters */
        string& getValue();
        string& getComment();
        int getPosPtr();
};

class INIReader {
    private:
        fstream ini_file;

        int get_pointer;    //Stream pointer to current get line
        int put_pointer;    //Stream pointer to current put line
        int lineNumber;

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
        bool load_ini(string filename, bool auto_parse = true);
        bool loaded(); //True = there is a .ini loaded   False = none is loaded or there was an error loading one
        bool exists(const string& section, const string& key);

        /* Manipulators */
        void addDefault(const string& section, const string& key, const string& value);
        template<class T> T extractValue(const string& section, const string& key);
        template<class T> T extractDefault(const string& section, const string& key);

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

template <class T>
T INIReader::extractDefault(const string& section, const string& key) {
	stringstream stream;
	stream << getDefault(section,key);

	T out;
	stream >> out;

	return out;
}

#endif // INIREADER_H_INCLUDED
