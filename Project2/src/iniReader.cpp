#include "iniReader.h"

//#define DEBUG

#define COMMENT_DELIMIT '#'
#define SEC_OPEN '['
#define SEC_CLOSE ']'
#define KV_DELIMIT '='

//Used for slicing substrings where you want to exclude the endpoints
//XSLICE -- Exclusive Slice
#define XSLICE(index1,index2) index2 - index1 - 2
//Takes a pair, like those returned from map inserts and accesses the value member
#define PAIR_TO_VAL(pr) pr.first->second

KeyRecord::KeyRecord(const string val) {
    value = val;
}

KeyRecord::KeyRecord(const string val, const string com, int ptr) {
    value = val;
    comment = com;
    PosPtr = ptr;
}

void KeyRecord::setValue(const string val) {
    value = val;
}

void KeyRecord::setComment(string com) {
    comment = com;
}

void KeyRecord::setPosPtr(int ptr) {
    PosPtr = ptr;
}

string& KeyRecord::getValue() {
    return value;
}

string& KeyRecord::getComment() {
    return comment;
}

int KeyRecord::getPosPtr() {
    return PosPtr;
}

INIReader::INIReader(/*Uint8 flags*/) {
    defaultSection = "-";
    addSection(defaultSection,true,true);
    currKey = "-";
    get_pointer = 0;
    put_pointer = 0;
    lineNumber = -1;
    overwriteMode = false;

    addDefault("==NO_DEF==","EMPTY", "");
}

INIReader::INIReader(string filename /*, Uint8 flags*/) {
    defaultSection = "-";
    addSection(defaultSection,true,true);
    currKey = "-";
    get_pointer = 0;
    put_pointer = 0;
    lineNumber = -1;
    overwriteMode = true;

    addDefault("==NO_DEF==","EMPTY", "");

    load_ini(filename,true);
}

void INIReader::parse_ini() {
    //Make sure a parse isn't being resumed
    if ( lineNumber == -1 ) {
        lineNumber = 0;
    }

    put_pointer = ini_file.tellp();
    while ( ini_file.good() ) {
        /* Following line does not work well on Windows
           because of a conversion from CLRF to CR. It can
           be fixed by opening in binary mode or I can just
           keep track of the file position myself */
        //get_pointer = ini_file.tellg();

        getline(ini_file,currLine);
        ++lineNumber;
        get_pointer += currLine.size() + 2;

        strip_white_space(currLine);
        if ( currLine.empty() ) {
            continue;
        }

        #ifdef DEBUG
        printf("\nLine %d is: %s\n",lineNumber,currLine.c_str());
        #endif

        //Check for special line types ( comments, section headers,...)
        switch ( currLine[0] ) {
            //Comment
            case COMMENT_DELIMIT:
                #ifdef DEBUG
                printf("Found a comment\n");
                #endif
                break;

            //Section header
            case SEC_OPEN:
                #ifdef DEBUG
                printf("Found a section header\n");
                #endif
                addSection(currLine);
                break;

            //Some special character \n,\b...skip it
            case '\\':
                break;

            default:
                #ifdef DEBUG
                printf("Found a (key,value) pair\n");
                #endif
                addKey(currLine,currSection);
                break;
        }
        currLine.erase();
    }
}

int INIReader::addSection(char* line, bool modCurrSection) {
    string char_to_str(line);
    return addSection(char_to_str, modCurrSection);
}

//Add a section to the section_map
int INIReader::addSection(string& line, bool modCurrSection, bool ignoreRules) {
    size_t open_bracket = line.find_first_of(SEC_OPEN);
    size_t close_bracket = line.find_last_of(SEC_CLOSE);

    if ( !ignoreRules) {
        if ( open_bracket == line.npos ||
            close_bracket == line.npos ||
            open_bracket == close_bracket) {
            fprintf(stderr, "Invalid section header in file: %s on line %d\n",ini_name.c_str(),lineNumber);
            return -1;
        }
        line = line.substr(open_bracket+1, line.size() - 2);
    }

    if ( section_map.count(line) != 0) {
		if ( modCurrSection ) currSection = line;
        return 1;
    } else {
        #ifdef DEBUG
        printf("Adding Section: %s\n",line.c_str());
        #endif
        section_map[line];

        if ( modCurrSection ) currSection = line;
    }
    return 0;
}

/* Wrapper function so I can use a non-static member as
   a default function parameter. Nicest way I could think
   of doing it */
int INIReader::addKey(string& line) {
    return addKey(line, currLine);
}

