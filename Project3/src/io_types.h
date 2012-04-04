/* Basic io types */

#ifndef IO_TYPES_H
#define IO_TYPES_H

#include <string.h>

#define null 0
#define MaxErrors 50 // more than needed (32), but ok
#define MAXPATH 200

typedef char *StringArr[MaxErrors];
typedef char byte;
typedef int boolean;
typedef char *String;

// Always parsing of pathnames
class StringTokenizer {
  char curr[MAXPATH];
  char delim[MAXPATH];
  int next_control;
  String r;

 public:
  StringTokenizer(String, String);
  boolean hasMoreTokens();
  String nextToken();
};

class StringBuffer {
  char *str;
public:
  void append (char);
  void append (short);
  void append (int);
  void append (String);
  String toString ();
  StringBuffer (int);
  StringBuffer (char*);
  StringBuffer (const char*);
};

#endif
