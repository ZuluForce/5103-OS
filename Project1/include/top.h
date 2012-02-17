#ifndef TOP_H_INCLUDED
#define TOP_H_INCLUDED

#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "utility/process_logger.h"
#include "process.h"

#include "boost/format.hpp"

using namespace std;
using namespace boost;

static const char procLogFile[] = "proc.log";

const char* getStatString(eProcState state);

#endif // TOP_H_INCLUDED