int INIReader::addKey(string& line, string& section) {
    sec_it = section_map.find(section);

    if ( sec_it == section_map.end() ) {                                //Check that the section exists
        fprintf(stderr,"Cannot insert key:\n");
        fprintf(stderr,"\tNo section: %s\n",section.c_str());
        return -1;
    }

    string key;
    string value;
    string comment;

    size_t key_end = line.find_first_of(KV_DELIMIT);                    //Fine the '=' sign
    size_t comm_index = line.find_first_of(COMMENT_DELIMIT);            //Check for comments

    if ( key_end == line.npos ) {                                       //Check for syntax problem ie. Key = (nothing)
        fprintf(stderr, "Invalid Key = Value pair\n");
        fprintf(stderr, "\tPossibly missing '=' delimiter\n");
        return -1;
    }

    key =   line.substr(0, key_end - 1);                                //Cut out the key

    if ( comm_index == line.npos) {
        value = line.substr(key_end + 1);                               //There is no comment, cut from '=' to the end
    } else {
        value = line.substr(key_end + 1, XSLICE(key_end,comm_index));   //There is a comment
        if ( comm_index + 1 != line.npos ) {                               //Check that it isn't an empty comment
            #ifdef DEBUG
            printf("Adding Comment to KV pair\n");
            #endif
            comment = line.substr( comm_index + 1 );
        }
    }

    strip_white_space(key);
    strip_white_space(value);
    strip_white_space(comment);

    if ( overwriteMode && !findInVector(noOverwriteSection, section, strEq) ) {
		if ( !overWriteOp(section, key, value) ) {
			cerr << "Failed to overwrite entry" << endl;
			cerr << "Section: " << section << endl;
			cerr << "Key: " << key << endl;
			cerr << "Value: " << value << endl;
		}

		return 0;
    }

    #ifdef DEBUG
    printf("KV Pair:\n");
    printf("\tKey     = %s\n",key.c_str());
    printf("\tValue   = %s\n",value.c_str());
    printf("\tComment = %s\n",comment.c_str());
    #endif

    if ( key.size() == 0 || value.size() == 0) {
        fprintf(stderr,"One side of the Key = Value pair is empty\n");
        return -1;
    }

    temp_map = &(sec_it->second);

    pair<KeyMap::iterator,bool> insert_result;
    insert_result = temp_map->insert( pair<string,KeyRecord>(key, KeyRecord(value)) );
    PAIR_TO_VAL(insert_result).setComment(comment);
    PAIR_TO_VAL(insert_result).setPosPtr(get_pointer);

    if ( insert_result.second == false ) {
        return 1;
    }

    return 0;
}

string& INIReader::getKeyValue(const string& section, const string& key) {
    sec_it = section_map.find(section);     //Get the key map for the particular section

    if ( sec_it == section_map.end() ) {
		return getDefault(section,key);
    }

    temp_map = &(sec_it->second);           //Get the value (key map) from the section map iterator
    key_it = temp_map->find(key);           //Get an iterator to the key in

    if ( key_it == temp_map->end() ) {
		return getDefault(section,key);
    }

    return (key_it->second).getValue();
}

string& INIReader::getKeyComment(const string& section, const string& key) {
    sec_it = section_map.find(section);     //Get the key map for the particular section
    temp_map = &(sec_it->second);           //Get the value (key map) from the section map iterator
    key_it = temp_map->find(key);           //Get an iterator to the key in
    return (key_it->second).getComment();
}

string& INIReader::getDefault(const string& section, const string& key) {
    def_sec_it = default_map.find(section);     //Get the key map for the particular section

    if ( def_sec_it == default_map.end() ) {
        //string ret_val = "";
        return default_map["==NO_DEF=="]["EMPTY"];
    }

    def_temp_map = &(def_sec_it->second);				//Get the value (key map) from the section map iterator
    def_key_it = def_temp_map->find(key);				//Get an iterator to the key in

    if ( def_key_it == def_temp_map->end() ) {
        //string ret_val = "";
        return default_map["==NO_DEF=="]["EMPTY"];
    }

    return def_key_it->second;
}

void INIReader::addDefault(const string& section, const string& key, const string& value) {
	default_map[section][key] = value;

	return;
}

bool INIReader::overWriteOp(const string& section, const string& key, const string& value) {
    sec_it = section_map.find(section);

    if ( sec_it == section_map.end() )
        return false;

    temp_map = &(sec_it->second);
    key_it = temp_map->find(key);

    if ( key_it == temp_map->end() )
        return false;

	(key_it->second).setValue(value);

	return true;
}

void INIReader::strip_white_space(string& str, const string& TrimChars, int TrimDir) {
    size_t startIndex = str.find_first_not_of(TrimChars);
    if (startIndex == std::string::npos){str.erase(); return;}
    if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
    if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(TrimChars) + 1);
}

bool INIReader::load_ini(string filename, bool auto_parse, bool overwrite) {
	if ( ini_file.is_open() ) {
		ini_file.close();
		overwriteMode = overwrite;
	}

    ini_file.open(filename.c_str(), ios_base::in);
    ini_name = filename;

    if ( auto_parse && ini_file.is_open() ) {
        parse_ini();
    }

    return ini_file.is_open();
}

bool INIReader::loaded() {
    return ini_file.is_open();
}

bool INIReader::exists(const string& section, const string& key) {
    sec_it = section_map.find(section);

    if ( sec_it == section_map.end() )
        return false;

    temp_map = &(sec_it->second);
    key_it = temp_map->find(key);

    if ( key_it == temp_map->end() )
        return false;

    return true;
}

string INIReader::extractComment(const string& section, const string& key) {
    string comm( getKeyComment(section, key) );
    return comm;
}


void INIReader::addOverwriteException(const string& section) {
	noOverwriteSection.push_back(section);

	return;
}
