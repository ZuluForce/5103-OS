#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <map>

#include "vmm_core.h"
#include "utility/id.h"
#include "settings/defaults.h"
#include "Policy/pr_fifo.h"

INIReader* settings;

#define MAX_SETTINGS_PROCS 20

#endif // MAIN_H_INCLUDED
