/* Basic io types */

#include <io_types.h>
#include <stdio.h>

StringTokenizer::StringTokenizer (String s, String d) {
  next_control = 1;
  strcpy (curr, s);
  strcpy (delim, d);
}

// A bit kludgey: strtok has internal state so we have handle this
// No good way to "poke ahead" so we have to read a token and remember it
boolean StringTokenizer::hasMoreTokens ()
{
  if (next_control == 1) {
    r = strtok (curr, delim);
    next_control = 2;
  }
  else if (next_control == 2) {
    r = strtok (NULL, delim);
    next_control = 3;
  }
  return (r != 0);

}

// *Assumes* hasMoreTokens has been called first ... as a side-effect it
// will grab a token that we simply return here - we also change states
String StringTokenizer::nextToken ()
{
  if (next_control == 1)
    next_control = 2;
  if (next_control == 2)
    next_control = 3;
  if (next_control == 3)
    next_control = 2;
  return r;
}

StringTokenizer_rr::StringTokenizer_rr (String s, String d) {
  next_control = 1;
  strcpy (curr, s);
  strcpy (delim, d);
}

// A bit kludgey: strtok has internal state so we have handle this
// No good way to "poke ahead" so we have to read a token and remember it
bool StringTokenizer_rr::hasMoreTokens ()
{
  if (next_control == 1) {
    r = strtok_r (curr, delim, &saveptr);
    next_control = 2;
  }
  else if (next_control == 2) {
    r = strtok_r (NULL, delim, &saveptr);
    next_control = 3;
  }
  return (r != 0);

}

// *Assumes* hasMoreTokens has been called first ... as a side-effect it
// will grab a token that we simply return here - we also change states
String StringTokenizer_rr::nextToken ()
{
  if (next_control == 1)
    next_control = 2;
  if (next_control == 2)
    next_control = 3;
  if (next_control == 3)
    next_control = 2;
  return r;
}

void StringBuffer::append (char c) {
  str = strncat (str, &c, 1);
}

void StringBuffer::append (short s) {
  sprintf (str, "%s%d", str, s);
};

void StringBuffer::append (int i) {
  sprintf (str, "%s%d", str, i);
}

void StringBuffer::append (String st) {
  str = strcat (str, st);
}

String StringBuffer::toString () {
  char *ns = new char[MAXPATH];
  strcpy (ns, str);
  return ns;
}

StringBuffer::StringBuffer (int sz) {
  str = new char[sz];
  memset((void*) str, '\0', sz);
}

StringBuffer::StringBuffer (char* s) {
  str = new char[MAXPATH];
  memset((void*) str, '\0', MAXPATH);
  strcpy (str, s);
}

StringBuffer::StringBuffer (const char* s) {
  str = new char[MAXPATH];
  memset((void*) str, '\0', MAXPATH);
  strcpy (str, s);
}

void StringCut(String cutFrom, const String cutOut, StringBuffer *outBuf) {
	if ( cutFrom == NULL || cutOut == NULL ) {
		return;
	}

	//Find the portion to be cut out
	short index = 0;
	while (true) {
		if ( cutFrom[index] == '\0' ||
			 cutOut[index] == '\0') {
			break;
		}

		if ( cutFrom[index] == cutOut[index] )
			++index;
		else
			break;
	}

	while (true) {
		if ( cutFrom[index] == '\0' )
			break;

		outBuf->append(cutFrom[index++]);
	}

	return;
}

