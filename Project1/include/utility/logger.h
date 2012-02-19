#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

/** @file */
/* This logger does not cut trees but it
 * does consume bits */

#include <assert.h>
#include <iostream>
#include <fstream>

using namespace std;

/** Initialize a trace log at filename */
FILE* initLog(const char* filename);

/** Close the file stream for the trace log */
void closeLog();

/** Get the file stream to write to */
FILE* getStream();


#endif // LOGGER_H_INCLUDED
