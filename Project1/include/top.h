#ifndef TOP_H_INCLUDED
#define TOP_H_INCLUDED

#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>

#include "utility/process_logger.h"
#include "process.h"

#include "boost/format.hpp"

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_SIZE (1024 * (EVENT_SIZE + 16)) // in case the optional name char[] is included in the inotify_event
#define DEFAULT_PORT 9000

#define TOO_LONG "--Error: Too Long--"

using namespace std;
using namespace boost;

char procLogFileDir[256];
static const char procLogFile[] = "proc.log";
static const char recvSockName[] = "proc.names.recv";
struct sockaddr_un local, dest;


const char* getStatString(eProcState state);

#endif // TOP_H_INCLUDED
