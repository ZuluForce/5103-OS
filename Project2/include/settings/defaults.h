#ifndef DEFAULTS_H_INCLUDED
#define DEFAULTS_H_INCLUDED

#include "iniReader.h"

#define STR(x) #x
#define SETD(sec,op,val) settings.addDefault(STR(sec),STR(op),STR(val))
#define SETDP(sec,op,val) settings->addDefault(STR(sec),STR(op),STR(val))

void setDefaults(INIReader* settings);

#endif
